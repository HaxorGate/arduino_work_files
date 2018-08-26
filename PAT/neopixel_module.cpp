#include "Arduino.h"
#include "neopixel_module.h"

NeoPixelModule::NeoPixelModule( ) 
	 : IModule( Modules::NeoPixel ) {
	_address = 0x52;
}

NeoPixelModule::~NeoPixelModule( ) {

}

bool NeoPixelModule::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) {
	if( size < 1 )
		return false;
	bool handled = false;
	 
	_pat->log( )->println( "Received a message" );
	Serial.println(( uint8_t )command, HEX );
	switch( command )
	{
		case Bytecodes::SetLEDs:
		{
			handled = true;
			if( size < 6 ) break;
			if( buffer[0] & 0x10 == 0x10 ) //exclusive
			{
				_pat->buildMessage( 1 );
			}
			else
			{
				_pat->buildMessage( 0 );
			}
			char startVal =( buffer[1] );
			char endVal =( buffer[2] );
			char r =( buffer[3] );
			char g =( buffer[4] );
			char b =( buffer[5] );
		 _pat->log( )->print( "Changing pixels " );
		 _pat->log( )->print( startVal, DEC );
		 _pat->log( )->print( " - " );
		 _pat->log( )->print( endVal, DEC );
		 _pat->log( )->print( " to " );
		 _pat->log( )->print( r, DEC );
		 _pat->log( )->print( ", " );
		 _pat->log( )->print( g, DEC );
		 _pat->log( )->print( ", " );
		 _pat->log( )->print( b, DEC );
		 _pat->log( )->println( );
		 Wire.beginTransmission( _address );
		 Wire.write( buffer, 6 );
		 Wire.endTransmission( );
		}
		break;
		default:
			break;
	}

	return handled;
}

uint32_t NeoPixelModule::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent )
		return kForever;
	else if( _state != ModuleState::Ready )
		return 0;
	return 500;
}

void NeoPixelModule::begin( PATCore* ctx ) {
	
	IModule::begin( ctx );
	
	if( ctx->hardwarePresent( _address ) )
	{
		_pat->log( )->println( "NEOPIXEL PRESENT!" );
			_state = ModuleState::Ready;
	}
	else
	{
		_pat->log( )->println( "NEOPIXEL NOT PRESENT!" );
	}
}

void NeoPixelModule::end( ) {
	IModule::end( );
}


