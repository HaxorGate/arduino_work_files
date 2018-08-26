#ifndef gpio_module_h
#define gpio_module_h

#include "imodule.h"
#include <Adafruit_MCP23017.h>

#define MAX_GPIO_MODULES 8
class GPIOModule : public IModule {
	public:
		GPIOModule();
		GPIOModule(uint8_t address_offset);
		virtual ~GPIOModule();
		
		virtual void begin(PATCore* ctx);
		virtual void end();
		virtual uint32_t update(uint32_t elapsed);
		virtual bool receiveCommand(Bytecodes command, uint8_t* buffer, uint32_t size);
	
	protected:
		Adafruit_MCP23017* _pins[ MAX_GPIO_MODULES ];
		uint8_t _address[ MAX_GPIO_MODULES ];
		uint8_t _pinCount;
	
		uint16_t _lastRead[ MAX_GPIO_MODULES ];
		uint16_t _thisRead[ MAX_GPIO_MODULES ];
		uint16_t _watchPins[ MAX_GPIO_MODULES ];
		uint16_t _outputPins[ MAX_GPIO_MODULES ];
		uint16_t _outputState[ MAX_GPIO_MODULES ];
		bool present[ MAX_GPIO_MODULES ];
	
		void reportWatchedPins( uint8_t index = 0 );
		void reportAllPins( uint8_t index = 0 );
	
		void setWatch(uint16_t newWatch, uint8_t index);
		void setOutput(uint16_t pins, uint16_t state, uint8_t index);
	private:
};

#endif
