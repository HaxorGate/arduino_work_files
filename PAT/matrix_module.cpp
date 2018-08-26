#include "Arduino.h"
#include "matrix_module.h"

MatrixModule::MatrixModule( ) //Used to monitor key presses
	 : IModule( Modules::Matrix ) {
	_address = 1;
	_pinCount = 16;

	_lastPress = 0;
	_thisPress = 0;
	_lastRead = 0;
	_thisRead = 0;
	_watchPins = 0;
	_matrixPins = 0;
	_strobeColumn = 0;
	_numColumns = 0;

	_lastKeyDown = -1;

	memset( _configuration, 0, 16 );
}

MatrixModule::~MatrixModule( ) {

}

void MatrixModule::setWatch( uint16_t newWatch ) {
	for( int i = 0; i < _pinCount; i++ )
	{
		uint16_t flag =( 1 << i );
		if(( _matrixPins & flag ) == flag )// if the matrix pin is set, do not set the watch pin
			continue;

		if(( newWatch & flag ) == flag )
		{
			_pins->pinMode( i, INPUT );
			_pins->pullUp( i, HIGH );
		}
	}

	_watchPins = newWatch;
}

void MatrixModule::setConfiguration( uint8_t* buffer, uint32_t size ){ //size is the size of the buffer 
	if( size < 2 )
		return;

	_matrixPins = 0;
	_numRows = 0;

	_repeat = buffer[0] > 0; //first byte is whether or not the system should resend button presses if the button is held down
	buffer++;
	size--;

	_numColumns = buffer[0]; //second byte is the number of columns, everything else is pin states
	if( _numColumns == 0 || size <( _numColumns + 1 ) )
		return;
	_numRows = size -( _numColumns + 1 );

	memcpy( _configuration, buffer + 1, size - 1 );

	for( int i = 0; i < _numColumns; i++ )
	{
		_pins->pinMode( _configuration[i], INPUT );		// when not strobing, set as input to decouple	
		_pins->digitalWrite( _configuration[i], HIGH );
		_matrixPins |=( 1 << _configuration[i] );
	}

	for( int i = 0; i < _numRows; i++ )
	{
		_pins->pinMode( _configuration[_numColumns + i], INPUT ); // set each row to input to measure if key is pressed
		_pins->pullUp( _configuration[_numColumns + i], HIGH );
		_matrixPins |=( 1 << _configuration[_numColumns + i] );
	}

	_strobeColumn = 0; //Sets a start position for which column we are currently applying power to
	_pins->pinMode( _configuration[_strobeColumn], OUTPUT );		// when not strobing, set as input to decouple	
	_pins->digitalWrite( _configuration[_strobeColumn], LOW ); // turn power off on the currently strobed column
}

void MatrixModule::dumpConfiguration( ) {//print out the configuration 
	_pat->log( )->print( "_numColumns " );
	_pat->log( )->println( _numColumns );

	for( int i = 0; i < _numColumns; i++ )
	{
		_pat->log( )->print( " " );
		_pat->log( )->print( _configuration[i] );
	}
	_pat->log( )->println( );

	_pat->log( )->print( "_numRows " );
	_pat->log( )->println( _numRows );

	for( int i = 0; i < _numRows; i++ )
	{
		_pat->log( )->print( " " );
		_pat->log( )->print( _configuration[i + _numColumns] );
	}
	_pat->log( )->println( );

	_pat->log( )->println( _matrixPins, HEX );
}

bool MatrixModule::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) {
	if( size < 1 )
		return false;

	bool handled = false;
	switch( command )
	{
		case Bytecodes::DigitalWatch: //use the buffer to set the watchPins
			{
				uint16_t watch = 0;
				for( int i = 0; i < _pinCount && i < size; i++ )
				{
					if( buffer[i] == '1' )
					{
						watch |=( 1 << i );
					}
				}
				setWatch( watch );
			}
			handled = true;
			break;

		case Bytecodes::Configure: //use the buffer to set the configuration
			setConfiguration( buffer, size );
			handled = true;
			break;

		default:
			break;
	}

	return handled;
}

