#ifndef mpr121_module_h
#define mpr121_module_h

#include "imodule.h"

class MPR121Module : public IModule {
public:
	MPR121Module( );
	virtual ~MPR121Module( );

	virtual void begin( PATCore* ctx );
	virtual void end( );
	virtual uint32_t update( uint32_t elapsed );
	virtual bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size );
	void reportTouch( uint8_t index, bool present, bool interrupt = true );
	
protected:
	uint8_t _address;
	uint8_t status;
	int lastSend;
	int _milli;

	uint16_t _activeMask;
	uint16_t _lastState;
	
	void mpr121_setup( );
	void set_register( int address, unsigned char r, unsigned char v );
	uint16_t readCurrentRegister16( );
	uint8_t readCurrentRegister8( );
	uint16_t readRegister16( uint8_t reg );
	uint8_t readRegister8( uint8_t reg );
	uint16_t getFilteredData( uint8_t t );
	uint16_t	getBaselineData( uint8_t t );
	void setThresholds( int pin, uint8_t touch, uint8_t release );
	void resetChip( );
	void recalibrate( );
};

#endif
