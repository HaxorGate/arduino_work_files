#ifndef lsm303_module_h
#define lsm303_module_h

#include "imodule.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303.h>

class LSM303Module : public IModule {
public:
	LSM303Module( );
	virtual ~LSM303Module( );

	virtual void begin( PATCore* ctx );
	virtual void end( );
	virtual uint32_t update( uint32_t elapsed );
	virtual bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size );

protected:
	Adafruit_LSM303* _device;
	uint8_t _updateMode;
	uint32_t _updateInterval;
	bool _sendNow;

	bool setMode( uint8_t mode, uint8_t* parameters, uint32_t parametersSize );
	bool parseData( );
	void sendData( Bytecodes command );

	float _fullRotation;
	uint16_t _lastRotation;
	uint16_t _thisRotation;
};

#endif
