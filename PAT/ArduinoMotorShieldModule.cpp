#include "Arduino.h"
#include "ArduinoMotorShieldModule.h"


#define MOTORSHIELD_ADDRESS		 0x60	//base address for thing

ArduinoMotorShieldModule::ArduinoMotorShieldModule ( ) : IModule( Modules::ArduinoMotorShield ) {
	
}


bool ArduinoMotorShieldModule::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size )
{
	if ( size < 1 )
		return false;
	bool handled = true;
	 
	_pat->log( )->println( "ArduinoMotorShieldModule	- Received a message" );
	Serial.println( ( uint8_t )command, HEX );
	uint8_t sent = 0xFF;
	int i2cAddressOffset = 0; //The first three dispensers go to base slave address, second three go to address + 1
	

	switch ( command )
	{
		case Bytecodes::StepperMotorStep:
		{			
			byte motorIndex = 0,
					 steps = 0,
					 direction = 0;
			String stringFromBuffer = ( char* )buffer;
			String stringFromMotorIndex = stringFromBuffer.substring( 0,2 );
			String stringFromSteps = stringFromBuffer.substring( 2,4 );
			String stringFromDirection = stringFromBuffer.substring( 4 );
//			Serial.println( stringFromBuffer );
//			Serial.print	( strtol( stringFromMotorIndex.c_str( ), NULL, 16 ) );
			motorIndex = ( byte )strtol( stringFromMotorIndex.c_str( ), NULL, 16 );
//			Serial.print	( " " );
//			Serial.print	( strtol( stringFromSteps.c_str( ), NULL, 16 ) );
			steps = ( byte )strtol( stringFromSteps.c_str( ), NULL, 16 );
//			Serial.print	( " " );
//			Serial.print	( strtol( stringFromDirection.c_str( ), NULL, 16 ) );
			direction = ( byte )strtol( stringFromDirection.c_str( ), NULL, 16 );
//			Serial.println( " " );
			motors[motorIndex]->step( steps,direction,INTERLEAVE );
		}
	break;
		default:
			handled = false;
		break;
	}

	if ( sent != 0xFF )
	{
		Serial.print( "Wire.endTransmission( ) = " );
		Serial.println( sent, HEX );
	}

	return handled;
}

ArduinoMotorShieldModule::~ArduinoMotorShieldModule ( )
{

}

uint32_t ArduinoMotorShieldModule::update( uint32_t elapsed )
{
	if ( _state == ModuleState::Absent )
		return kForever;
	else if ( _state != ModuleState::Ready )
		return 0;

	return 150;
}


void ArduinoMotorShieldModule::begin( PATCore* ctx )
{
	IModule::begin( ctx );

	_state = ModuleState::Absent; //Overwritten if any is ready 
	
	for ( int i = 0; i <= sizeof( AFMS )/sizeof( AFMS[0] ); ++i )
	{
		_pat->log( )->print( "Motor Shield( " );
		_pat->log( )->print( i, DEC );
		if ( ctx->hardwarePresent( MOTORSHIELD_ADDRESS + i ) )
		{
				_pat->log( )->println( " ) PRESENT!" );		
				_state = ModuleState::Ready;
				AFMS[i] = Adafruit_MotorShield( MOTORSHIELD_ADDRESS + i ); 
				motors[i*2] = AFMS[i].getStepper( 512, 1 );
				motors[i*2 + 1] = AFMS[i].getStepper( 512, 2 );
				//Wire.beginTransmission( MOTORSHIELD_ADDRESS + i );
				//Wire.write( 0x3F );	//Currently, this does nothing, just like in the Tiki Module
				//Wire.endTransmission( ); 
		} else _pat->log( )->println( " ) absent" );
	}
}

void ArduinoMotorShieldModule::end( )
{
	IModule::end( );
}
