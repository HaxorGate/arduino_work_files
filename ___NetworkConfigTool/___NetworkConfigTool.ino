
/******************************************************************************/
/*!
@file   <project name>
@author <your name>
@date   <project date>
@par Company: Epic Team Adventures (ETA)
@brief  <describe your program's purpose>
*/
/******************************************************************************/

//INCLUDES
#include <EEPROM.h>

#define EEPROM_WRITE

//GLOBALS
const char networkInfo[] =  
  #include "NetworkInfo.h"
;
/*!
@brief The setup() function holds all the initialization routines for the sketch
*/
void setup() {
  Serial.begin(115200);
  Serial.println(sizeof(networkInfo));
  uint8_t size = 70;//sizeof(networkInfo);
  
  EEPROM.begin( size );
  uint8_t i;
  for ( i = 0; i < size; ++i){
    #ifdef EEPROM_WRITE
    EEPROM.write( i, networkInfo[i] );
    #endif
    Serial.write(EEPROM.read(i));
    digitalWrite(0,0);
    delay(100);
  }
  EEPROM.end();
  Serial.print( "EEPROM reserves " );
  Serial.print( size );
  Serial.println( " bytes" );
}

/*!
@brief The loop() function holds all the execution logic routines for the sketch
*/
void loop() {
}
