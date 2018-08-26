#ifndef tiki_module_h
#define tiki_module_h

#include "imodule.h"
#include <Adafruit_TCS34725.h>

class TikiModule : public IModule {
	public:
		TikiModule( );
		virtual ~TikiModule( );

		virtual void begin( PATCore* ctx );
		virtual void end( );
		virtual uint32_t update( uint32_t elapsed );
		bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size );
	private:
		uint8_t _lastStatus; 
		uint32_t _elapsed;
		uint32_t _neopixelLockout;
		bool _coinDebounce;
};

#endif
