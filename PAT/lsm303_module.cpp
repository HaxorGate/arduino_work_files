#include "Arduino.h"
#include "lsm303_module.h"

#define MINIMUM_INTERVAL	33

#define MODE_UNKNOWN		0
#define MODE_ROTATION		1

LSM303Module::LSM303Module( )
	 : IModule( Modules::LSM303 ) {
}

LSM303Module::~LSM303Module( ) {

}

const float _kScale =( float )( 1 << 14 );
bool LSM303Module::setMode( uint8_t mode, uint8_t* parameters, uint32_t parametersSize ) {
	bool handled = false;
	switch( mode )
	{
		case MODE_ROTATION:
			if( parametersSize > 0 )
			{
				_updateMode = MODE_ROTATION;
				_fullRotation =( float )parameters[0];
				_lastRotation = _thisRotation = 0;
			}
			handled = true;
			break;

		default:
			break;
	}

	if( !handled )
		_updateMode = MODE_UNKNOWN;

	return handled;
}

bool LSM303Module::parseData( ) {
	switch( _updateMode )
	{
		case MODE_ROTATION:
		{
			float x = _device->accelData.x / _kScale;
			float y = _device->accelData.y / _kScale;
			float angle = atan2( y, x ) * _fullRotation /( 2.0f * M_PI );
			if( angle < 0 )
				angle += _fullRotation;
			uint16_t val = round( _fullRotation - angle );
			if( val != _thisRotation )
			{
				_lastRotation = _thisRotation;
				_thisRotation = val;
				return true;
			}
		}
		break;

		default:
			break;
	}

	return false;
}

void LSM303Module::sendData( Bytecodes command ) {
	if( _updateMode == MODE_UNKNOWN )
		return;

	_pat->buildMessage( command );
	switch( _updateMode )
	{
		case MODE_ROTATION:
			_pat->buildMessage( _thisRotation >> 8 );
			_pat->buildMessage( _thisRotation & 0xFF );
			_pat->buildMessage( _lastRotation >> 8 );
			_pat->buildMessage( _lastRotation & 0xFF );
			break;

		default:
			break;
	}
	_pat->writeMessage( module );
}

bool LSM303Module::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) {
	if( size < 1 )
		return false;

	bool handled = true;
	switch( command )
	{
		case Bytecodes::Configure:
			if( size < 3 )
				handled = false;
			else
			{
				_updateInterval =( buffer[1] << 8 ) + buffer[2];
				handled = setMode( buffer[0], buffer + 3, size - 3 );
			}
			break;

		case Bytecodes::DataReport:
			if( _updateMode == MODE_UNKNOWN )
				handled = false;
			else
				_sendNow = true;
			break;

		default:
			handled = false;
			break;
	}

	return handled;
}

uint32_t LSM303Module::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent )
		return kForever;
	else if( _state != ModuleState::Ready )
		return 0;

	if( !isWaitDone( elapsed ) )
		return remainingWait( );

	_device->read( );

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

void LSM303Module::begin( PATCore* ctx ) {
	IModule::begin( ctx );
	bool present = ctx->hardwarePresent( LSM303_ADDRESS_ACCEL );

	_pat->log( )->print( "LSM303 Module " );
	if( present )
	{
		_device = new Adafruit_LSM303( );
		_device->begin( );

		_state = ModuleState::Ready;

		_updateMode = MODE_UNKNOWN;

		_updateInterval = 33;

		_sendNow = true;

		_pat->log( )->println( "PRESENT" );
	}
	else
	{
		_pat->log( )->print( "absent [" );
		_pat->log( )->print( LSM303_ADDRESS_ACCEL, HEX );
		_pat->log( )->println( "]" );
		_state = ModuleState::Absent;
	}
}

void LSM303Module::end( ) {
	IModule::end( );
}


