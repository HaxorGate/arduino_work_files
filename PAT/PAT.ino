// Experimenting with this for Maze. If it's useful, let's use it in other rooms.
// EspSaveCrash saves WDT reset information into flash memory. It can be accessed
// after the crash to see why it happened. I'm going to try to send the crash data
// to the server on initialization so that we won't need to pull the PAT to analyze.
////THIS USES THE SAME SPACE IN FLASH AS THE NETWORK INFO. ONE OF THOSE WILL NEED 
////MODIFICATION BEFORE WE CAN USE THIS
//#include <EspSaveCrash.h>

//#define USE_NODEMCU

/* [Epic Adventures] PUZZLE ACTIVITY TERMINAL
	// Software Core
	//
	// KEEP THIS FILE AS EMPTY AS POSSIBLE.
	//
	// Module rules:
	// * Each module should live in its own .h/.c pair( plus whatever other files are needed )
	// * Include the module's .h in the section marked below
	// * Immediately above the .h, include all the libraries the module needs.
	//	 - THIS MUST BE DONE HERE, not in your file and not in a .h, because of how the Arduino IDE works.
	//	 - Include ALL the libraries you need, even if you see them here already...
	//		the exception is the ones in the "Core Libraries" block.
	// * In setup( ), call _pat.register( new YourModule( ) ) BEFORE _pat.begin( )
	// * When the system is ready, your module's begin( ) will be called.
	//	 - In your begin( ), review the I2C bus to see whether your hardware is present. If not, allocate as little memory as possible.
*/

// CORE LIBRARIES
#include <EEPROM.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <PCFlash.h>
#include <PuzzleClient.h>

#ifdef USE_NODEMCU
#define FASTLED_ESP8266_D1_PIN_ORDER
#endif
#include <FastLED.h>

#include "pat_core.h"


// For each module:
// Module Name
// Purpose & Required Hardware
// #include library dependencies
// #include "modulename.h"

// RFID Module
// The RFID Module reads RFID cards
#include "rfid_module.h"
// end RFIDModule

// GPIO Module
// The GPIO Module provides 16 pins capapble of simple digital input or output
//#include "C:\Users\ETAStaff\Documents\Arduino\libraries\Adafruit_MCP23017\Adafruit_MCP23017.h"
#include <Adafruit_MCP23017.h>
#include "gpio_module.h"
// end GPIOModule

// Matrix Module
// The Matrix Module uses 16 GPIO pins in a special configuration to read matrixed keypads( e.g. 12-button telephone dials )
//#include "C:\Users\ETAStaff\Documents\Arduino\libraries\Adafruit_MCP23017\Adafruit_MCP23017.h"
//#include <Adafruit_MCP23017.h>
//#include "matrix_module.h"
// end MatrixModule

// Tiki Module
// The Tiki Module controls the different aspects of the Tikis
//#include "tiki_module.h"
// end TikiModule

// MOSFET Module
// The MOSFET Module controls banks of MOSFETs for switching high-current loads( such as white LEDs or solenoids )
#include "mosfet_module.h"
// end MOSFETModule

// GemDispenser Module
// The GemDispenser Module controls a Gem Dispenser( IR Sensor and Stepper Motor Controller ) via a Metro 328
//#include "gem_dispenser_module.h"
// end GemDispenser

// captouch Module
// The captouch Module controls "homebrew" capacitive touch devices
//#include "captouch_module.h"
// end captouch

// captouch Module
// The captouch Module controls "homebrew" capacitive touch devices
//#include "captouchmulti_module.h"
// end captouch

// MPR121 Module
// The MPR121 module controls an MPR121 capacitive touch sensor module
//#include "mpr121_module.h"
// end MPR121

// LSM303 Module
// The LSM303 module controls an LSM303 accelerometer/magnetometer sensor module
//#include "lsm303_module.h"
//#include <Adafruit_Sensor.h>
//#include <Adafruit_LSM303.h>
// end LSM303

// TCS34725 Module
// The TCS34725 module controls a TCS34725 color sensor module
#include "tcs34725_module.h"
#include <Adafruit_TCS34725.h>
// end TCS34725

// ArduinoMotorShieldModule module
// The ArduinoMotorShieldModule module controls any number of motor shields and their stepper motors
//#include "ArduinoMotorShieldModule.h"
//#include <Adafruit_MotorShield.h>
// end ArduinoMotorShieldModule

// DotstarModule module
// The DotstarModule module controls Dotstars
#include "DotstarModule.h"
// end DotstarModule


// I2CMuxModule module
// The I2CMux is a TCA9548A multiplexer that routes I2C signals from it's address( 0x70 ) to	up to 8
// destination devices. This lets us have multiple devices with the same address acting independently
// on the I2C bus.
#include	"tca9548a.h"
#include	"I2CMuxModule.h"

