#ifndef constants_h
#define constants_h

//Wire.endTransmission return codes
#define WIRE_SUCCESS		0
#define WIRE_OVERFLOW		1
#define WIRE_ADDR_NACK	2
#define WIRE_DATA_NACK	3
#define WIRE_OTHER_ERR	4

enum Hardware {
	kGPIO0						= 0,

	kSoftwareVersion	= 0x0104,
	kBoardRevision		= 0x0101,
	#ifdef USE_NODEMCU
	kSPIDataPin				= D8,
	kSPIClockPin			= D7,
	kI2CDataPin			  = D4,
	kI2cClockPin			= D5,
	#else
	kSPIDataPin			  = 13,
	kSPIClockPin			= 14,
	kI2CDataPin				= 4,
	kI2CClockPin			= 5,
	#endif
	
	kOnboardLEDs			= 2,
	kStatusLEDIndex		= 0,
	kActivityLEDIndex = 1,

	kMaxModuleCount	  = 16,

	kMaxMessageSize	  = 250,

	kMaxI2CAddress		= 127,

	kForever					= 0x240C8400 // one week in milliseconds
};

const char compile_date[] = __DATE__ " " __TIME__;

#define BEGIN_LITERAL 174
#define END_LITERAL 175

enum class Bytecodes {
	ActivityLED										= 0x02,
	HardwareCapabilities					= 0x03,
	ModulesPresent								= 0x04,
	Log														= 0x05,
	Hello													= 0x06,
	Ping													= 0x07,
	EnablePing										= 0x08,
																//0x09,
	Configure											= 0x0A,
	SaveConfiguration							= 0x0B,
																//0x0C,
																//0x0D,
																//0x0E,
																//0x0F,
	SetLEDs												= 0x10,
	FadeLEDs											= 0x11,
																//0x12,
																//0x13,
																//0x14,
																//0x15,
																//0x16,
																//0x17,
	SetLights											= 0x18,
	FadeLights										= 0x19,
																//0x1A,
																//0x1B,
																//0x1C,
																//0x1D,
																//0x1E,
																//0x1F,
	DigitalRead										= 0x20,
	DigitalWrite									= 0x21,
	DigitalWatch									= 0x22,
	AnalogRead										= 0x23,
	AnalogWatch										= 0x24,
																//0x25,
																//0x26,
																//0x27,
																//0x28,
																//0x29,
																//0x2A,
																//0x2B,
																//0x2C,
																//0x2D,
																//0x2E,
																//0x2F,
	DataAvailable									= 0x30,
	RFIDInterrupt									= 0x30,
	DataReport										= 0x31,
																//0x32,
																//0x33,
																//0x34,
																//0x35,
																//0x36,
																//0x37,
																//0x38,
																//0x39,
																//0x3A,
																//0x3B,
																//0x3C,
																//0x3D,
																//0x3E,
																//0x3F,
	ShowText											= 0x40,
	KeyDown												= 0x41,
	KeyUp													= 0x42,
																//0x43,
																//0x44,
																//0x45,
																//0x46,
																//0x47,
																//0x48,
																//0x49,
																//0x4A,
																//0x4B,
																//0x4C,
																//0x4D,
																//0x4E,
																//0x4F,
	UnityAppFirstReserved					= 0x50,
																//0x51,
																//0x52,
																//0x53,
																//0x54,
																//0x55,
																//0x56,
																//0x57,
																//0x58,
																//0x59,
																//0x5A,
																//0x5B,
																//0x5C,
																//0x5D,
																//0x5E,
	UnityAppLastReserved					= 0x5F,
	TikiDispenseCoconut						= 0x60,
	TikiSetCoinGate								= 0x61,
																//0x62,
																//0x63,
	TikiSequenceExecuting					= 0x64,
	TikiCoinCollected							= 0x65,
	TikiStatusChange							= 0x66,
																//0x67,
																//0x68,
																//0x69,
	TikiManualLoadMode						= 0x6A,
	TikiManualSetScrews						= 0x6B,
	TikiManualCalibrateScrew			= 0x6C,
	TikiManualAdjustScrew					= 0x6D,
	TikiManualSetEMag							= 0x6E,
	TikiManualReset								= 0x6F,
																//0x70,
																//0x71,
																//0x72,
																//0x73,
																//0x74,
																//0x75,
																//0x76,
																//0x77,
																//0x78,
																//0x79,
																//0x7A,
																//0x7B,
																//0x7C,
																//0x7D,
																//0x7E,
																//0x7F
	CapTouchRegisterNoTouchState	= 0x80,
	CapTouchRegisterTouchState		= 0x81,
	CapTouchReportState						= 0x82,
	CapTouchInterrupt							= 0x83,
	CapTouchResult								= 0x84,
	CapTouchMilliSecDelay					= 0x85,
																//0x86,
																//0x87,
																//0x88,
																//0x89,
	MPR121SetPins									= 0x8A,
	MPR121Recalibrate							= 0x8B,
	MPR121SetThresholds						= 0x8C,
																//0x8D,
																//0x8E,
	MPR121Reset										= 0x8F,
	GemDispenserDispenseGem				= 0x90,
	GemDispenserStopAll						= 0x91,
	GemDispenserErrorReported			= 0x92,
																//0x93,
																//0x94,
																//0x95,
																//0x96,
																//0x97,
																//0x98,
																//0x99,
																//0x9A,
																//0x9B,
																//0x9C,
																//0x9D,
																//0x9E,
																//0x9F,
	StepperMotorStep							= 0xA0,
																//0xA1,
																//0xA2,
																//0xA3,
																//0xA4,
																//0xA5,
																//0xA6,
																//0xA7,
																//0xA8,
																//0xA9,
																//0xAA,
																//0xAB,
																//0xAC,
																//0xAD,
																//0xAE,
																//0xAF,
																//0xB0,
																//0xB1,
																//0xB2,
																//0xB3,
																//0xB4,
																//0xB5,
																//0xB6,
																//0xB7,
																//0xB8,
																//0xB9,
																//0xBA,
																//0xBB,
																//0xBC,
																//0xBD,
																//0xBE,
																//0xBF,
																//0xC0,
																//0xC1,
																//0xC2,
																//0xC3,
																//0xC4,
																//0xC5,
																//0xC6,
																//0xC7,
																//0xC8,
																//0xC9,
																//0xCA,
																//0xCB,
																//0xCC,
																//0xCD,
																//0xCE,
																//0xCF,
																//0xD0,
																//0xD1,
																//0xD2,
																//0xD3,
																//0xD4,
																//0xD5,
																//0xD6,
																//0xD7,
																//0xD8,
																//0xD9,
																//0xDA,
																//0xDB,
																//0xDC,
																//0xDD,
																//0xDE,
																//0xDF,
	HardwareError									= 0xE0,
	SoftwareError									= 0xE1,
	HarwareWarning								= 0xE2,
	SoftwareWarning								= 0xE3,
	ModuleNotPresent							= 0xE4,
	ModuleReceiveError						= 0xE5,
																//0xE6,
																//0xE7,
																//0xE8,
																//0xE9,
																//0xEA,
																//0xEB,
																//0xEC,
																//0xED,
																//0xEE,
	Acknowledged									= 0xEF,
	FIRST_RESERVED								= 0xF0,
																//0xF1,
																//0xF2,
																//0xF3,
																//0xF4,
																//0xF5,
																//0xF6,
																//0xF7,
																//0xF8,
																//0xF9,
																//0xFA,
																//0xFB,
																//0xFC,
																//0xFD,
																//0xFE,
																//0xFF
};

