#ifndef imodule_h
#define imodule_h

#include "pat_core.h"
#define PURE_VIRTUAL 0
class IModule {
public:
	enum class ModuleState
	{
		Uninitialized = 0,
		Beginning		 = -1,
		Ready				 = 1,
		Ended				 = -2,
		Absent				= -99
	};

	virtual ~IModule( )
	{
	}

	virtual void begin( PATCore* ctx )
	{
		_pat = ctx;
		_state = ModuleState::Beginning;
	}

	virtual uint32_t update( uint32_t elapsed ) = PURE_VIRTUAL;

	virtual ModuleState state( )
	{
		return _state;
	}

	virtual void end( )
	{
		_state = ModuleState::Ended;
	}

	virtual bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) { 
		_pat->log( )->println( "IModule::receiveCommand( )\n THIS IS NOT SUPPOSED TO BE CALLED!" );
		return false; 
	}

	const Modules module;

	bool isPresent( ) { return _state != ModuleState::Absent; }

protected:
	IModule( Modules mod ) : module( mod )
	{
		_pat = NULL;
		_state = ModuleState::Uninitialized;
		_updateWait = 0;
	}

	PATCore* _pat;
	ModuleState _state;

	bool isWaitDone( uint32_t elapsed )
	{
		if( elapsed >= _updateWait )
		{
			_updateWait = 0;
			return true;
		}
		else
		{
			_updateWait -= elapsed;
			return false;
		}
	}

	uint32_t setWait( uint32_t window )
	{
		_updateWait = window;
	}

	uint32_t remainingWait( )
	{
		return _updateWait;
	}

private:
	uint32_t _updateWait;
};

#endif
