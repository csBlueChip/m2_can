#ifndef MACROS_H_
#define MACROS_H_

#define SAY(...)    SerialUSB.print(__VA_ARGS__)
#define SAYLN(...)  SerialUSB.println(__VA_ARGS__)

#define FSAY(s)     SerialUSB.print(F(s))
#define FSAYLN(s)   SerialUSB.println(F(s))

#endif // MACROS_H_

