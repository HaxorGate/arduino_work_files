#ifndef rfid_module_h
#define rfid_module_h

#include "imodule.h"

class RFIDModule : public IModule {
	public:
		RFIDModule( );
		virtual ~RFIDModule( );

		virtual void begin( PATCore* pat );
		virtual void end( );
		virtual uint32_t update( uint32_t elapsed );

		bool wasRFIDPresent( int );

	private:
		uint8_t * _lastCardID[8];
		uint8_t _lastCardIDSize[8];

	void reportCard( bool present, uint8_t );
};

#endif
