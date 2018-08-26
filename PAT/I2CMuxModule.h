#ifndef I2CMUX_INCL
#define I2CMUX_INCL
#include "Arduino.h"
#include "imodule.h"
#include "tca9548a.h"/*
 * I AM FUDGING THIS TOGETHER BY MAKING IT SPECIFIC TO MUXING THE COLOR SENSORS IN
 * MAZE OF GAMES. THIS MUST BE REWORKED TO THE GENERAL CASE, BUT I AM RUNNING OUT
 * OF TIME. AS SOON AS EVERYONE ELSE FALLS BEHIND SCHEDULE, I WILL WORK ON THIS
*/
#include <Adafruit_TCS34725.h>
#define TCS_NUM_SENSORS 6

class I2CMuxModule : public IModule {
	public:
		I2CMuxModule( uint8_t offset = 0 );
		virtual ~I2CMuxModule( void );
		virtual void begin( PATCore *ctx );
		virtual uint32_t update( uint32_t elapsed );
		virtual bool receiveCommand( Bytecodes command, uint8_t *buffer, uint32_t size );
		uint8_t howMany( void );// access _object_count
	private:
		TCA9548A _mux;
		Adafruit_TCS34725 tcs[ TCS_NUM_SENSORS ];
		uint16_t this_c[ TCS_NUM_SENSORS ], last_c[ TCS_NUM_SENSORS ], delta_c[ TCS_NUM_SENSORS ];
		uint16_t this_r[ TCS_NUM_SENSORS ], last_r[ TCS_NUM_SENSORS ], delta_r[ TCS_NUM_SENSORS ];
		uint16_t this_g[ TCS_NUM_SENSORS ], last_g[ TCS_NUM_SENSORS ], delta_g[ TCS_NUM_SENSORS ];
		uint16_t this_b[ TCS_NUM_SENSORS ], last_b[ TCS_NUM_SENSORS ], delta_b[ TCS_NUM_SENSORS ];
		uint16_t epsilon[ 4 ];
		static uint8_t _object_count;
};

#endif
