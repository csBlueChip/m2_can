#ifndef BOOL_H_
#define BOOL_H_

//
// FFS. The core library "wiring_constants.h" has decided to reimplement "bool"
//
// warning: "true" redefined [enabled by default]
//   C:\Users\Username\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.11\cores\arduino/wiring_constants.h:33:0: 
//     #define true 0x1
// note: this is the location of the previous definition 
//   c:\users\username\appdata\local\arduino15\packages\arduino\tools\arm-none-eabi-gcc\4.8.3-2014q1\lib\gcc\arm-none-eabi\4.8.3\include\stdbool.h:34:0: 
//     #define true 1
//

#include <Arduino.h>

#ifndef true
#	include <stdbool.h>
#endif

#endif // BOOL_H_
