#include "macros.h"

//+============================================================================= =======================================
// Print decimal numbers with a comma every three digits
// Do NOT call directly ...call with the BIGDEC macro
//
void  bigdec (int n,  int d)
{
  if (n >= 10)  bigdec(n / 10, d + 1) ;
  SAY(n % 10, DEC);
  if (d && !(d % 3))  FSAY(",") ;
}


