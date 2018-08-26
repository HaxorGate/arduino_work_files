#include "Arduino.h"
#include "pat_core.h"
#include "imodule.h"
#include "pixeltypes.h"
#include "NetworkVariables.h"
//#include <EspSaveCrash.h>
////////////////////////////////////////////////////////////////////////////////
// ctor
////////////////////////////////////////////////////////////////////////////////
PATCore::PATCore( ) {
	_moduleCount = 0;
	memset( _modules, 0, Hardware::kMaxModuleCount * sizeof( IModule* ) );
	memset( _bus, 0, Hardware::kMaxModuleCount );
	_i2cScan = 0;
	_busCount = 0;
	_modulesBegun = false;
	_cursor = 0;
	_messageBuffer = new uint8_t[kMaxMessageSize + 1];
	_commandBuffer = new uint8_t[kMaxMessageSize + 1];
	_logBuffer = new uint8_t[kMaxMessageSize + 1];
}
////////// end ctor

////////////////////////////////////////////////////////////////////////////////
// begin
////////////////////////////////////////////////////////////////////////////////
void PATCore::begin( PCFlash* flash ) {
	_flash = flash;


	Wire.pins( Hardware::kI2CDataPin, Hardware::kI2CClockPin );
	Wire.begin( Hardware::kI2CDataPin, Hardware::kI2CClockPin );
	randomSeed( analogRead( A0 ) );
	_unrequitedPingLimit = 5;

	pinMode( Hardware::kGPIO0, INPUT_PULLUP );

	// TODO Eventually we need to fetch the number of installed LEDs from flash... but we know we always have at least two
	_ledCount = Hardware::kOnboardLEDs;
	_leds = new CRGB[_ledCount];
	memset( _leds, 0, _ledCount * sizeof( CRGB ) );
	// No need to limit the data rate when we only have two pixels...
//FastLED.addLeds<LED_TYPE = DOTSTAR,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	FastLED.addLeds<DOTSTAR, Hardware::kSPIDataPin, Hardware::kSPIClockPin, BGR, DATA_RATE_MHZ( 10 )>( _leds, _ledCount );
	_leds[Hardware::kStatusLEDIndex] = CRGB::Red;
	_leds[Hardware::kActivityLEDIndex] = CRGB::Blue;
	FastLED.show( );
	//	_network = new PuzzleClient( );
//  _network = new PuzzleClient( "ETAInfrastructure7", "soepicsoveryepic", "10.7.1.2:9070", "CastlePAT", "72" );
	_network = new PuzzleClient( _ssid_.c_str( ), _password_.c_str( ), _host_.c_str( ), _patName_.c_str( ), _node_.c_str( ) );

	_network->begin( _flash );
	_i2cScan = 1;
	_refreshLEDS = true;
//	SaveCrash.print( _network->getNetwork() );

	_lastMessageReal = false;

	setPingCount( 0 );
}
////////// end begin( )


////////////////////////////////////////////////////////////////////////////////
// setPingCount
////////////////////////////////////////////////////////////////////////////////
void PATCore::setPingCount( int newCount ) {
	_pingCount = newCount;
	_pingTimeout = random( 5 +( 2 * _pingCount ), 10 +( 5 * _pingCount ) ) * 1000;
}
////////// end setPingCount


