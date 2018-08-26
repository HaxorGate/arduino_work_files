#ifndef captouch_module_h
#define captouch_module_h

#include "imodule.h"

class CapTouchModule : public IModule {
public:
	CapTouchModule( );
	virtual ~CapTouchModule( );

	virtual void begin( PATCore* ctx );
	virtual void end( );
	virtual uint32_t update( uint32_t elapsed );
	virtual bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size );
	void reportTouch( uint8_t index,bool present, bool inturrupt = true );
	
protected:
	uint8_t _address[8];
	
	uint8_t status[8];
	int lastSend;
	int _milli;
};

#endif
