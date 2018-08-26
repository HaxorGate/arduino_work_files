#include "Arduino.h"
#include "captouch_module.h"


CapTouchModule::CapTouchModule( ) 
	 : IModule( Modules::CapTouch ) {
	for( uint8_t i = 0; i < 8; ++i ){
		_address[i] = 0x5A + i;
		status[i] = 0;
	}
	lastSend = -1000;
	_milli = 250;
}


CapTouchModule::~CapTouchModule( ) {

}


bool CapTouchModule::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) {
	if( size < 1 )
		return false;
	bool handled = false;
	 
	Serial.println(( uint8_t )command, HEX );
	switch( command )
	{
		case Bytecodes::CapTouchRegisterNoTouchState:
		{
			uint8_t buf = 0;
			Wire.beginTransmission( _address[buffer[0]] );
			 Wire.write( &buf, 1 );
			 Wire.endTransmission( );
		break;
		}
		case Bytecodes::CapTouchRegisterTouchState:
		{
			uint8_t buf = 1;
			Wire.beginTransmission( _address[buffer[0]] );
			 Wire.write( &buf, 1 );
			 Wire.endTransmission( );
		break;
		}
		case Bytecodes::CapTouchReportState:
			reportTouch( buffer[0],( status[buffer[0]] == 1 ) ? true : false, false );
		break;
		 case Bytecodes::CapTouchMilliSecDelay:
			_milli = buffer[0]<<4;
			_milli += buffer[1];
		break;
		default:
		break;
	}

	return handled;
}

void CapTouchModule::reportTouch( uint8_t index, bool present, bool interrupt ) {
	_pat->buildMessage( Bytecodes::CapTouchInterrupt );
	_pat->buildMessage( index );
	_pat->buildMessage( present ? 1 : 0 );
	_pat->buildMessage( interrupt ? 1 : 0 );
	_pat->writeMessage( module );
	_pat->log( )->println( "TOUCH CHANGE" );
}

uint32_t CapTouchModule::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent )
		return kForever;
	else if( _state != ModuleState::Ready )
		return 0;

	if( millis( )-lastSend < _milli ) return 0;
//	_pat->log( )->println( "Captouch updating" );
	for( uint8_t i = 0; i < 8; ++i )
	{
		Wire.requestFrom(( uint8_t )( _address[i] ),( uint8_t )1 );
	
		uint8_t s = Wire.read( );
		if( s!= status[i] )//hand was either put on or removed
		{
			if( s == 1 ) //TOUCH
			{
				reportTouch( i,true );
				
			}
			else //No touch
			{
				reportTouch( i,false );
			}
			lastSend = millis( );
			status[i] = s;
		}
	}
	return 150;
}

void CapTouchModule::begin( PATCore* ctx ) {
	IModule::begin( ctx );
	bool found = false;
	for( uint8_t i = 0; i < 8; ++i )
	{
		if( ctx->hardwarePresent( _address[i] ) )
		{
			Wire.beginTransmission( _address[i] );
			Wire.write( 0x13 );
			Wire.endTransmission( );
			_state = ModuleState::Ready;
			found = true;
		}
	}

	if( !found )
	{
		_pat->log( )->print( "absent [" );
		_pat->log( )->print( _address[0], HEX );
		_pat->log( )->println( "]" );
		_state = ModuleState::Absent;
	}
}

void CapTouchModule::end( ) {
	IModule::end( );
}