////////////////////////////////////////////////////////////////////////////////
// update
////////////////////////////////////////////////////////////////////////////////
uint32_t PATCore::update( uint32_t elapsed ) {
	uint32_t minDesiredInterval = 20;

	uint32_t request = _network->update( elapsed );
	if( request < minDesiredInterval )
		minDesiredInterval = request;

	_networkAvailable = false;

	switch( _network->status( ) )
	{
		case PuzzleClient::Status::Boot:
			_leds[Hardware::kStatusLEDIndex] = CRGB::DarkViolet;
			_leds[Hardware::kActivityLEDIndex] = CRGB::Black;
			_refreshLEDS = true;
		break;

		case PuzzleClient::Status::ReadFlash:
		case PuzzleClient::Status::FlashRead:
			_leds[Hardware::kStatusLEDIndex] = CRGB::Orange;
			_leds[Hardware::kActivityLEDIndex] = CRGB::Black;
			_refreshLEDS = true;
		break;

		case PuzzleClient::Status::FindWifi:
		case PuzzleClient::Status::WifiFound:
			_leds[Hardware::kStatusLEDIndex] = CRGB::Yellow;
			_leds[Hardware::kActivityLEDIndex] = CRGB::Black;
			_refreshLEDS = true;
		break;

		case PuzzleClient::Status::FindServer:
		case PuzzleClient::Status::ServerFound:
			_leds[Hardware::kStatusLEDIndex] = CRGB::Green;
			_leds[Hardware::kActivityLEDIndex] = CRGB::Black;
			_i2cScanReported = false;
			_modulesReported = false;
			_bootComplete = false;
			_refreshLEDS = true;
		break;

		case PuzzleClient::Status::Connected:
			{
				uint8_t value = _network->freshness( );
				if( value < 16 )
					value = 16;
				uint8_t hue = HUE_BLUE;
				if( !_lastMessageReal )
					hue = HUE_PURPLE;
				_leds[Hardware::kStatusLEDIndex].setHSV( hue, 255, value );
				_networkAvailable = true;
				_refreshLEDS = true;
			}
		break;

		case PuzzleClient::Status::Disconnected:
		case PuzzleClient::Status::ServerError:
		case PuzzleClient::Status::WifiError:
		case PuzzleClient::Status::FlashError:
			_leds[Hardware::kStatusLEDIndex] = CRGB::Red;
			_i2cScanReported = false;
			_modulesReported = false;
			_bootComplete = false;
			_refreshLEDS = true;
		break;

		default:
			_leds[Hardware::kStatusLEDIndex] = CRGB::White;
			_refreshLEDS = true;
		break;
	}

	if( _i2cScan <= kMaxI2CAddress )
	{
		if( _network->isConfigLoaded( ) )
		{
			log( )->print( "scanning for I2C @" );
			log( )->print( _i2cScan, HEX );
			Wire.beginTransmission( _i2cScan );
			int error = Wire.endTransmission( );
			log( )->print( "...received error code " );
			log( )->println( error );

			if( error == 0 ) {
				_network->log( )->print( "FOUND I2C " );
				_network->log( )->println( _i2cScan, HEX );

				_bus[_busCount] = _i2cScan;
				++_busCount;
			}

			++_i2cScan;
		}
	}
	else
	{
		if( !_i2cScanReported && isNetworkAvailable( ) )
		{
			// Report results
			buildMessage( Bytecodes::HardwareCapabilities );
			buildMessage( _busCount );
			buildMessage( _bus, _busCount );
			_i2cScanReported = writeMessage( Modules::Core );
		}

		if( !_modulesBegun )
		{
			beginModules( );
		}
		if( !_modulesReported && isNetworkAvailable( ) )
		{
			buildMessage( Bytecodes::ModulesPresent );
			uint8_t* sizePtr = byteBuffer( );
			*sizePtr = 0;
			for( int i = 0; i < _moduleCount; i++ )
			{
				if( _modules[i]->isPresent( ) )
				{
					buildMessage(( uint8_t )_modules[i]->module );
					*sizePtr = *sizePtr + 1;
				}
			}

			_modulesReported = writeMessage( Modules::Core );
		}
	}
	if( _i2cScanReported && _modulesReported && !_bootComplete )
	{
		buildMessage( Bytecodes::Hello );
		buildMessage( kSoftwareVersion >> 8 );
		buildMessage( kSoftwareVersion & 0xFF );
		buildMessage( kBoardRevision >> 8 );
		buildMessage( kBoardRevision & 0xFF );

		int len = strlen( compile_date );
		for( int i = 0; i < len; i++ )
			buildMessage( compile_date[i] );

		_bootComplete = writeMessage( Modules::Core );
	}
	// Read incoming messages
	uint32_t size = 0;
	bool sawMessage = false;
	bool handled = false;
	bool present = false;

	size = _network->readNextMessage( _commandBuffer, kMaxMessageSize + 1 );
	if( size > 1 )
	{
		sawMessage = true;

		Modules dest =( Modules )_commandBuffer[0];
		Bytecodes command =( Bytecodes )_commandBuffer[1];
		_lastMessageReal =( command != Bytecodes::Ping );

		if( dest == Modules::Core )
		{
			present = true;
			handled = receiveCommand( command, _commandBuffer + 2, size - 2 );
			// TODO Really ought to make PATCore implement IModule, and then just have core be in the isPresent list...
		}
		else if(( uint8_t )dest >=( uint8_t )Modules::FIRST_RESERVED )
		{
			// We're not supposed to understand this message.
			present = true;
			handled = true;
		}
		else
		{
			for( int i = 0; i < _moduleCount; i++ )
			{
				if( _modules[i]->isPresent( ) && _modules[i]->module == dest )
				{
					present = true;
					handled = _modules[i]->receiveCommand( command, _commandBuffer + 2, size - 2 );
					log( )->print( "<<PATCore: Module " );
					log( )->print( static_cast<uint8_t>( _modules[i]->module ), HEX );
					log( )->print( " was sent command " );
					log( )->print( static_cast<uint8_t>( command ), HEX );
					log( )->print( " with a payload of \"" );
					for( uint8_t index = 0; index <(size - 2); ++index ) {
						log( )->print(( _commandBuffer + 2 )[ index ], HEX );
					}
					log( )->print( "\"" );
					log( )->print( " and the command was " );
					log( )->println( handled? " handled " : " not handled " );
				}
			}
		}
	}
	else
	{
		// We just saw a one-byte message, which is not parseable... :( 
	}

	if( sawMessage )
	{
		setPingCount( 0 );

		if( !present )
		{
			buildMessage( Bytecodes::ModuleNotPresent );
		}
		else if( !handled )
		{
			buildMessage( Bytecodes::ModuleReceiveError );
			log( )->println( "ModuleReceiveError" );
		}
		else
		{
			buildMessage( Bytecodes::Acknowledged );
		}
		buildMessage( _commandBuffer, size );
		writeMessage( Modules::Core );
	}
	else if( _unrequitedPingLimit > 0 )
	{
		if( _pingTimeout <= elapsed )
		{
			setPingCount( _pingCount + 1 );

			if( _pingCount > _unrequitedPingLimit )
			{
				ESP.reset( );
			}

			buildMessage( Bytecodes::Ping );
			buildMessage( _pingCount );
			buildMessage( _unrequitedPingLimit );
			writeMessage( Modules::Core );
		}
		else
			_pingTimeout -= elapsed;
	}

	if( _modulesBegun )
	{
		for( int i = 0; i < _moduleCount; i++ )
		{
			if( _modules[i]->isPresent( ) )
			{
				uint32_t requestedInterval = _modules[i]->update( elapsed );
				if( requestedInterval > 0 && requestedInterval < minDesiredInterval )
					minDesiredInterval = requestedInterval;
			}
		}
	}

	if( _refreshLEDS )
	{
		uint32_t gap = millis( ) - _lastLEDRefresh;
		if( gap > 50 )
		{
			FastLED.show( );
			_refreshLEDS = false;
			_lastLEDRefresh = millis( );
		}
	}

	if( sawMessage )
		return 5;
	else
		return minDesiredInterval;
}
////////// end update


