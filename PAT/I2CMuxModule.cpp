/*
	 I AM FUDING THIS TOGETHER BY MAKING IT SPECIFIC TO MUXING THE COLOR SENSORS IN
	 MAZE OF GAMES. THIS MUST BE REWORKED TO THE GENERAL CASE, BUT I AM RUNNING OUT
	 OF TIME. AS SOON AS EVERYONE ELSE FALLS BEHIND SCHEDULE, I WILL WORK ON THIS
*/
#include <Adafruit_TCS34725.h>
#include "tcs34725_module.h"
#include "Arduino.h"
#include "I2CMuxModule.h"
#define iC 0
#define iR 1
#define iG 2
#define iB 3
#define iADDR 0
#define iSEG0 3
#define iSEG1 4
#define iSEG2 5
#define iSEG3 6
#define iSEG4 7
#define iSEG5 8

uint8_t I2CMuxModule::_object_count = 0;

uint8_t I2CMuxModule::howMany( void ) {
	return _object_count;
}

uint16_t rawC, rawR, rawG, rawB;

I2CMuxModule::I2CMuxModule( uint8_t offset ) : IModule( Modules::I2CMuxModule ) {
	_mux[ iADDR ] = 0x70 + offset;
	_mux[ 1 ] = 0x29;
	_mux[ 2 ] = 0x29;
	_mux[ iSEG0 ] = 0x29;
	_mux[ iSEG1 ] = 0x29;
	_mux[ iSEG2 ] = 0x29;
	_mux[ iSEG3 ] = 0x29;
	_mux[ iSEG4 ] = 0x29;
	_mux[ iSEG5 ] = 0x29;
	epsilon[ iC ] = 100;	// clear
	epsilon[ iR ] = 16;	// red
	epsilon[ iG ] = 16;	// green
	epsilon[ iB ] = 16;	// blue

	_object_count += 1;
}

I2CMuxModule::~I2CMuxModule( void ) {
}

void
I2CMuxModule::begin( PATCore *ctx ) {
	_pat = ctx;
	_state = ModuleState::Beginning;
	_pat->log( )->print( "I2CMuxModule	" );
	_pat->log( )->print( _mux[0], HEX );
	if( ctx->hardwarePresent( _mux[0] ) ) {
		_pat->log( )->println( " PRESENT" );
		_state = ModuleState::Ready;
	} else {
		_pat->log( )->println( " absent" );
	}
}

uint32_t
	I2CMuxModule::update( uint32_t elapsed ) {
		if( _state == ModuleState::Absent ) {
			return kForever;
		} else if( _state != ModuleState::Ready ) {
			return 0;
		}

	for( uint8_t sensor_index = 0; sensor_index < 6; ++sensor_index ) {
		if( !tcs[ sensor_index ].isInitialized( ) ) {
			_mux <<(sensor_index + 2);
			tcs[ sensor_index ].setIntegrationTime( tcs34725IntegrationTime_t::TCS34725_INTEGRATIONTIME_24MS );
			_mux <<(sensor_index + 2);
			tcs[ sensor_index ].setGain( tcs34725Gain_t::TCS34725_GAIN_4X );
		}
		
		_mux <<( sensor_index + 2 );

		tcs[ sensor_index ].getRawData( &this_r[ sensor_index ], &this_g[ sensor_index ], &this_b[ sensor_index ], &this_c[ sensor_index ]);
		
		delta_c[ sensor_index ] = abs( this_c[ sensor_index ] - last_c[ sensor_index ] );
		delta_r[ sensor_index ] = abs( this_r[ sensor_index ] - last_r[ sensor_index ] );
		delta_g[ sensor_index ] = abs( this_g[ sensor_index ] - last_g[ sensor_index ] );
		delta_b[ sensor_index ] = abs( this_b[ sensor_index ] - last_b[ sensor_index ] );
		
		if( delta_c[ sensor_index ] > epsilon[ iC ] || delta_r[ sensor_index ] > epsilon[ iR ] || delta_g[ sensor_index ] > epsilon[ iG ] || delta_b[ sensor_index ] > epsilon[ iB ] ) {
			_pat->buildMessage( '[' );
			_pat->buildMessage( _mux[ iADDR ] );
			_pat->buildMessage( ']' );
			_pat->buildMessage( '[' );
			_pat->buildMessage( sensor_index );
			_pat->buildMessage( ']' );
			_pat->buildMessage( 'C' );
			_pat->buildMessage( highByte( this_c[ sensor_index ] ) );
			_pat->buildMessage( lowByte( this_c[ sensor_index ] ) );
			_pat->buildMessage( 'R' );
			_pat->buildMessage( highByte( this_r[ sensor_index ] ) );
			_pat->buildMessage( lowByte( this_r[ sensor_index ] ) );
			_pat->buildMessage( 'G' );
			_pat->buildMessage( highByte( this_g[ sensor_index ] ) );
			_pat->buildMessage( lowByte( this_g[ sensor_index ] ) );
			_pat->buildMessage( 'B' );
			_pat->buildMessage( highByte( this_b[ sensor_index ] ) );
			_pat->buildMessage( lowByte( this_b[ sensor_index ] ) );

			_pat->log( )->print( "segment = " );
			_pat->log( )->print( sensor_index );
			_pat->log( )->print( " rawC = " );
			_pat->log( )->print( this_c[ sensor_index ] );
			_pat->log( )->print( " rawR = " );
			_pat->log( )->print( this_r[ sensor_index ] );
			_pat->log( )->print( " rawG = " );
			_pat->log( )->print( this_g[ sensor_index ] );
			_pat->log( )->print( " rawB = " );
			_pat->log( )->println( this_b[ sensor_index ] );
			_pat->writeMessage( module );

			last_c[ sensor_index ] = this_c[ sensor_index ];
			last_r[ sensor_index ] = this_r[ sensor_index ];
			last_g[ sensor_index ] = this_g[ sensor_index ];
			last_b[ sensor_index ] = this_b[ sensor_index ];
			
			return 24;
		}

	}
	if( !isWaitDone( elapsed ) ) {
		return remainingWait( );
	}
	return 130;
}

