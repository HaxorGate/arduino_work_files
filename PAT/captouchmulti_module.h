#ifndef captouchmulti_module_h
#define captouchmulti_module_h

#include "imodule.h"

class CapTouchMultiModule : public IModule {
public:
	CapTouchMultiModule( );
	virtual ~CapTouchMultiModule( );

	virtual void begin( PATCore* ctx );
	virtual void end( );
	virtual uint32_t update( uint32_t elapsed );
	virtual bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size );
	void reportTouch( uint8_t index,bool present, bool inturrupt = true );
	
protected:
	uint8_t _address;
	uint8_t status;
	int lastSend;
	int _milli;
	
};

#endif