// end I2CMuxModule
PATCore _pat;
PCFlash* _flash;
uint32_t _tickCount;
uint32_t _interval;

#define LINE( name,val ) Serial.print( name ); Serial.print( "\t" ); Serial.println( val );
////////////////////////////////////
#include "NetworkVariables.h"
extern String _buffer_		 = "                                                                                  ";
extern String _ssid_			 = "                                                                                  ";
extern String _password_	 = "                                                                                  ";
extern String _host_			 = "                                                                                  ";
extern String _patName_	   = "                                                                                  ";
extern String _node_       = "                                                                                  ";
////////////////////////////////////
void setup( ) {
	uint8_t eeSize = 70;
	Serial.begin( 115200 );
	EEPROM.begin( eeSize );
	_buffer_.reserve( eeSize );
	Serial.println( "Checking EEPROM For Network Variables..." );
	uint8_t eeprom_it, beg, end;
	for( eeprom_it = 0; eeprom_it < eeSize; ++eeprom_it ) {
		char e = EEPROM.read( eeprom_it );
		_buffer_.setCharAt( eeprom_it, e );
	}

	beg = 0;
	end = _buffer_.indexOf( ' ', beg );
	_ssid_ = _buffer_.substring( beg, end );
	_ssid_.trim( );

	beg = end + 1;
	end = _buffer_.indexOf( ' ', beg );
	_password_ = _buffer_.substring( beg, end );
	_password_.trim( );

	beg = end + 1;
	end = _buffer_.indexOf( ' ', beg );
	_host_ = _buffer_.substring( beg, end );
	_host_.trim( );

	beg = end + 1;
	end = _buffer_.indexOf( ' ', beg );
	_patName_ = _buffer_.substring( beg, end );
	_patName_.trim( );

	beg = end + 1;
	end = _buffer_.indexOf( ' ', beg );
	_node_ = _buffer_.substring( beg, end );
	_node_.trim( );

	Serial.print( _buffer_ );
	Serial.println( "{" );
	Serial.print( '\t' );
	Serial.println( _ssid_ );
	Serial.print( '\t' );
	Serial.println( _password_ );
	Serial.print( '\t' );
	Serial.println( _host_ );
	Serial.print( '\t' );
	Serial.println( _patName_ );
	Serial.print( '\t' );
	Serial.println( _node_ );
	Serial.println( "}" );

	LINE( "__FILE__", __FILE__ );
	LINE( "__DATE__", __DATE__ );
	LINE( "__TIME__", __TIME__ );

	uint32_t begun = millis( );

	Serial.print( "Checking flash..." );
	_flash = new PCFlash( );
	Serial.println( " complete." );

	Serial.print( "Boot delay to allow devices to settle..." );
	uint32_t idle = millis( ) - begun;
	if( idle < 4000 )
		delay( 4000 - idle );
	Serial.println( " complete." );

	// Object creation
	_pat.signup( new RFIDModule( ) );
	_pat.signup( new GPIOModule(0) );
	_pat.signup( new GPIOModule(1) );
//	_pat.signup( new MatrixModule( ) );
//	_pat.signup( new TikiModule( ) );

//	_pat.signup( new MPR121Module( ) );
//	_pat.signup( new CapTouchModule( ) );
//	_pat.signup( new CapTouchMultiModule( ) );

	_pat.signup( new MOSFETModule( ) );
//	_pat.signup( new GemDispenserModule( ) );
//	_pat.signup( new LSM303Module( ) );
//	_pat.signup( new TCS34725Module( ) );

	_pat.signup( new DotstarModule( ) );
	_pat.signup( new I2CMuxModule(0) );
	_pat.signup( new I2CMuxModule(1) );
	_pat.signup( new I2CMuxModule(2) );
	_pat.signup( new I2CMuxModule(3) );
	_pat.signup( new I2CMuxModule(4) );
//	_pat.signup( new ArduinoMotorShieldModule( ) );
	// Start up the system
	_pat.begin( _flash );
	_tickCount = millis( );
	_interval = 0;
}

void loop( ) {
	uint32_t now = millis( );
	uint32_t elapsed = 0;

	if( now < _tickCount )
	{
		// Did we wrap around?
		uint32_t oldDelta = UINT32_MAX - _tickCount;
		if( oldDelta < kForever )
		{
			// Plausible that we did wrap around, so assume that
			elapsed = now + oldDelta;
		}
	}
	else
	{
		elapsed = now - _tickCount;
	}

	if( elapsed >= _interval )
	{
		_tickCount = millis( );
		_interval = _pat.update( elapsed );
	}
}
