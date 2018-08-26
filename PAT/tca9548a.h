// #define WIRE_SUCCESS		0
// #define WIRE_OVERFLOW	 1
// #define WIRE_ADDR_NACK	2
// #define WIRE_DATA_NACK	3
// #define WIRE_OTHER_ERR	4
#pragma once

#include "Arduino.h"

class TCA9548A {
	public:
		TCA9548A( );
//		TCA9548A( uint8_t offset = 0 );
//		TCA9548A( uint8_t slaves[8], uint8_t offset = 0 );
		uint8_t operator<<( uint8_t portIndex );
		uint8_t &operator[]( uint8_t portIndex );
	private:
		uint8_t _address[9];
};

