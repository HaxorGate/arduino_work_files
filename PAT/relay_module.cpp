#include "Arduino.h"
#include "relay_module.h"

RelayModule::RelayModule( ) //Currently the same as the GPIO module. Is separate to allow for modification
	 : IModule( Modules::Relay ) {
	_address = 2;
	_pinCount = 16;

	_lastRead = 0;
	_thisRead = 0;
	_watchPins = 0;
	_outputPins = 0;
	_outputState = 0;
}

RelayModule::~RelayModule( ) {

}
//Grabs all the input pins
void RelayModule::setWatch( uint16_t newWatch ) {
	for( int i = 0; i < _pinCount; i++ )
	{
		uint16_t flag =( 1 << i );//Setting one bit up at a time. E.g 0001, 0010, 0100, 1000

		if(( newWatch & flag ) == flag ) //Is the bit we set, also set in newWatch E.g. 0101 & 0001 = 0001
		{
			_pins->pinMode( i, INPUT );
			_pins->pullUp( i, HIGH );
		}
		else
			_pins->pinMode( i, OUTPUT );
	}

	_watchPins = newWatch;
}
//Grabs all the output pins
void RelayModule::setOutput( uint16_t pins, uint16_t state ) {
	for( int i = 0; i < _pinCount; i++ )
	{
		uint16_t flag =( 1 << i );

		if(( pins & flag ) == flag ) //if pin is set
		{
			_pins->pinMode( i, OUTPUT );
			_pins->digitalWrite( i,(( state & flag ) == flag ) ? HIGH : LOW ); //if state is set, set pin to HIGH
		}
	}

	_outputPins = pins;
	_outputState = state;
}

void RelayModule::reportAllPins( ) {
	uint16_t read = _pins->readGPIOAB( );

	_pat->buildMessage( Bytecodes::DigitalRead );
	_pat->buildMessage( BEGIN_LITERAL );
	for( int i = 0; i < _pinCount; i++ )
	{
		uint16_t flag =( 1 << i );

		if(( _watchPins & flag ) == flag ) //Check all the input pins
		{
			bool was =( _lastRead & flag ) == flag; //Was the pin up or down last frame
			bool is =( read & flag ) == flag; //Is the pin up or down right now

			if( was && is ) //if the pin was up, and still is
				_pat->buildMessage( '+' );
			else if( was && !is ) //if the pin JUST went down
				_pat->buildMessage( 'v' );
			else if( !was && is ) //If the pin JUST went up
				_pat->buildMessage( '^' );
			else //If the pin was down, and still is
				_pat->buildMessage( '-' );
		}
		else if(( _outputPins & flag ) == flag ) //check all the output pins
		{
			_pat->buildMessage((( _outputState & flag ) == flag ) ? '*' : '.' ); // * for UP, . for DOWN
		}
		else //All unused pins.
		{
			_pat->buildMessage((( read & flag ) == flag ) ? '1' : '0' ); 
			continue;
		}
	}
	_pat->writeMessage( module );
}

bool RelayModule::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) {
	if( size < 1 )
		return false;

	bool handled = false;
	switch( command )
	{
		case Bytecodes::DigitalRead:
			reportAllPins( );
			handled = true;
			break;

		case Bytecodes::DigitalWatch:
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

		case Bytecodes::DigitalWrite:
			{
				uint16_t pins = 0;
				uint16_t state = 0;

				for( int i = 0; i < _pinCount && i < size; i++ )
				{
					uint16_t flag =( 1 << i );
					if( buffer[i] == '1' )
					{
						pins |= flag;
						state |= flag;
					}
					else if( buffer[i] == '0' )
					{
						pins |= flag;
					}
					else if( buffer[i] == '=' )
					{
						pins |=( _outputPins & flag );
						state |=( _outputState & flag );
					}
				}
				setOutput( pins, state );
				handled = true;
			}
			break;

		default:
			break;
	}

	return handled;
}

void RelayModule::reportWatchedPins( ) {
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

		bool was =( _lastRead & flag ) == flag;
		bool is =( _thisRead & flag ) == flag;

		if( was && is )
			_pat->buildMessage( '+' );
		else if( was && !is )
			_pat->buildMessage( 'v' );
		else if( !was && is )
			_pat->buildMessage( '^' );
		else
			_pat->buildMessage( '-' );
	}
	_pat->writeMessage( module );
}


uint32_t RelayModule::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent )
		return kForever;
	else if( _state != ModuleState::Ready )
		return 0;

	if( !isWaitDone( elapsed ) )
		return remainingWait( );

	if( _watchPins > 0 )
	{
		_thisRead = _pins->readGPIOAB( );

		/* Much faster to read all at once, but to read one at a time do this:

		for( int i = 0; i < _pinCount; i++ )
		{
			if( _pins->digitalRead( i ) == HIGH )
				_thisRead |=( 1 << i );
		}
		*/

		if( _lastRead != _thisRead )
		{
			reportWatchedPins( );
			_lastRead = _thisRead;
		}

		return setWait( 20 );
	}

	return setWait( 50 );
}

void RelayModule::begin( PATCore* ctx ) {
	IModule::begin( ctx );
	bool present = ctx->hardwarePresent( MCP23017_ADDRESS + _address );
	// MCP23017_ADDRESS is provided by Adafruit_MCP23017.h and specifies the base I2C address of the MCP23017 chip.
	// Pins 15 through 17 form a 3-bit offset( 15 = LSB ) to this.

	_pat->log( )->print( "Relay Module " );
	if( present )
	{
		_pins = new Adafruit_MCP23017( );
		_pins->begin( _address );

		// TODO Not sure this is always the right answer, but do it for now...
		setOutput( 0xFFFF, 0 );
		_state = ModuleState::Ready;

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

void RelayModule::end( ) {
	IModule::end( );
}


