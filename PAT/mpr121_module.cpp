#include "Arduino.h"
#include "mpr121_module.h"
#include "mpr121_constants.h"

MPR121Module::MPR121Module( ) 
	 : IModule( Modules::MPR121 ) {
		_address = 0x5A;
		status = 0;
		lastSend = -1000;
		_milli = 250;
		_activeMask = 0;
		_lastState = 0;
}

MPR121Module::~MPR121Module( ) {

}

bool MPR121Module::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) {
	if( size < 1 )
		return false;
	bool handled = false;
	 
	switch( command )
	{
		case Bytecodes::MPR121SetPins:
		{
			int numPins =( int )buffer[0];
			if( numPins <=( size - 1 ) )
			{
				for( int i = 0; i < numPins; i++ )
				{
					_activeMask |=( 1 <<( int )buffer[i + 1] );
				}
			}
		}
	break;

		case Bytecodes::MPR121SetThresholds:
		{
			int pin =( int )buffer[0];
			_activeMask |=( 1 << pin );

			setThresholds( pin, buffer[1], buffer[2] );
		}
	break;

		case Bytecodes::MPR121Recalibrate:
			recalibrate( );
		break;

			case Bytecodes::MPR121Reset:
				resetChip( );
			break;

		default:
		break;
	}

	return handled;
}

void MPR121Module::resetChip( ) {
	mpr121_setup( );
}

void MPR121Module::recalibrate( ) {
	set_register( _address, ELE_CFG, 0x00 );
	set_register( _address, ATO_CFG0, 0x0B );
	set_register( _address, ATO_CFGU, 0xC9 );	// USL =( Vdd-0.7 )/vdd*256 = 0xC9 @3.3V	 
	set_register( _address, ATO_CFGL, 0x82 );	// LSL = 0.65*USL = 0x82 @3.3V
	set_register( _address, ATO_CFGT, 0xB5 );	// Target = 0.9*USL = 0xB5 @3.3V	
	set_register( _address, ELE_CFG, 0x8F );
}

void MPR121Module::setThresholds( int pin, uint8_t touch, uint8_t release ) {
	set_register( _address, ELE_CFG, 0x00 );
	set_register( _address, ELE0_T +( 2*pin ), TOU_THRESH );
	set_register( _address, ELE0_R +( 2*pin ), REL_THRESH );
	set_register( _address, ELE_CFG, 0x8F ); 
}

void MPR121Module::reportTouch( uint8_t index, bool present, bool interrupt ) {
	_pat->buildMessage( Bytecodes::CapTouchInterrupt );
	_pat->buildMessage( index );
	_pat->buildMessage( present ? 1 : 0 );
	_pat->buildMessage( interrupt ? 1 : 0 );
	_pat->writeMessage( module );
	_pat->log( )->println( "TOUCH CHANGE" );
}

uint32_t MPR121Module::update( uint32_t elapsed ) {
	if( _state == ModuleState::Absent )
		return kForever;
	else if( _state != ModuleState::Ready )
		return 0;

	if( millis( )-lastSend < _milli ) return millis( )-lastSend;
	
	uint16_t touched = readRegister16( MPR121_TOUCHSTATUS_L ) & _activeMask;

	for( int i = 0; i < NUM_PINS; i++ )
	{
		if(( _activeMask &( 1 << i ) ) == 0 )
			continue;

		bool is =(( touched &( 1 << i ) ) ==( 1 << i ) );
		bool was =(( _lastState &( 1 << i ) ) ==( 1 << i ) );

		if( is && !was )
			reportTouch( i, true, true );
		else if( was && !is )
			reportTouch( i, false, true );
	}

	_lastState = touched;
	lastSend = millis( );

	return 150;
}