/*
	 A DigitalRead or DigitalWrite command will allow you to select a slave device
	 I'mma pull some hinky shit in here and make this specific to Maze of Games by
	 calling the color sensors directly. I ABSOLUTELY need to make this fit a more
	 general scenario, but I want to have something functional very soon!
*/
bool
I2CMuxModule::receiveCommand( Bytecodes command, uint8_t *buffer, uint32_t size ) {
	bool handled = true;
	_pat->log( )->print( "TCA9548A receiveCommand( " );
	_pat->log( )->print( static_cast<uint8_t>( command ), HEX );
	_pat->log( )->print( " ) : " );
	_pat->log( )->print( "with a " );
	_pat->log( )->print( size );
	_pat->log( )->println( "-byte payload of " );
	for( uint8_t i = 0; i < size; ++i ) {
		_pat->log( )->print( "[" );
		_pat->log( )->print( i );
		_pat->log( )->print( "] = " );
		_pat->log( )->print( buffer[ i ], HEX );
		_pat->log( )->println( "\n" );
	}

	uint8_t sensor_index = buffer[0];
	uint8_t slave_dest = buffer[1];
	Bytecodes slave_command = static_cast<Bytecodes>( buffer[2] );

	switch( command ) {
		case Bytecodes::DigitalRead:
		case Bytecodes::DigitalWrite:
			if( size < 3 ) {
				handled = false;
				return handled;
			} else {
				_pat->log( )->print( "...TCA9548A::port( " );
				_pat->log( )->print( sensor_index, HEX );
				_pat->log( )->print( " ) " );
				_pat->log( )->print( "received destination " );
				_pat->log( )->print( slave_dest, HEX );
				if( slave_dest == static_cast<uint8_t>( Modules::TCS34725 ) ) {
					if( !tcs[ sensor_index ].isInitialized( ) ) {
						_mux << sensor_index;
						tcs[ sensor_index ].begin( );
					}
					_pat->log( )->print( ", and command " );
					_pat->log( )->println( static_cast<uint8_t>( slave_command ), HEX );

					switch( slave_command ) {
						case Bytecodes::DataReport:
							handled = true;
							_pat->buildMessage( 'C' );
							_pat->buildMessage( highByte( this_c[ sensor_index ] ) );
							_pat->buildMessage( lowByte( this_c[ sensor_index ] ) );
							_pat->buildMessage( 'R' );
							_pat->buildMessage( highByte( this_r[ sensor_index ] ) );
							_pat->buildMessage( lowByte( this_r[ sensor_index ] ) );
							_pat->buildMessage( 'G' );
							_pat->buildMessage( highByte( this_g[ sensor_index ] ) );
							_pat->buildMessage( lowByte( this_g[ sensor_index ] ) );
							_pat->buildMessage( 'B' );
							_pat->buildMessage( highByte( this_b[ sensor_index ] ) );
							_pat->buildMessage( lowByte( this_b[ sensor_index ] ) );
							_pat->log( )->print( "handled = " );
							_pat->log( )->println( handled );
							_pat->log( )->print( " rawC = " );
							_pat->log( )->print( this_c[ sensor_index ] );
							_pat->log( )->print( " rawR = " );
							_pat->log( )->print( this_r[ sensor_index ] );
							_pat->log( )->print( " rawG = " );
							_pat->log( )->print( this_g[ sensor_index ] );
							_pat->log( )->print( " rawB = " );
							_pat->log( )->println( this_b[ sensor_index ] );
							_pat->writeMessage( module );
							return handled;
						default:
							_pat->log( )->println( "invalid slave command" );
					}
				default:
					handled = false;
					break;
				}
			}
	}

	return handled;
}



