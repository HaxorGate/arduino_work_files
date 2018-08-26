#include "Arduino.h"
#include "gpio_module.h"
//ctor
GPIOModule::GPIOModule() : IModule(Modules::GPIO) {
	for(uint8_t i = 0; i < sizeof(_pins)/sizeof(_pins[0]); ++i ) {
		_address[i] = i;
		
		_pinCount = 16;
	
		_lastRead[i] = 0;
		_thisRead[i] = 0;
		_watchPins[i] = 0;
		_outputPins[i] = 0;
		_outputState[i] = 0;
	}
}//ctor

//ctor with offset
GPIOModule::GPIOModule( uint8_t address_offset ) : IModule(Modules::GPIO) {
	Serial.print( "GPIOModule::GPIOModule( " );
	static uint8_t index = 0;
	_address[ index ] = address_offset;
	Serial.print( _address[ index ], HEX );
	Serial.print( " ]\n" ); 
	_pinCount = 16;
	_lastRead[ index ] = 0;
	_thisRead[ index ] = 0;
	_watchPins[ index ] = 0b0000000000000000;
	_outputPins[ index ] = 0;
	_outputState[ index ] = 0;
	++index;
}//ctor with offset

//dtor
GPIOModule::~GPIOModule() {
}//dtor

//begin
void GPIOModule::begin( PATCore* ctx ) {
	IModule::begin( ctx );
	uint8_t count = 0;
	// MCP23017_ADDRESS is provided by Adafruit_MCP23017.h and specifies the base I2C address of the MCP23017 chip.
	// Pins 15 through 17 form a 3-bit offset(15 = LSB) to this.
	for( uint8_t i= 0; i < MAX_GPIO_MODULES; ++i ) {
		present[ i ] = ctx->hardwarePresent( MCP23017_ADDRESS | i );
		if( present[ i ] ) {
			++count;
			_pins[ i ] = new Adafruit_MCP23017( );
			_pins[ i ]->begin( i );
			_state = ModuleState::Ready;
			_pat->log( )->print( "GPIO @" );
			_pat->log( )->print( MCP23017_ADDRESS | i, HEX );
			_pat->log( )->print( " READY" );
			setOutput( 0b0000000000000000, 0, i );
			setWatch( 0b0000000000000000, i );
		} else {
			_pat->log( )->print( "GPIO @" );
			_pat->log( )->print( MCP23017_ADDRESS | i, HEX );
			_pat->log( )->print( " absent\n" );
		}
	}
//		if( present[0] && present[1] ) {
//			_pins[0] = new Adafruit_MCP23017();
//			_pins[0]->begin( 0 );
//			_pins[1] = new Adafruit_MCP23017();
//			_pins[1]->begin( 1 );
//			_state = ModuleState::Ready;
//			_pat->log()->print( "BOTH _pins[0]@ [" );
//			_pat->log()->print( MCP23017_ADDRESS | 0, HEX );
//			_pat->log()->println( "] PRESENT" );
//			_pat->log()->print( "AND	_pins[1]@ [ " );
//			_pat->log()->print( MCP23017_ADDRESS | 1, HEX );
//			_pat->log()->println( "] PRESENT" );
//			
//			setOutput(0xFFFF, 0, 0);
//			setOutput(0xFFFF, 0, 1);
//			setWatch(0xFFFF, 0);
//			setWatch(0xFFFF, 1);
//		} else 
//		if( ! present[0] && ! present[1] ) {
//			_pat->log()->print( "absent [" );
//			_pat->log()->print( MCP23017_ADDRESS | 0, HEX );
//			_pat->log()->println( "]" );
//			_pat->log()->print( " absent [ " );
//			_pat->log()->print( MCP23017_ADDRESS | 1, HEX );
//			_pat->log()->println( "]" );
//			_state = ModuleState::Absent;
//		} else if( present[0] && !present[1] ) {
//			_pins[0] = new Adafruit_MCP23017();
//			_pins[0]->begin( 0 );
//			setOutput(0xFFFF, 0, 0);
//			_state = ModuleState::Ready;
//			_pat->log()->println( "!!!!!!ONLY [0] PRESENT!!!!!!!" );
//		} else if( !present[0] && present[1] ) {
//			_pins[1] = new Adafruit_MCP23017();
//			_pins[1]->begin( 1 );
//			setOutput(0xFFFF, 0, 1);
//			_state = ModuleState::Ready;
//			_pat->log()->println( "!!!!!!!ONLY [1] PRESENT!!!!!!!" );
//		} 
//	}
	if( count == 0 ) {
		_state = ModuleState::Absent;
	}
}//begin

//end
void GPIOModule::end() {
	IModule::end();
}//end

