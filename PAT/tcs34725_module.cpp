#include "Arduino.h"
#include "tcs34725_module.h"

#define MINIMUM_INTERVAL	33

#define MODE_UNKNOWN			0
#define MODE_DELTA				1
#define MODE_CRGB				 2

TCS34725Module::TCS34725Module( ) : IModule( Modules::TCS34725 ) {
}

TCS34725Module::~TCS34725Module( ) {

}

bool TCS34725Module::setMode( uint8_t mode, uint8_t* pars, uint32_t parSize ) {
	bool handled = false;
	switch( mode ) {
		case MODE_DELTA:
			if( parSize > 0 ) {
				_delta = pars[0];
				_updateMode = mode;
				handled = true;
			}
		break;
		case MODE_CRGB:
			_updateMode = mode;
		break;
		default:
		break;
	}

	if( !handled ) {
		_updateMode = MODE_UNKNOWN;
	}
	return handled;
}

void TCS34725Module::setUpdateInterval( uint16_t interval ) {
	uint16_t val = interval;
	if( val < MINIMUM_INTERVAL ) {
		val = MINIMUM_INTERVAL;
	}
	_updateInterval = val;

	if( _updateInterval >= 700 ) {
		_device->setIntegrationTime( TCS34725_INTEGRATIONTIME_700MS );
		_scale = 256;
	} else if( _updateInterval >= 154 ) {
		_device->setIntegrationTime( TCS34725_INTEGRATIONTIME_154MS );
		_scale = 64;
	} else if( _updateInterval >= 101 ) {
		_device->setIntegrationTime( TCS34725_INTEGRATIONTIME_101MS );
		_scale = 42;
	} else if( _updateInterval >= 50 ) {
		_device->setIntegrationTime( TCS34725_INTEGRATIONTIME_50MS );
		_scale = 20;
	} else {
		_device->setIntegrationTime( TCS34725_INTEGRATIONTIME_24MS );
		_scale = 10;
	}
}

bool TCS34725Module::parseData( ) {
	if( _scale == 0 ) {
		return false;
	}
	uint16_t rawR, rawG, rawB, rawC;

	_device->getRawData( &rawR, &rawG, &rawB, &rawC );

	switch( _updateMode ) {
		case MODE_DELTA: {
			uint8_t r, g, b;
			r = round( 256.0 *( float )rawR /( float )rawC );
			g = round( 256.0 *( float )rawG /( float )rawC );
			b = round( 256.0 *( float )rawB /( float )rawC );

			int dR =(( int )r - _lastR );
			int dG =(( int )g - _lastG );
			int dB =(( int )b - _lastB );

			_lastR = r;
			_lastG = g;
			_lastB = b;

			uint32_t udelta = dR * dR;
			udelta += dG * dG;
			udelta += dB * dB;
			if( udelta > _delta ) {
					_lastDelta = udelta;
					_sendNow = true;
			}
		}
		break;
		case MODE_CRGB: {
			_data = ":C:ccccc:R:rrrrr:G:ggggg:B:bbbbb";
			String _buff = String( rawC );
			_data.replace( "ccccc", _buff );
			_buff = String( rawR );
			_data.replace( "rrrrr", _buff );
			_buff = String( rawG );
			_data.replace( "ggggg", _buff );
			_buff = String( rawB );
			_data.replace( "bbbbb", _buff );
			_sendNow = true;
		}
		default:
		break;
	}

	return false;
}

void TCS34725Module::sendData( Bytecodes command ) {
	if( _updateMode == MODE_UNKNOWN ) {
		return;
	}
	_pat->buildMessage( command );
	switch( _updateMode ) {
		case MODE_DELTA:
			_pat->buildMessage( _lastDelta );
			_pat->buildMessage( _lastR );
			_pat->buildMessage( _lastG );
			_pat->buildMessage( _lastB );
		break;

		case MODE_CRGB: {
			uint16_t length = _data.length( );
			uint8_t *crgb_string =( uint8_t * )malloc( length );
			_data.getBytes( crgb_string, length );
			_pat->buildMessage( crgb_string, length );
		}
		break;
		default:
		break;
	}
	_pat->writeMessage( module );
}

bool TCS34725Module::receiveCommand( Bytecodes cmd, uint8_t* buff, uint32_t sz ) {
	if( sz < 1 )
		return false;

	bool handled = true;
	_pat->log( )->print( "Color Sensor received command " );
	_pat->log( )->println( static_cast<uint8_t>( cmd ) );
	switch( cmd )
	{
		case Bytecodes::Configure:
			if( sz < 3 ) {
				handled = false;
			} else {
				setUpdateInterval(( buff[1] << 8 ) + buff[2] );
				handled = setMode( buff[0], buff + 3, sz - 3 );
			}
		break;

		case Bytecodes::DataReport:
			if( _updateMode == MODE_UNKNOWN ) {
				handled = false;
			} else {
				_sendNow = true;
			}
		break;

		default:
			handled = false;
		break;
	}

	return handled;
}

uint32_t TCS34725Module::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent )
		return kForever;
	else if( _state != ModuleState::Ready )
		return 0;

	if( !isWaitDone( elapsed ) )
		return remainingWait( );

	if( parseData( ) )
		sendData( Bytecodes::DataAvailable );
	else if( _sendNow )
	{
		sendData( Bytecodes::DataReport );
		_sendNow = false;
	}

	if( _updateInterval < MINIMUM_INTERVAL )
		return setWait( MINIMUM_INTERVAL );
	else
		return setWait( _updateInterval );
}

void TCS34725Module::begin( PATCore* ctx ) {
	IModule::begin( ctx );
	bool present = ctx->hardwarePresent( TCS34725_ADDRESS );

	_pat->log( )->print( "TCS34725 Module " );
	if( present ) 	{
		_device = new Adafruit_TCS34725( );
		_device->begin( );

		_state = ModuleState::Ready;

		_updateMode = MODE_UNKNOWN;
		setUpdateInterval( 33 );

		_sendNow = true;

		_pat->log( )->print( "PRESENT [" );
	} else {
		_pat->log( )->print( "absent [" );
		_state = ModuleState::Absent;
	}
	_pat->log( )->print( TCS34725_ADDRESS, HEX );
	_pat->log( )->println( "]" );
}

void TCS34725Module::end( ) {
	IModule::end( );
}


