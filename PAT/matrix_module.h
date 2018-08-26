#ifndef matrix_module_h
#define matrix_module_h

#include "imodule.h"
#include <Adafruit_MCP23017.h>

class MatrixModule : public IModule {
public:
	MatrixModule( );
	virtual ~MatrixModule( );

	virtual void begin( PATCore* ctx );
	virtual void end( );
	virtual uint32_t update( uint32_t elapsed );
	virtual bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size );

protected:
	Adafruit_MCP23017* _pins;
	uint8_t _address;
	uint8_t _pinCount;

	uint16_t _lastPress;
	uint16_t _thisPress;
	uint16_t _lastRead;
	uint16_t _thisRead;
	uint16_t _watchPins;
	uint16_t _matrixPins;

	uint8_t _strobeColumn;
	uint8_t _numColumns;
	uint8_t _numRows;
	uint8_t _configuration[16];
	int _lastKeyDown;
	bool _repeat;

	void reportWatchedPins( );
	void reportKeyDown( int code );
	void reportKeyUp( );
	void setWatch( uint16_t newWatch );
	void setConfiguration( uint8_t* buffer, uint32_t size );

	void dumpConfiguration( );
};

#endif