//update
uint32_t GPIOModule::update( uint32_t elapsed ) {
	uint32_t wait = 20;
	for( uint8_t p = 0; p < sizeof( present ) / sizeof( present[ 0 ] ); ++p ) {
		Wire.beginTransmission( MCP23017_ADDRESS | p );
		present[ p ] = Wire.endTransmission( ) == 0;
	}
	if( _state == ModuleState::Absent ) {
		return kForever;
	} else if( _state != ModuleState::Ready ) {
		return 0;
	}
	if( !isWaitDone( elapsed ) ) {
		return remainingWait();
	}
	for( uint8_t i = 0; i < 2; ++i ){
		if( present[ i ] && _watchPins[i] > 0 ) {
			_thisRead[i] = _pins[i]->readGPIOAB();
			/* Much faster to read all at once, but to read one at a time do this:
	
				for(int i = 0; i < _pinCount; i++)
				{
				if(_pins->digitalRead(i) == HIGH)
					_thisRead |=(1 << i);
				}
			*/
			if( _lastRead[i] != _thisRead[i] ) {
				_pat->log()->print("thisRead[");
				_pat->log()->print(i);
				_pat->log()->print("] = ");
				_pat->log()->println(_thisRead[i], BIN);
				_pat->log()->print("lastRead[");
				_pat->log()->print(i);
				_pat->log()->print("] = ");
				_pat->log()->println(_lastRead[i], BIN);
				reportWatchedPins(i);
				_lastRead[i] = _thisRead[i];
			}
		}
		wait = setWait(20);
	}
	return wait;
}//update

//receiveCommand
bool GPIOModule::receiveCommand( Bytecodes command, uint8_t *buffer, uint32_t size ) {
	if( size < 1 || size > 17	) {
		return false;
	}
	uint8_t index = 0;
	String payload = "fedcba9876543210";
	payload = String( reinterpret_cast<char *>(buffer) );
	uint8_t pld_len = payload.length();
	if( pld_len > size ){
		payload.remove(size, pld_len - size);
	}
	if( size == _pinCount*2 ) {
		payload += '0';
	} 
	if( payload.endsWith("0") && present[ 0 ] ) {
		_pat->log()->print("0->");
		index = 0;
	} else if( payload.endsWith("1") && present[ 1 ] ) {
		_pat->log()->print("1->");
		index = 1;
	} else if( payload.endsWith("2") && present[ 2 ] ) {
		_pat->log()->print("2->");
		index = 2;
	} else if( payload.endsWith("3") && present[ 3 ] ) {
		_pat->log()->print("3->");
		index = 3;
	} else if( payload.endsWith("4") && present[ 4 ] ) {
		_pat->log()->print("4->");
		index = 4;
	} else if( payload.endsWith("5") && present[ 5 ] ) {
		_pat->log()->print("5->");
		index = 5;
	} else if( payload.endsWith("6") && present[ 6 ] ) {
		_pat->log()->print("6->");
		index = 6;
	} else if( payload.endsWith("7") && present[ 7 ] ) {
		_pat->log()->print("7->");
		index = 7;
	}
	bool handled = false;
	switch(command) {
		case Bytecodes::DigitalRead:
			index = String( static_cast<char>(buffer[0]) ).toInt();
			_pat->log()->print( "DigitalRead " );
			reportAllPins( index );
			handled = true;
			break;

		case Bytecodes::DigitalWatch:
		{
			uint16_t watch = 0;
			_pat->log()->print( "setWatch" );
			for( int i = 0; i < _pinCount && i < payload.length() - 1; i++ ) {
				if(payload[i] == '1') {
					watch |=(1 << i);				 
				}
			}
			setWatch(watch, index);
		}
			handled = true;
			break;

		case Bytecodes::DigitalWrite:
		{
			_pat->log()->print( "DigitalWrite" );
			uint16_t pins = 0;
			uint16_t state = 0;
			for( int i = 0; i < _pinCount && i < payload.length() - 1; i++ ) {
				uint16_t flag =(1 << i);
				if( payload[i] == '1' ) {
					pins |= flag;
					state |= flag;
				} else if( payload[i] == '0' ) {
					pins |= flag;
				} else if( payload[i] == '=' ) {
					pins |=(_outputPins[ index ] & flag);
					state |=(_outputState[ index ] & flag);
				}
			}			
			setOutput( pins, state, index );
			handled = true;
		}
		break;
		default:
		break;
	}
	return handled;
}//receiveCommand

//reportWatchedPins
void GPIOModule::reportWatchedPins( uint8_t index ) {
	if( present[ index ] ) {
		_pat->log()->print( "GPIOModule::reportWatchedPins \n" );
		_pat->buildMessage(Bytecodes::DigitalWatch);
		_pat->buildMessage(BEGIN_LITERAL);
		for( int i = 0; i < _pinCount; i++ ) {
			uint16_t flag =(1 << i);
			if((_watchPins[ index ] & flag) != flag ) {
				_pat->buildMessage( '_' );
				continue;
			}
			bool was =(_lastRead[ index ] & flag) == flag;
			bool is =(_thisRead[ index ] & flag) == flag;
			if( was && is ) {
				_pat->buildMessage( '+' );
			} else if(was && !is) {
				_pat->buildMessage( 'v' );
			} else if(!was && is) {
				_pat->buildMessage( '^' );
			} else {
				_pat->buildMessage( '-' );
			}
		}
		_pat->buildMessage( END_LITERAL );
		_pat->buildMessage( MCP23017_ADDRESS | index );
		_pat->writeMessage( module );
	} else {
		_pat->log()->print( "No module present at index " );
		_pat->log()->print( index );
		_pat->log()->print( "\n");
		_pat->buildMessage((uint8_t*)("XXXXXXXXXXXXXXXX"), 16 );
		_pat->writeMessage( module );
	}
}//reportWatchedPins