////////////////////////////////////////////////////////////////////////////////
// receiveCommand
////////////////////////////////////////////////////////////////////////////////
bool PATCore::receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size ) {
	bool handled = true;
	String buff =( char * )buffer;
		log( )->print( "PATCore::receiveCommand " );
		log( )->print( static_cast<uint8_t>( command ), HEX );
	switch( command )
	{
		case Bytecodes::Ping:
		break;

		case Bytecodes::EnablePing:
			if( size > 0 )
			{
				_unrequitedPingLimit = buffer[0];
			}
		break;

		case Bytecodes::ActivityLED:
			if( size >= 3 )
			{
				setPixelColor( PixelIDs::Activity, CRGB( buffer[0], buffer[1], buffer[2] ) );
			}
		break;

		case Bytecodes::SetLEDs:
			{
				int startIndex = 1;
				int endIndex = 2;

				if( size >= 6 )
				{
					if( buffer[0] != 0x00 )		// non-zero = exclusive mode, where these are the ONLY LEDs to set
					{
						setPixelRange( 0, _ledCount - 1, CRGB( 0, 0, 0 ) );
					}

					uint16_t startVal = 0;
					uint16_t endVal = 0;
					if( size >= 8 )
					{
						startIndex = 2;	 // start( LSB of two-byte big-endian value )
						endIndex = 4;		 // end( LSB of two-byte big-endian value )
						startVal = buffer[1] << 8;
						endVal = buffer[3] << 8;
					}
					startVal += buffer[startIndex];
					endVal += buffer[endIndex];
					if( endVal > 2000 )
						endVal = 2000;
					char r =( buffer[endIndex + 1] );
					char g =( buffer[endIndex + 2] );
					char b =( buffer[endIndex + 3] );
					setPixelRange( startVal, endVal, CRGB( r, g, b ) );

					// Artificially force the refresh, since this was a direct server command...
					_lastLEDRefresh = 0;
				}
				else
				{
					handled = false;
				}
			}
		break;
		default:
			handled = false;
		break;
	}

	return handled;
}
////////// end receiveCommand


////////////////////////////////////////////////////////////////////////////////
// beginModules
////////////////////////////////////////////////////////////////////////////////
void PATCore::beginModules( ) {
	_network->log( )->println( "Beginning modules..." );
	for( int i = 0; i < _moduleCount; i++ )
	{
		_modules[i]->begin( this );
	}
	_modulesBegun = true;
	_network->log( )->print( _moduleCount );
	_network->log( )->println( " modules begun" );
}
////////// end beginModules


////////////////////////////////////////////////////////////////////////////////
//	hardwarePresent
////////////////////////////////////////////////////////////////////////////////
bool PATCore::hardwarePresent( uint8_t address ) {
	for( int i = 0; i < _busCount; i++ )
	{
		if( _bus[i] == address )
			return true;
	}

	return false;
}
////////// end hardwarePresent


