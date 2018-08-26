#include "Arduino.h"
#include "gem_dispenser_module.h"


#define GEMDISPENSER_ADDRESS		 0x56	//Arbitrary number of slave module that doesn't conflict with standards or currently used addresses
#define ErrorNone									 0x00

GemDispenserModule ::GemDispenserModule( )
	 : IModule( Modules::GemDispenser ) {
	_elapsed = 0;
	_lastStatus = 0xFF;
	for( int i = 0; i <= 1; i++ ){
	_isDispensing[i] = false;
	}
}


bool GemDispenserModule ::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) {
	if( size < 1 )
		return false;
	bool handled = true;
	 
	_pat->log( )->println( "GemDispenserModule	- Received a message" );
	Serial.println(( uint8_t )command, HEX );
	uint8_t sent = 0xFF;
	int gemDispenser = buffer[0];
	int i2cAddressOffset = 0; //The first three dispensers go to base slave address, second three go to address + 1
	if( gemDispenser > 2 )
	{
		gemDispenser -= 3;
		i2cAddressOffset = 1;
	}

	switch( command )
	{
		case Bytecodes::GemDispenserDispenseGem:
		{
			_pat->log( )->print( "GemDispenserModule " );
			_pat->log( )->println( _isDispensing[i2cAddressOffset] ? "is dispensing" : "is NOT dispensing" );
			//Can't dispense gem if they are currently dispensing, so don't handle
			if( _isDispensing[i2cAddressOffset] )
			{
				handled = false;
			break;
			}
			uint8_t toSend[2];
			toSend[0] = 0x10;										//GemDispenser Slave recognizes this as Dispense Gem Command
			toSend[1] = gemDispenser;						//Second byte is which gem wheel to dispense
			Wire.beginTransmission( GEMDISPENSER_ADDRESS + i2cAddressOffset );
			Wire.write( toSend, 2 );
			sent = Wire.endTransmission( );
		}
	break;

		case Bytecodes::GemDispenserStopAll:
		{
			_pat->log( )->println( "GemDispenserModule stop all" );
			uint8_t toSend[2];
			toSend[0] = 0x20;								 //GemDispenser Slave recognizes this as Stop All command
			toSend[1] = gemDispenser;				 //Second byte is which gem wheel to stop
			Wire.beginTransmission( GEMDISPENSER_ADDRESS + i2cAddressOffset );
			Wire.write( toSend, 2 );
			sent = Wire.endTransmission( );
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

GemDispenserModule ::~GemDispenserModule( ) {

}

uint32_t GemDispenserModule ::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent )
		return kForever;
	else if( _state != ModuleState::Ready )
		return 0;

	if( !isWaitDone( elapsed ) )
		return remainingWait( );
	//uint32_t target = 150;
	//_elapsed += elapsed;
	//if( _elapsed < target )
	//	return target - _elapsed;
	//_elapsed = 0;


	for( int i = 0; i <= 1; i++ )
	{
		Wire.requestFrom(( uint8_t )GEMDISPENSER_ADDRESS + i, 3 );
		if( Wire.available( ) )
		{
			uint8_t interrupts = Wire.read( );
			uint8_t status = Wire.read( );
			uint8_t dispenser = Wire.read( );
			//_pat->log( )->print( "Status " );
			//_pat->log( )->println( status, HEX );
			/*
				Interrupt byte:
				0b00000111			Sequence step
				0b00001000			Dispensing gem
				*/

			//If any errors are reported, push them up to the server
			if( status != ErrorNone )
			{
				_pat->buildMessage( Bytecodes::GemDispenserErrorReported );
				_pat->buildMessage( status );
				_pat->buildMessage((( uint8_t )i * 3 ) + dispenser ); //Had to change Gem Dispenser slave code to pass this back up on error
				_pat->writeMessage( module );

				Serial.print( "Dispenser with Error: " );
				Serial.println( dispenser, HEX );
			}
	
			_isDispensing[i] =(( interrupts & 0x08 ) == 0x08 );
		}
	}
	
	return setWait( 150 );
	//return 150;
}


void GemDispenserModule ::begin( PATCore* ctx ) {
	IModule::begin( ctx );

	_state = ModuleState::Absent; //Overwritten if any is ready 
	
	for( int i = 0; i <= 1; i++ )
	{
		_pat->log( )->print( "Gem Dispenser Module( " );
		_pat->log( )->print( i, DEC );
		if( ctx->hardwarePresent( GEMDISPENSER_ADDRESS + i ) )
		{
				_pat->log( )->println( " ) PRESENT!" );		
				_state = ModuleState::Ready;
				Wire.beginTransmission( GEMDISPENSER_ADDRESS + i );
				Wire.write( 0x3F );	//Currently, this does nothing, just like in the Tiki Module
				Wire.endTransmission( ); 
		}
		else
		{
			_pat->log( )->println( " ) absent" );		
		}
	}
}

void GemDispenserModule ::end( ) {
	IModule::end( );
}
