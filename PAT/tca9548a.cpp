#include "Arduino.h"
#include <Wire.h>
#include "tca9548a.h"

TCA9548A::TCA9548A( ){
	
}
//
//TCA9548A::TCA9548A( uint8_t offset ){
//	_address[ 0 ] = 0x70 + offset;
//}
//
//TCA9548A::TCA9548A( uint8_t slaves[8], uint8_t offset ) {
//	_address[0] = 0x70 + offset;
//	for( uint8_t i = 1; i < 9; ++i ) {
//		_address[i] = slaves[i - 1];
//	}
//}


uint8_t &
TCA9548A::operator[]( uint8_t portIndex ){
	return _address[ portIndex ];
}

uint8_t
TCA9548A::operator<<( uint8_t portIndex ) {
	Wire.beginTransmission( _address[ 0 ] );
	Wire.write( 1 << portIndex	);
	return Wire.endTransmission( );
}

