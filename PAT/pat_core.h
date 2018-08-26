#ifndef pat_core_h
#define pat_core_h

#include "Arduino.h"
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <SPI.h>

#ifdef USE_NODEMCU
#define FASTLED_ESP8266_D1_PIN_ORDER
#endif

#include <FastLED.h>

#include <PCFlash.h>
#include <PuzzleClient.h>

#include "constants.h"

class IModule;
class PATCore {
public:
	PATCore( );
	void begin( PCFlash* flash );
	uint32_t update( const uint32_t elapsed );
	void signup( IModule* module );

	bool isNetworkAvailable( ) { return _networkAvailable; }

	enum class PixelIDs
	{
		Status = -1,
		Activity = -2,
		First = 0
	};
	void setPixelColor( const int pixel, const CRGB color );
	void setPixelColor( const PixelIDs pixel, const CRGB color ) { setPixelColor(( int )pixel, color ); }
	void setPixelRange( uint16_t start, uint16_t notstart, CRGB color );


	uint8_t* messageBuffer( ) { return _messageBuffer + 1; }
	uint8_t* byteBuffer( ) { uint8_t* result = _messageBuffer + 1 + _cursor; ++_cursor; return result; }
	bool writeMessage( Modules module );
	void buildMessage( uint8_t value );
	void buildMessage( Bytecodes value ) { buildMessage(( uint8_t )value ); }
	void buildMessage( uint8_t* buffer, uint32_t count );
	bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size );

	bool hardwarePresent( uint8_t address );

	PuzzleClient::Log* log( ) { return _network->log( ); }

	bool writeConfiguration( Modules module, const uint8_t* src, uint32_t size );
	uint32_t readConfiguration( Modules module, uint8_t* buffer, uint32_t maxSize );

	const char* getName( ) { return _network->GetName( ); }

private:
	IModule* _modules[kMaxModuleCount];
	uint8_t _moduleCount;
	uint8_t _bus[kMaxModuleCount];
	uint8_t _busCount;
	bool _refreshLEDS;
	CRGB* _leds;
	uint16_t _ledCount;
	PuzzleClient* _network;
	int _cursor;
	uint8_t* _messageBuffer;
	uint8_t* _commandBuffer;
	uint8_t* _logBuffer;
	PCFlash* _flash;

	int _unrequitedPingLimit;
	bool _networkAvailable;
	bool _lastMessageReal;
	uint32_t _pingTimeout;
	int _pingCount;
	uint8_t _i2cScan;
	bool _i2cScanReported;

	bool _modulesBegun;
	void beginModules( );
	bool _modulesReported;

	bool _bootComplete;
	uint32_t _lastLEDRefresh;

	char encodeNibble( uint8_t val );

	void setPingCount( int newCount );
};

#endif
