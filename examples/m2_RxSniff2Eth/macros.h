#ifndef MACROS_H_
#define MACROS_H_

//------------------------------------------------------------------------------ ---------------------------------------
// Preprocessor assertions
//
#define CASSERT(predicate, file)  _impl_CASSERT_LINE(predicate,__LINE__,file)
#define _impl_PASTE(a,b)          a##b
#define _impl_CASSERT_LINE(predicate, line, file) \
    typedef char _impl_PASTE(assertion_failed_##file##_,line)[2*!!(predicate)-1];
    
//------------------------------------------------------------------------------ ---------------------------------------
// I hate typing: SerialUSB.println(F("message"));
// I prefer       FSAYLN("message");
// ...This also means, you can move the outpout device by editing STDOUT
//
#define STDOUT      SerialUSB

#define SAY(...)    STDOUT.print(__VA_ARGS__)
#define SAYLN(...)  STDOUT.println(__VA_ARGS__)

#define FSAY(s)     STDOUT.print(F(s))
#define FSAYLN(s)   STDOUT.println(F(s))

//------------------------------------------------------------------------------ ---------------------------------------
// Get size of a typedef'ed struct'ure member
//
#define SIZEOFM(type,member)  (sizeof(((type*)0)->member))

#endif // MACROS_H_

