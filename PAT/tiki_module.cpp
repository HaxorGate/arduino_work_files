#include "Arduino.h"
#include "tiki_module.h"


#define TIKI_ADDRESS 		0x53

TikiModule::TikiModule( ) : IModule( Modules::Tiki ) {
	_elapsed = 0;
	_lastStatus = 0xFF;
	_coinDebounce = false;
}


bool TikiModule::receiveCommand( Bytecodes cmd, uint8_t* buff, uint32_t size ) {
	if( size < 1 )
		return false;
	bool handled = true;
	 
	_pat->log( )->println( "TikiModule - Received a message" );
	_pat->log( )->println(( uint8_t )cmd, HEX );
	uint8_t sent = 0xFF;
	
	switch( cmd ) {
		case Bytecodes::SetLEDs: {
			int startIndex = 1;
			int endIndex = 2;

			if( size >= 6 ) {
				if( size >= 8 ) {
					startIndex = 2;	 // start( LSB of two-byte big-endian value )
					endIndex = 4;		 // end( LSB of two-byte big-endian value )
				}
				uint8_t toSend[6];
				toSend[0] =( buff[0] == 0x00 ) ? 0x10 : 0x11;
				toSend[1] = buff[startIndex];
				toSend[2] = buff[endIndex];
				toSend[3] = buff[endIndex + 1];
				toSend[4] = buff[endIndex + 2];
				toSend[5] = buff[endIndex + 3];
				Wire.beginTransmission( TIKI_ADDRESS );
				Wire.write( toSend, 6 );
				sent = Wire.endTransmission( );
			} else {
				handled = false;
			}
		}
		break;
		case Bytecodes::TikiDispenseCoconut: {
			uint8_t toSend[2];
			toSend[0] = 0x12;
			Wire.beginTransmission( TIKI_ADDRESS );
			Wire.write( toSend, 2 );
			sent = Wire.endTransmission( );
		}
	break;
		case Bytecodes::TikiSetCoinGate: {
			uint8_t toSend[2];
			toSend[0] = 0x13;
			toSend[1] = buff[0];
			Wire.beginTransmission( TIKI_ADDRESS );
			Wire.write( toSend, 2 );
			sent = Wire.endTransmission( );
		}
		break;
		case Bytecodes::TikiManualSetEMag: {
			uint8_t toSend[2];
			toSend[0] = 0x30;
			toSend[1] = buff[0];
			Wire.beginTransmission( TIKI_ADDRESS );
			Wire.write( toSend, 2 );
			sent = Wire.endTransmission( );
		}
		break;
		case Bytecodes::TikiManualLoadMode: {
			uint8_t toSend[2];
			toSend[0] = 0x31;
			toSend[1] = buff[0];
			Wire.beginTransmission( TIKI_ADDRESS );
			Wire.write( toSend, 2 );
			sent = Wire.endTransmission( );
		}
		break;
		case Bytecodes::TikiManualSetScrews: {
			uint8_t toSend[2];
			toSend[0] = 0x31;
			toSend[1] = buff[0];
			Wire.beginTransmission( TIKI_ADDRESS );
			Wire.write( toSend, 2 );
			sent = Wire.endTransmission( );
		}
		break;
		case Bytecodes::TikiManualCalibrateScrew: {
			uint8_t toSend[2];
			toSend[0] = 0x32;
			toSend[1] = buff[0];
			Wire.beginTransmission( TIKI_ADDRESS );
			Wire.write( toSend, 2 );
			sent = Wire.endTransmission( );
		}
		break;
		case Bytecodes::TikiManualAdjustScrew: {
			uint8_t toSend[4];
			toSend[0] = 0x33;
			toSend[1] = buff[0];
			toSend[2] = buff[1];
			toSend[3] = buff[2];
			Wire.beginTransmission( TIKI_ADDRESS );
			Wire.write( toSend, 4 );
			sent = Wire.endTransmission( );
		}
		break;
		case Bytecodes::TikiManualReset: {
			uint8_t toSend[2];
			toSend[0] = 0x3F;
			Wire.beginTransmission( TIKI_ADDRESS );
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
		_pat->log( )->print( "Wire.endTransmission( ) = " );
		_pat->log( )->println( sent, HEX );
	}

	return handled;
}

TikiModule::~TikiModule( ) {

}

uint32_t TikiModule::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent ) {
		return kForever;
	} else if( _state != ModuleState::Ready ) {
		return 0;
	}

	uint32_t target = 150;
	_elapsed += elapsed;
	if( _elapsed < target ) {
		return target - _elapsed;
	}
	_elapsed = 0;

	bool sawCoin = false;

	Wire.requestFrom(( uint8_t )TIKI_ADDRESS, 2 );
	if( Wire.available( ) ) {
		uint8_t interrupts = Wire.read( );
		uint8_t status = Wire.read( );

		/*
			Interrupt byte:
			0b00000111			Sequence step
			0b00001000			Executing program
			0b00010000			COIN DETECTED
			0b00100000			Top withdrawn
			0b01000000			Bottom withdrawn
			0b10000000			Steppers calibrated
		*/

		if(( interrupts & 0x10 ) == 0x10 ) {
			sawCoin = true;
		} else if( status != _lastStatus ) {
			_pat->buildMessage( Bytecodes::TikiStatusChange );
			_pat->buildMessage( status );
			_pat->buildMessage( _lastStatus );
			_pat->writeMessage( module );
			_lastStatus = status;
		}
	}
	
	if( sawCoin ) {
		if( !_coinDebounce ) {
			_pat->buildMessage( Bytecodes::TikiCoinCollected );
			_pat->writeMessage( module );
			_coinDebounce = true;
		}

		uint8_t toSend[2];
		toSend[0] = 0x14;
		Wire.beginTransmission( TIKI_ADDRESS );
		Wire.write( toSend, 2 );
		Wire.endTransmission( );
	} else {
		_coinDebounce = false;
	}
	return 150;
}


void TikiModule::begin( PATCore* ctx ) {
	IModule::begin( ctx );
	if( ctx->hardwarePresent( TIKI_ADDRESS ) ) {
			_state = ModuleState::Ready;
			Wire.beginTransmission( TIKI_ADDRESS );
			Wire.write( 0x3F ); 
			Wire.endTransmission( ); 
	} else {
		_state = ModuleState::Absent;
	}
}

void TikiModule::end( ) {
	IModule::end( );
}
