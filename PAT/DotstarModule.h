#ifndef dotstar_module_h
#define dotstar_module_h

#include "imodule.h"


class DotstarModule	: public IModule {
public:
	DotstarModule( );
	virtual ~DotstarModule( );

	virtual void begin( PATCore* ctx );
	virtual void end( );
	virtual uint32_t update( uint32_t elapsed );
	bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size );
private:
};

#endif