void MatrixModule::reportWatchedPins( ) {
	_pat->buildMessage( Bytecodes::DigitalWatch );
	_pat->buildMessage( BEGIN_LITERAL );
	for( int i = 0; i < _pinCount; i++ )
	{
		uint16_t flag =( 1 << i );

		if(( _watchPins & flag ) != flag )
		{
			_pat->buildMessage( '_' );
			continue;
		}

		bool was =( _lastRead & flag ) == flag; //Was the pin up or down last frame
		bool is =( _thisRead & flag ) == flag; //Is the pin up or down right now

		if( was && is ) //if the pin was up, and still is
			_pat->buildMessage( '+' );
		else if( was && !is ) //if the pin JUST went down
			_pat->buildMessage( 'v' );
		else if( !was && is ) //If the pin JUST went up
			_pat->buildMessage( '^' );
		else //If the pin was down, and still is
			_pat->buildMessage( '-' );
	}
	_pat->writeMessage( module );
}

void MatrixModule::reportKeyDown( int code ) {
	_pat->buildMessage( Bytecodes::KeyDown );
	_pat->buildMessage( code );
	if( _lastKeyDown >= 0 && _lastKeyDown != code )
		_pat->buildMessage( _lastKeyDown );
	_pat->writeMessage( module );

	_lastKeyDown = code;
}

void MatrixModule::reportKeyUp( ) {
	_pat->buildMessage( Bytecodes::KeyUp );
	_pat->buildMessage( _lastKeyDown );
	_pat->writeMessage( module );

	_lastKeyDown = -1;
}

uint32_t MatrixModule::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent )
		return kForever;
	else if( _state != ModuleState::Ready )
		return 0;

	if( !isWaitDone( elapsed ) )
		return remainingWait( );

	uint16_t read = _pins->readGPIOAB( );

	_thisRead = read & _watchPins;
	if( _watchPins > 0 )
	{
		if( _lastRead != _thisRead )
		{
			reportWatchedPins( );
			_lastRead = _thisRead;
		}
	}

	read &= _matrixPins;
	int code = -1;
	_thisPress = 0;
	uint16_t strobeMask =( 1 << _configuration[_strobeColumn] );
	for( int i = 0; i < _numRows; i++ )
	{
		uint16_t flag =( 1 << _configuration[_numColumns + i] );
		if(( read & flag ) == 0 )
		{
			_thisPress |= strobeMask;
			_thisPress |=( 1 << _configuration[_numColumns + i] );
			code = _strobeColumn +( i * _numColumns );
			break;
		}
	}

	if( _lastPress == 0 && code >= 0 ) //if no key was recorded being down previously
	{
		if( _repeat || _lastPress != _thisPress ) //if this is a new key, or we want to be told about repeating keys
		{
			reportKeyDown( code );
			_lastPress = _thisPress;
		}
	}
	else if(( _lastPress & strobeMask ) == strobeMask ) //if the key we knew about last time is now up
	{
		// We only clear _lastPress if we're on the same strobe we were then
		reportKeyUp( );
		_lastPress = 0;
	}

	if( _numColumns > 0 )
	{
		_pins->pinMode( _configuration[_strobeColumn], INPUT );
		_pins->pullUp( _configuration[_strobeColumn], HIGH );
		_strobeColumn += 1; //move to the next column
		if( _strobeColumn >= _numColumns )
			_strobeColumn = 0;
		_pins->pinMode( _configuration[_strobeColumn], OUTPUT );
		_pins->digitalWrite( _configuration[_strobeColumn], LOW );
	}

	return setWait( 10 );
}

void MatrixModule::begin( PATCore* ctx ) {
	IModule::begin( ctx );
	bool present = ctx->hardwarePresent( MCP23017_ADDRESS + _address );
	// MCP23017_ADDRESS is provided by Adafruit_MCP23017.h and specifies the base I2C address of the MCP23017 chip.
	// Pins 15 through 17 form a 3-bit offset( 15 = LSB ) to this.

	_pat->log( )->print( "Matrix Module " );
	if( present )
	{
		_pins = new Adafruit_MCP23017( );
		_pins->begin( _address );

		_state = ModuleState::Ready;

		for( int i = 0; i < _pinCount; i++ )
		{
			_pins->pinMode( i, INPUT );
			_pins->pullUp( i, HIGH );
		}

		_pat->log( )->println( "PRESENT" );
	}
	else
	{
		_pat->log( )->print( "absent [" );
		_pat->log( )->print( MCP23017_ADDRESS + _address, HEX );
		_pat->log( )->println( "]" );
		_state = ModuleState::Absent;
	}
}

void MatrixModule::end( ) {
	IModule::end( );
}


