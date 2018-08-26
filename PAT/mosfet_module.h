#ifndef mosfet_module_h
#define mosfet_module_h

#include "imodule.h"

class MOSFETModule : public IModule {
public:
	MOSFETModule( );
	virtual ~MOSFETModule( );

	virtual void begin( PATCore* ctx );
	virtual void end( );
	virtual uint32_t update( uint32_t elapsed );
	bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size );
private:
};

#endif
