#include "Arduino.h"
#include "mosfet_module.h"


#define MOSFET_ADDRESS 		0x55

MOSFETModule::MOSFETModule( )
	 : IModule( Modules::MOSFET ) {
	
}


bool MOSFETModule::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) {
	if( size < 1 )
		return false;
	bool handled = false;
	 
	_pat->log( )->println( "MOSFETModule - Received a message" );
	Serial.println(( uint8_t )command, HEX );
	switch( command )
	{
		case Bytecodes::SetLights:
		{
			_pat->log( )->println( "Bytecodes::SetLights" );

			uint8_t reg = 0x10;
			Wire.beginTransmission( MOSFET_ADDRESS );
			Wire.write( &reg, 1 );
			Wire.write( buffer, size );
			Wire.endTransmission( );

			handled = true;
		break;
		}
		case Bytecodes::FadeLights:
		{
			_pat->log( )->println( "Bytecodes::FadeLights" );

			uint8_t reg = 0x20;
			Wire.beginTransmission( MOSFET_ADDRESS );
			Wire.write( &reg, 1 );
			Wire.write( buffer, size );
			Wire.endTransmission( );

			handled = true;
		break;
		}
		case Bytecodes::DigitalWrite:
		{
			_pat->log( )->println( "Bytecodes::DigitalWrite" );

			uint8_t reg = 0x30;
			Wire.beginTransmission( MOSFET_ADDRESS );
			Wire.write( &reg, 1 );
			Wire.write( buffer, size );
			Wire.endTransmission( );

			handled = true;
		break;
		}
		default:
		break;
	}

	_pat->log( )->print( "handled = " );
	_pat->log( )->println( handled ? "true" : "false" );
	return handled;
}

MOSFETModule::~MOSFETModule( ) {

}

uint32_t MOSFETModule::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent )
		return kForever;
	else if( _state != ModuleState::Ready )
		return 0;
}

void MOSFETModule::begin( PATCore* ctx ) {
	IModule::begin( ctx );

	_pat->log( )->print( "MOSFET Module " );
	if( ctx->hardwarePresent( MOSFET_ADDRESS ) )
	{
		_state = ModuleState::Ready;
		_pat->log( )->println( "present" );
	}
	else
	{
		_pat->log( )->print( "absent [" );
		_pat->log( )->print( MOSFET_ADDRESS, HEX );
		_pat->log( )->println( "]" );
		_state = ModuleState::Absent;
	}
}

void MOSFETModule::end( ) {
	IModule::end( );
}
