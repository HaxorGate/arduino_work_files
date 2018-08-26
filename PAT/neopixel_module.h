#ifndef neopixel_module_h
#define neopixel_module_h

#include "imodule.h"

class NeoPixelModule : public IModule {
public:
	NeoPixelModule( );
	virtual ~NeoPixelModule( );

	virtual void begin( PATCore* ctx );
	virtual void end( );
	virtual uint32_t update( uint32_t elapsed );
	virtual bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size );

protected:
	uint8_t _address;
};

#endif
