#ifndef tcs34725_module_h
#define tcs34725_module_h

#include "imodule.h"
#include <Adafruit_TCS34725.h>

class TCS34725Module : public IModule {
public:
	TCS34725Module( );
	virtual ~TCS34725Module( );

	virtual void begin( PATCore* ctx );
	virtual void end( );
	virtual uint32_t update( uint32_t elapsed );
	virtual bool receiveCommand( Bytecodes command, 
																uint8_t *buffer, 
																uint32_t size 
 );

protected:
	Adafruit_TCS34725 *_device;
	uint8_t _updateMode;
	uint32_t _updateInterval;
	bool _sendNow;

	uint16_t _lastC;
	uint16_t _lastR;
	uint16_t _lastG;
	uint16_t _lastB;
	uint16_t _lastDelta;

	int _scale;
	uint16_t _delta;

	bool setMode( uint8_t mode, uint8_t *parameters, uint32_t parametersSize );
	void setUpdateInterval( uint16_t interval );
	bool parseData( );
	void sendData( Bytecodes command );
	String _data;
};

#endif