//reportAllPins
void GPIOModule::reportAllPins( uint8_t index ) {
	if( present[ index ] ) {
		_pat->log()->print( "GPIOModule::reportAllPins( " );
		_pat->log()->print( index );
		_pat->log()->print( "<" );
		_pat->log()->print( _watchPins[index], BIN );
		_pat->log()->print( ">" );
		_pat->log()->print( ")\n" );
		Adafruit_MCP23017 *gpio = _pins[ index ];
		_thisRead[ index ] = gpio->readGPIOAB();
	
		_pat->buildMessage(Bytecodes::DigitalRead);
		_pat->buildMessage(BEGIN_LITERAL);
		for( int i = _pinCount - 1; i >= 0; i-- ) {
			uint16_t flag =(1 << i);
	
			if((_watchPins[ index ] & flag) == flag) { // Check all the input pins
				bool was =(_lastRead[ index ] & flag) == flag; //Was the pin up or down last frame
				bool is =(_thisRead[ index ] & flag) == flag; //Is the pin up or down right now
	
				if(was && is) { //if the pin was up, and still is
					_pat->buildMessage('+');
				} else if(was && !is) { // if the pin JUST went down
					_pat->buildMessage('v');
				} else if(!was && is) { // If the pin JUST went up
					_pat->buildMessage('^');
				} else { //If the pin was down, and still is
					_pat->buildMessage('-');
				}
			} else if((_outputPins[ index ] & flag) == flag) { // check all the output pins
				_pat->buildMessage(((_outputState[ index ] & flag) == flag) ? '*' : '.'); // * for UP, . for DOWN
			}
			else { //All unused pins.
				_pat->buildMessage(((_thisRead[ index ] & flag) == flag) ? '1' : '0');
				continue;
			}
		}
		_pat->buildMessage( END_LITERAL );
		_pat->buildMessage( MCP23017_ADDRESS |	index );
		_pat->writeMessage(module);
	} else {
		_pat->log()->print( "No module present at index " );
		_pat->log()->println( index );
		_pat->buildMessage((uint8_t*)("XXXXXXXXXXXXXXXX"), 16 );
		_pat->writeMessage( module );
	}
}//reportAllPins

//setWatch
void GPIOModule::setWatch(uint16_t newWatch, uint8_t index = 0 ) {
	if( present[ index ] ) {
		_pat->log()->print( "GPIOModule::setWatch( " );
		_pat->log()->print( newWatch );
		_pat->log()->print( ", " );
		_pat->log()->print( index );
		_pat->log()->print( ")\n" );
		Adafruit_MCP23017 *gpio = _pins[ index ];
		for( int i = 0; i < _pinCount; i++ ) {
			uint16_t flag =(1 << i);//Setting one bit up at a time. E.g 0001, 0010, 0100, 1000
			if((newWatch & flag) == flag ) { // Is the bit we set, also set in newWatch E.g. 0101 & 0001 = 0001
				gpio->pinMode(i, INPUT);
				gpio->pullUp(i, HIGH);
			} else {
				gpio->pinMode(i, OUTPUT);
			}
		}
		_watchPins[ index ] = newWatch;
	} else {
		_pat->log()->print( "No module present at index " );
		_pat->log()->println( index );
		_pat->buildMessage((uint8_t *)("XXXXXXXXXXXXXXXX"), 16 );
		_pat->writeMessage( module );
	}
}//setWatch

//setOutput
void GPIOModule::setOutput( uint16_t pins, uint16_t state, uint8_t index = 0 ) {
	if( present[ index ] ) {
		_pat->log()->print( "GPIOModule::setOutput( " );
		_pat->log()->print( pins );
		_pat->log()->print( ", " );
		_pat->log()->print( state );
		_pat->log()->print( ", " );
		_pat->log()->print( index );
		_pat->log()->print( ")\n" );
		Adafruit_MCP23017 *gpio = _pins[ index ];
		for( int i = _pinCount - 1; i >= 0; i-- ) {
			uint16_t flag =(1 << i);
	
			if((pins & flag) == flag) { //if pin is set
				gpio->pinMode(i, OUTPUT);
				gpio->digitalWrite(i,((state & flag) == flag) ? HIGH : LOW); //if state is set, set pin to HIGH
			}
		}
	
		_outputPins[ index ] = pins;
		_outputState[ index ] = state;
	} else {
		_pat->log()->print( "No module present at index " );
		_pat->log()->println( index );
		_pat->buildMessage((uint8_t *)("XXXXXXXXXXXXXXXX"), 16 );
		_pat->writeMessage( module );
	}
}//setOutput
