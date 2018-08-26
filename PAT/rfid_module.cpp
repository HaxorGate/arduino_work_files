#include "Arduino.h"
#include "rfid_module.h"

#define MAX_CARD_ID_LENGTH	7

#define STATUS_NO_ACTIVITY	0
#define STATUS_PRESENT			1
#define STATUS_GONE					2

#define RFID_ADDRESS 				0x49

bool RFID_Addresses[8];

RFIDModule::RFIDModule( ) : IModule( Modules::RFID ) {
  for ( int i = 0; i < 8; ++i ) {
    _lastCardID[i] = NULL;
  }
}

RFIDModule::~RFIDModule( ) {

}

bool RFIDModule::wasRFIDPresent( int i ) {
  return ( _lastCardIDSize[i] > 0 );
}

void RFIDModule::reportCard( bool present, uint8_t i ) {
  _pat->buildMessage( Bytecodes::DataAvailable );
  _pat->buildMessage( i );
  _pat->buildMessage( _lastCardIDSize[i] );
  _pat->buildMessage( _lastCardID[i], _lastCardIDSize[i] );
  _pat->buildMessage( present ? 1 : 0 );
  _pat->writeMessage( module );
}

uint32_t RFIDModule::update( uint32_t elapsed ) {
  if ( _state == ModuleState::Absent ) {
    return kForever;
  } else if ( _state != ModuleState::Ready ) {
    return 0;
  }
  for ( uint8_t index = 0; index < 8; ++index ) {
    if ( _lastCardID[index] == NULL )continue; {
      bool wasPresent = false;
      if ( wasRFIDPresent( index ) ) {
        // Once we detect an RFID, slow down updates.
        /*if( !isWaitDone( elapsed ) )
        	return remainingWait( );*/
        wasPresent = true;
      }
      uint8_t cardID[MAX_CARD_ID_LENGTH];
      uint8_t length;
      uint8_t status;

      Wire.requestFrom(
        static_cast<uint8_t>( RFID_ADDRESS + index ),
        static_cast<uint8_t>( MAX_CARD_ID_LENGTH + 2 )
      );
      status = Wire.read( );
      length = Wire.read( );
      for ( int i = 0; i < MAX_CARD_ID_LENGTH; i++ ) {
        if ( Wire.available( ) ) {
          cardID[i] = Wire.read( );
        } else {
          cardID[i] = 0;
        }
      }
      bool known = false;
      if ( status != STATUS_NO_ACTIVITY ) {
        _pat->log( )->print( status, HEX );
        _pat->log( )->print( " " );
        _pat->log( )->print( length, HEX );
        _pat->log( )->print( " " );
        for ( int i = 0; i < MAX_CARD_ID_LENGTH; i++ ) {
          if ( i > 0 ) {
            _pat->log( )->print( " " );
          }
          _pat->log( )->print( cardID[i], HEX );
        }
        _pat->log( )->println( "" );

        // We have a card! Did we previously have a card?
        if ( _lastCardIDSize[index] ) {
          // Okay. Are they the same card?
          if ( length == _lastCardIDSize[index] ) {
            if ( memcmp( cardID, _lastCardID[index], length ) == 0 )
              known = true;
          }

          // If this isn't the same card, then first report the departure of the old one
          if ( !known ) {
            _pat->log( )->println( "KNOWN CARD GONE" );
            reportCard( false, index );
            _lastCardIDSize[index] = 0;
          }
        }
        // We haven't seen this card, so remember it and report it
        if ( !known ) {
          memcpy( _lastCardID[index], cardID, length );
          _lastCardIDSize[index] = length;
          _pat->log( )->println( "NEW CARD FOUND" );
          reportCard( true, index );
        }
        if ( _lastCardIDSize[index] && status == STATUS_GONE ) {
          _pat->log( )->println( "NEW CARD GONE" );
          reportCard( false, index );
          _lastCardIDSize[index] = 0;
        }
      }
    }
    // If we have a card, don't check for updates so often
    //return wasRFIDPresent( ) ? 300 : 150;
    return 150;
  }
}


void RFIDModule::begin( PATCore* ctx ) {
  IModule::begin( ctx );
  int count = 0;
  for ( uint8_t i = 0; i < 8; ++i )
  {
    if ( ctx->hardwarePresent( RFID_ADDRESS + i ) )
    {
      Wire.beginTransmission( RFID_ADDRESS + i );
      Wire.write( 0x22 );
      Wire.endTransmission( );
      _lastCardID[i] = new uint8_t[MAX_CARD_ID_LENGTH + 2];
      ++count;
    }
  }
  if ( count > 0 )
  {
    _state = ModuleState::Ready;
  }
  else
  {
    _state = ModuleState::Absent;
  }
}

void RFIDModule::end( ) {
  IModule::end( );
}
