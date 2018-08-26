#include "Arduino.h"
#include "captouchmulti_module.h"


CapTouchMultiModule::CapTouchMultiModule( ) 
	 : IModule( Modules::CapTouchMulti ) {
		_address = 0x64;
		status = 0;
		lastSend = -1000;
	_milli = 250;
}



CapTouchMultiModule::~CapTouchMultiModule( ) {

}


bool CapTouchMultiModule::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) {
	if( size < 1 )
		return false;
	bool handled = false;
	 
	Serial.println(( uint8_t )command, HEX );
	switch( command )
	{
		case Bytecodes::CapTouchRegisterNoTouchState:
		{
			uint8_t buf = 0;
			Wire.beginTransmission( _address );
			 Wire.write( &buf, 1 );
			 Wire.endTransmission( );
		break;
		}
		case Bytecodes::CapTouchRegisterTouchState:
		{
			uint8_t buf = 1;
			Wire.beginTransmission( _address );
			 Wire.write( &buf, 1 );
			 Wire.endTransmission( );
		break;
		}
		case Bytecodes::CapTouchReportState:
			reportTouch( buffer[0],( status== 1 ) ? true : false, false );
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

void CapTouchMultiModule::reportTouch( uint8_t index, bool present, bool interrupt ) {
	_pat->buildMessage( Bytecodes::CapTouchInterrupt );
	_pat->buildMessage( index );
	_pat->buildMessage( present ? 1 : 0 );
	_pat->buildMessage( interrupt ? 1 : 0 );
	_pat->writeMessage( module );
	_pat->log( )->println( "TOUCH CHANGE" );
}

uint32_t CapTouchMultiModule::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent )
		return kForever;
	else if( _state != ModuleState::Ready )
		return 0;

	if( millis( )-lastSend < _milli ) return 0;
	
	Wire.requestFrom(( uint8_t )( _address ),( uint8_t )3 );

	for( uint8_t i = 0; i < 3; ++i )
	{
		uint8_t s = Wire.read( );
		if( s!= status )//hand was either put on or removed
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
			status = s;
		}
	}
	return 150;
}

void CapTouchMultiModule::begin( PATCore* ctx ) {
	IModule::begin( ctx );
	if( ctx->hardwarePresent( _address ) )
	{
		Wire.beginTransmission( _address );
		Wire.write( 0x13 );
			Wire.endTransmission( );
			_state = ModuleState::Ready;	
	}
	else	
	{
			_pat->log( )->print( "absent [" );
			_pat->log( )->print( _address, HEX );
			_pat->log( )->println( "]" );
			_state = ModuleState::Absent;

	}
}

void CapTouchMultiModule::end( ) {
	IModule::end( );
}