enum class Modules {
	Core								= 0x00,
	Diagnostic					= 0x01,
	RFID								= 0x02,
	GPIO								= 0x03,
	Matrix							= 0x04,
	Relay								= 0x05,
	NeoPixel						= 0x06,
	Tiki								= 0x07,
	CapTouch						= 0x08,
	CapTouchMulti				= 0x09,
	MPR121							= 0x0A,
	MOSFET							= 0x0B,
	TCS34725						= 0x0C,
	LSM303							= 0x0D,
	GemDispenser				= 0x0E,
											//0x0F,
	DotstarModule				= 0x10,
	ArduinoMotorShield	= 0x11,
											//0x12,
											//0x13,
											//0x14,
											//0x15,
											//0x16,
											//0x17,
											//0x18,
											//0x19,
											//0x1A,
											//0x1B,
											//0x1C,
											//0x1D,
											//0x1E,
											//0x1F,
											//0x20,
											//0x21,
											//0x22,
											//0x23,
											//0x24,
											//0x25,
											//0x26,
											//0x27,
LaserWireModuleSlave  = 0x28,
											//0x29,
											//0x2A,
											//0x2B,
											//0x2C,
											//0x2D,
											//0x2E,
											//0x2F,
											//0x30,
											//0x31,
											//0x32,
											//0x33,
											//0x34,
											//0x35,
											//0x36,
											//0x37,
											//0x38,
											//0x39,
											//0x3A,
											//0x3B,
											//0x3C,
											//0x3D,
											//0x3E,
											//0x3F,
											//0x40,
											//0x41,
	VolcanoPlynth				= 0x42,
											//0x43,
											//0x44,
											//0x45,
											//0x46,
											//0x47,
											//0x48,
											//0x49,
											//0x4A,
											//0x4B,
											//0x4C,
											//0x4D,
											//0x4E,
											//0x4F,
											//0x50,
											//0x51,
											//0x52,
											//0x53,
											//0x54,
											//0x55,
											//0x56,
											//0x57,
											//0x58,
											//0x59,
											//0x5A,
											//0x5B,
											//0x5C,
											//0x5D,
											//0x5E,
											//0x5F,
											//0x60,
											//0x61,
											//0x62,
											//0x63,
											//0x64,
											//0x65,
											//0x66,
											//0x67,
											//0x68,
											//0x69,
											//0x6A,
											//0x6B,
											//0x6C,
											//0x6D,
											//0x6E,
											//0x6F,
	I2CMuxModule				= 0x70,
											//0x71,
											//0x72,
											//0x73,
											//0x74,
											//0x75,
											//0x76,
											//0x77,
											//0x78,
											//0x79,
											//0x7A,
											//0x7B,
											//0x7C,
											//0x7D,
											//0x7E,
											//0x7F,
	FIRST_KANGAROO			= 0x80,
											//0x81,
											//0x82,
											//0x83,
											//0x84,
											//0x85,
											//0x86,
											//0x87,
											//0x88,
											//0x89,
											//0x8A,
											//0x8B,
											//0x8C,
											//0x8D,
											//0x8E,
											//0x8F,
											//0x90,
											//0x91,
											//0x92,
											//0x93,
											//0x94,
											//0x95,
											//0x96,
											//0x97,
											//0x98,
											//0x99,
											//0x9A,
											//0x9B,
											//0x9C,
											//0x9D,
											//0x9E,
											//0x9F,
											//0xA0,
											//0xA1,
											//0xA2,
											//0xA3,
											//0xA4,
											//0xA5,
											//0xA6,
											//0xA7,
											//0xA8,
											//0xA9,
											//0xAA,
											//0xAB,
											//0xAC,
											//0xAD,
											//0xAE,
											//0xAF,
											//0xB0,
											//0xB1,
											//0xB2,
											//0xB3,
											//0xB4,
											//0xB5,
											//0xB6,
											//0xB7,
											//0xB8,
											//0xB9,
											//0xBA,
											//0xBB,
											//0xBC,
											//0xBD,
											//0xBE,
											//0xBF,
											//0xC0,
											//0xC1,
											//0xC2,
											//0xC3,
											//0xC4,
											//0xC5,
											//0xC6,
											//0xC7,
											//0xC8,
											//0xC9,
											//0xCA,
											//0xCB,
											//0xCC,
											//0xCD,
											//0xCE,
											//0xCF,
											//0xD0,
											//0xD1,
											//0xD2,
											//0xD3,
											//0xD4,
											//0xD5,
											//0xD6,
											//0xD7,
											//0xD8,
											//0xD9,
											//0xDA,
											//0xDB,
											//0xDC,
											//0xDD,
											//0xDE,
											//0xDF,
	AmbienceK						= 0xE0,
											//0xE1,
											//0xE2,
											//0xE3,
											//0xE4,
											//0xE5,
											//0xE6,
											//0xE7,
											//0xE8,
											//0xE9,
											//0xEA,
											//0xEB,
											//0xEC,
											//0xED,
											//0xEE,
											//0xEF,
	FIRST_RESERVED			= 0xF0,
											//0xF0,
											//0xF1,
											//0xF2,
											//0xF3,
											//0xF4,
											//0xF5,
											//0xF6,
											//0xF7,
											//0xF8,
											//0xF9,
											//0xFA,
											//0xFB,
											//0xFC,
											//0xFD,
											//0xFE,
	UNKNOWN							= 0xFF
};

#endif