////////////////////////////////////////////////////////////////////////////////
// writeMessage
////////////////////////////////////////////////////////////////////////////////
bool PATCore::writeMessage( Modules module ) {
	_messageBuffer[0] =( uint8_t )module;

	bool result = _network->writeMessage( _messageBuffer, _cursor + 1 );
	_cursor = 0;
	return result;
}
////////// end writeMessage


////////////////////////////////////////////////////////////////////////////////
// buildMessage with byte value
////////////////////////////////////////////////////////////////////////////////
void PATCore::buildMessage( uint8_t value ) {
	if( _cursor >= kMaxMessageSize )
		return;

	_messageBuffer[1 + _cursor] = value;
	++_cursor;
}

////////////////////////////////////////////////////////////////////////////////
// buildMessage with byte array
////////////////////////////////////////////////////////////////////////////////
void PATCore::buildMessage( uint8_t* buffer, uint32_t count ) {
	uint32_t cap = kMaxMessageSize - _cursor;
	if( count > cap )
		count = cap;

	for( uint32_t index = 0; index < count; index++ )
	{
		_messageBuffer[1 + _cursor + index] = buffer[index];
	}

	_cursor += count;
}


////////////////////////////////////////////////////////////////////////////////
// signup
////////////////////////////////////////////////////////////////////////////////
void PATCore::signup( IModule* module ) {
	if( _moduleCount >= Hardware::kMaxModuleCount )
		return;

	_modules[_moduleCount] = module;
	++_moduleCount;
}


////////////////////////////////////////////////////////////////////////////////
// setPixelColor
////////////////////////////////////////////////////////////////////////////////
void PATCore::setPixelColor( const int pixel, const CRGB color ) {
	uint8_t index;
	switch( pixel )
	{
		case( int )PixelIDs::Status:
			index =( int )Hardware::kStatusLEDIndex;
		break;

		case( int )PixelIDs::Activity:
			index =( int )Hardware::kActivityLEDIndex;
		break;

		default:
			index = pixel +( int )Hardware::kOnboardLEDs;
		break;
	}

	if( index >= 0 && index < _ledCount )
		_leds[index] = color;
	_refreshLEDS = true;
}



////////////////////////////////////////////////////////////////////////////////
// setPixelRange
////////////////////////////////////////////////////////////////////////////////
void PATCore::setPixelRange( uint16_t start, uint16_t notstart, CRGB color ) {
	if( start > notstart )notstart = start;
	if( notstart >= _ledCount )
	{
		CRGB * newLEDS = new CRGB[notstart + 1]( );

		for( int i = 0; i < _ledCount; ++i )
		{
			newLEDS[i] = _leds[i];
		}
		delete[] _leds;
		_leds = newLEDS;
		_ledCount = notstart;
		FastLED.addLeds<DOTSTAR, Hardware::kSPIDataPin, Hardware::kSPIClockPin, BGR, DATA_RATE_MHZ( 10 )>( _leds, _ledCount );
	}

	for( ; start <= notstart; ++start )
	{
		_leds[start] = color;
	}
	_refreshLEDS = true;
}



////////////////////////////////////////////////////////////////////////////////
// encodeNibble
////////////////////////////////////////////////////////////////////////////////
char PATCore::encodeNibble( uint8_t nibble ) {
	uint8_t val = nibble & 0x0F;

	if( val < 0x0A )
		return '0' + val;
	else
		return 'A' +( val - 0x0A );
}



////////////////////////////////////////////////////////////////////////////////
// writeConfiguration
////////////////////////////////////////////////////////////////////////////////
bool PATCore::writeConfiguration( Modules module, const uint8_t* src, uint32_t size ) {
	char key[9] = "config__";
	uint8_t val =( uint8_t )module;
	key[6] = encodeNibble( val >> 4 );
	key[7] = encodeNibble( val );

	bool success = false;
	if( _flash && _flash->isOkay( ) )
		success = _flash->write( key,( const char* )src, size );
	return success;
}



////////////////////////////////////////////////////////////////////////////////
// readConfiguration
////////////////////////////////////////////////////////////////////////////////
uint32_t PATCore::readConfiguration( Modules module, uint8_t* buffer, uint32_t maxSize ) {
	if( _flash == NULL || !_flash->isOkay( ) )
		return	0;

	char key[9] = "config__";
	uint8_t val =( uint8_t )module;
	key[6] = encodeNibble( val >> 4 );
	key[7] = encodeNibble( val );

	uint32_t actuallyRead = _flash->read( key,( char* )buffer, maxSize );
	return actuallyRead;
}
