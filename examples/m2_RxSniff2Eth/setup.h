#ifndef SETUP_H_
#define SETUP_H_

//------------------------------------------------------------------------------ ---------------------------------------
#define APPNAME "m2_RxSniff2Eth"

#define VER_MAJ 1
#define VER_MIN 0
#define VER_SUB 0

#define STR(s)  #s
#define XSTR(s) STR(s)
#define VERS    "v" XSTR(VER_MAJ) "." XSTR(VER_MIN) "." XSTR(VER_SUB)

//------------------------------------------------------------------------------ ---------------------------------------
#define MBOX_USED  3

//============================================================================== =======================================
void  help         (void) ;
void  serial_setup (void) ;
bool  can_setup    (void) ;
void  mbox_setup   (void) ;
void  eth_setup    (void) ;

#endif // SETUP_H_

