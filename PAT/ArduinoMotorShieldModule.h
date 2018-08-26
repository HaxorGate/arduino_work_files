#ifndef arduino_motor_shield_module_h
#define arduino_motor_shield_module_h

#include "imodule.h"

#include <Adafruit_MotorShield.h>
#define kMaxShields 15
class ArduinoMotorShieldModule	: public IModule {
public:
	ArduinoMotorShieldModule( );
	virtual ~ArduinoMotorShieldModule( );

	virtual void begin( PATCore* ctx );
	virtual void end( );
	virtual uint32_t update( uint32_t elapsed );
	bool receiveCommand( Bytecodes command, uint8_t* buffer, uint32_t size );
private:
	Adafruit_MotorShield AFMS[kMaxShields];
	Adafruit_StepperMotor * motors[kMaxShields * 2];
};

#endif
