#ifndef relay_module_h
#define relay_module_h

#include "imodule.h"
#include <Adafruit_MCP23017.h>

class RelayModule : public IModule {
	public:
		RelayModule( );
		virtual ~RelayModule( );

		virtual void begin( PATCore* pat );
		virtual void end( );
		virtual uint32_t update( uint32_t elapsed );
		virtual bool receiveCommand( Bytecodes cmd, uint8_t* buff, uint32_t size );

	protected:
		Adafruit_MCP23017* _pins;
		uint8_t _address;
		uint8_t _pinCount;

		uint16_t _lastRead;
		uint16_t _thisRead;
		uint16_t _watchPins;
		uint16_t _outputPins;
		uint16_t _outputState;

		void reportWatchedPins( );
		void reportAllPins( );

		void setWatch( uint16_t newWatch );
		void setOutput( uint16_t pins, uint16_t state );
};

#endif
