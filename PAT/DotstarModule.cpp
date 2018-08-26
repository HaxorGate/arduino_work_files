#include "Arduino.h"
#include "DotstarModule.h"


#define DOTSTAR_ADDRESS		 0x10	//base address for thing

DotstarModule::DotstarModule( )
	 : IModule( Modules::DotstarModule ) {
	
}


bool DotstarModule::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) {
	if( size < 1 )
		return false;
	bool handled = true;
	 
	_pat->log( )->println( "Dotstar	- Received a message" );
	_pat->log( )->println(( uint8_t )command, HEX );
	uint8_t sent = 0xFF;
	int i2cAddressOffset = 0; 
	

	switch( command )
	{
		case Bytecodes::SetLEDs:
		{
			Wire.beginTransmission( DOTSTAR_ADDRESS );
			//							 cmd	 excl	strt1 strt2 end1	end2	 r		 g		 b
			Wire.write(( char )command );
			Wire.write( buffer,9 );	
			Wire.endTransmission( ); 
			
		}
	break;
		default:
			handled = false;
		break;
	}

	if( sent != 0xFF )
	{
		Serial.print( "Wire.endTransmission( ) = " );
		Serial.println( sent, HEX );
	}

	return handled;
}

DotstarModule::~DotstarModule( ) {

}

uint32_t DotstarModule::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent )
		return kForever;
	else if( _state != ModuleState::Ready )
		return 0;

	return 150;
}


void DotstarModule::begin( PATCore* ctx ) {
	IModule::begin( ctx );

	_state = ModuleState::Absent; //Overwritten if any is ready 
	
	_pat->log( )->print( "DotstarModule( " );
	if( ctx->hardwarePresent( DOTSTAR_ADDRESS ) )
	{
			_pat->log( )->println( " ) PRESENT!" );		
			_state = ModuleState::Ready;
			Wire.beginTransmission( DOTSTAR_ADDRESS );
			//							 cmd	 excl	strt1 strt2 end1	end2	 r		 g		 b
			char buffer[] = {0x10, 0x01, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00};
			Wire.write( buffer,9 );	//Currently, this does nothing, just like in the Tiki Module
			Wire.endTransmission( ); 
	} else {
		_pat->log( )->print( DOTSTAR_ADDRESS );
		_pat->log( )->println( " ) absent" );
	}

}

void DotstarModule::end( ) {
	IModule::end( );
}