void MPR121Module::mpr121_setup( void ) {
	set_register( _address, SOFT_RESET, 0x63 );
	// Puts chip into standby mode automatically, or at least we think it does, so let's give it a try

	
	// Section A - Controls filtering when data is > baseline.
	set_register( _address, MHD_R, 0x01 );
	set_register( _address, NHD_R, 0x01 );
	set_register( _address, NCL_R, 0x0E );
	set_register( _address, FDL_R, 0x00 );

	// Section B - Controls filtering when data is < baseline.
	set_register( _address, MHD_F, 0x01 );
	set_register( _address, NHD_F, 0x05 );
	set_register( _address, NCL_F, 0x01 );
	set_register( _address, FDL_F, 0x00 );

	set_register( _address, NHDT, 0x00 );
	set_register( _address, NCLT, 0x00 );
	set_register( _address, FDLT, 0x00 );

	set_register( _address, DEB_CFG, 1 );
	set_register( _address, OTH_CFG, 0x10 ); // default, 16uA charge current
	set_register( _address, FIL_CFG, 0x20 ); // 0.5uS encoding, 1ms period
	
	// Section C - Sets touch and release thresholds for each electrode
	for( uint8_t reg = ELE0_T; reg <= ELE11_T; reg += 0x02 )
	{
		set_register( _address, reg, TOU_THRESH );
		set_register( _address, reg + 1, REL_THRESH );
	}
	
	// Section F
	// Enable Auto Config and auto Reconfig
	set_register( _address, ATO_CFG0, 0x0B );
	set_register( _address, ATO_CFGU, 0xC9 );	// USL =( Vdd-0.7 )/vdd*256 = 0xC9 @3.3V	 
	set_register( _address, ATO_CFGL, 0x82 );	// LSL = 0.65*USL = 0x82 @3.3V
	set_register( _address, ATO_CFGT, 0xB5 );	// Target = 0.9*USL = 0xB5 @3.3V	

	// All done, so switch out of standby mode
	set_register( _address, ELE_CFG, 0x8F );
}


void MPR121Module::set_register( int address, unsigned char r, unsigned char v ) {
		Wire.beginTransmission( address );
		Wire.write( r );
		Wire.write( v );
		Wire.endTransmission( );
}


void MPR121Module::begin( PATCore* ctx ) {
	IModule::begin( ctx );
	if( ctx->hardwarePresent( _address ) )
	{
		mpr121_setup( );
		_state = ModuleState::Ready;	
	}
	else	
	{
			_pat->log( )->print( "absent [" );
			_pat->log( )->print( _address, HEX );
			_pat->log( )->println( "]" );
			_state = ModuleState::Absent;

	}
}

void MPR121Module::end( ) {
	IModule::end( );
}

uint16_t MPR121Module::readCurrentRegister16( ) {
		//read the touch state from the MPR121
		Wire.requestFrom( _address,2 ); 
		
		byte LSB = Wire.read( );
		byte MSB = Wire.read( );
		
		return(( MSB << 8 ) | LSB ); //16bits that make up the touch states
}

uint8_t MPR121Module::readCurrentRegister8( ) {
		Wire.requestFrom( _address,1 );
		return Wire.read( );
}

uint16_t MPR121Module::readRegister16( uint8_t reg ) {
		Wire.beginTransmission( _address );
		Wire.write( reg );
		Wire.endTransmission( false );

		return readCurrentRegister16( );
}

uint8_t MPR121Module::readRegister8( uint8_t reg ) {
		Wire.beginTransmission( _address );
		Wire.write( reg );
		Wire.endTransmission( false );

		return readCurrentRegister8( );
}

uint16_t MPR121Module::getFilteredData( uint8_t t ) {
	if( t > 12 ) return 0;
	return readRegister16( MPR121_FILTDATA_0L + t*2 );
}

uint16_t	MPR121Module::getBaselineData( uint8_t t ) {
	if( t > 12 ) return 0;
	uint16_t bl = readRegister8( MPR121_BASELINE_0 + t );
	return( bl << 2 );
}








