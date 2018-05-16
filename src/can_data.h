#ifndef CAN_ENDIAN_H_
#define CAN_ENDIAN_H_

#include <stdint.h>

//------------------------------------------------------------------------------ ---------------------------------------
// Set the endian'ness here
//
#define ENDIAN_BIG    (1)
#define ENDIAN_LITTLE (2)            // Macchina M2 aka Arduino Due aka Atmel SAM
#define ENDIAN        ENDIAN_LITTLE  // There's a test harness at the end of this file you can use to work this out

//------------------------------------------------------------------------------ ---------------------------------------
#if (ENDIAN == ENDIAN_LITTLE)  // Tested :)
	typedef
		union {
			uint64_t      md;
			struct {
				uint32_t  mdl;
				uint32_t  mdh;
			};
			struct {
				uint16_t  mdll;
				uint16_t  mdlh;
				uint16_t  mdhl;
				uint16_t  mdhh;
			};
			struct {
				uint8_t   b7;  // mdl[ 7.. 0]
				uint8_t   b6;  // mdl[15.. 8]
				uint8_t   b5;  // mdl[23..16]
				uint8_t   b4;  // mdl[31..24]
				uint8_t   b3;  // mdh[ 7.. 0]
				uint8_t   b2;  // mdh[15.. 8]
				uint8_t   b1;  // mdh[23..16]
				uint8_t   b0;  // mdh[31..24]
			};
		}
	canData_t;

	typedef
	union {
		uint32_t  time;
		struct {
			uint16_t  can;  // [15..00] : Time read from CAN controller
			uint16_t  ovr;  // [31..16] : Number of times the timer has overflowed
		};
	}
	canTime_t;

#elif (ENDIAN == ENDIAN_BIG) //! No BE machine to hand - this has not been tested!
	typedef
		union {
			uint64_t      md;
			struct {
				uint32_t  mdh;
				uint32_t  mdl;
			};
			struct {
				uint16_t  mdhh;
				uint16_t  mdhl;
				uint16_t  mdlh;
				uint16_t  mdll;
			};
			struct {
				uint8_t   b0;  // mdh[31..24]
				uint8_t   b1;  // mdh[23..16]
				uint8_t   b2;  // mdh[15.. 8]
				uint8_t   b3;  // mdh[ 7.. 0]
				uint8_t   b4;  // mdl[31..24]
				uint8_t   b5;  // mdl[23..16]
				uint8_t   b6;  // mdl[15.. 8]
				uint8_t   b7;  // mdl[ 7.. 0]
			};
		}
	canData_t;

	typedef
	union {
		uint32_t  time;
		struct {
			uint16_t  ovr;  // [31..16] : Number of times the timer has overflowed
			uint16_t  can;  // [15..00] : Time read from CAN controller
		};
	}
	canTime_t;
#endif

//------------------------------------------------------------------------------ ---------------------------------------
// Feel free to use this test harness
//
#if 0

//+===================================== C test harness
#include <stdint.h>
#include <stdio.h>
int main (int argc,  char* argv[],  char* envv[])
{
	uint16_t  end = 0x0100;
	uint8_t*  ep  = (uint8_t*)&end;
	printf("Processor Endian'ness: %s\n", *ep ? "big" : "little");

	canData_t  x;
	x.md = 0x1122334455667788;
	printf("mdh  (11223344) = |%8X|\n", x.mdh  );  // (expected) = |actual|
	printf("mdl  (55667788) = |%8X|\n", x.mdl  );
	printf("\n");
	printf("mdhh (    1122) = |%8X|\n", x.mdhh );
	printf("mdhl (    3344) = |%8X|\n", x.mdhl );
	printf("mdlh (    5566) = |%8X|\n", x.mdlh );
	printf("mdll (    7788) = |%8X|\n", x.mdll );
	printf("\n");
	printf("b1   (      11) = |%8X|\n", x.b1   );
	printf("b2   (      22) = |%8X|\n", x.b2   );
	printf("b3   (      33) = |%8X|\n", x.b3   );
	printf("b4   (      44) = |%8X|\n", x.b4   );
	printf("b5   (      55) = |%8X|\n", x.b5   );
	printf("b6   (      66) = |%8X|\n", x.b6   );
	printf("b7   (      77) = |%8X|\n", x.b7   );
	printf("b8   (      88) = |%8X|\n", x.b8   );
	x.md = 0x99AA;
	printf("mdh  (        ) = |%8X|\n", x.mdh  );  // (expected) = |actual|
	printf("mdl  (    99AA) = |%8X|\n", x.mdl  );
	printf("\n");
	printf("mdhh (        ) = |%8X|\n", x.mdhh );
	printf("mdhl (        ) = |%8X|\n", x.mdhl );
	printf("mdlh (        ) = |%8X|\n", x.mdlh );
	printf("mdll (    99AA) = |%8X|\n", x.mdll );
	printf("\n");
	printf("b7   (      AA) = |%8X|\n", x.b7   );
	printf("b6   (      99) = |%8X|\n", x.b6   );
	printf("b5   (        ) = |%8X|\n", x.b5   );
	printf("b4   (        ) = |%8X|\n", x.b4   );
	printf("b3   (        ) = |%8X|\n", x.b3   );
	printf("b2   (        ) = |%8X|\n", x.b2   );
	printf("b1   (        ) = |%8X|\n", x.b1   );
	printf("b0   (        ) = |%8X|\n", x.b0   );

	return 0;
}

//+===================================== An equaivalent arduino sketch
#include <stdint.h>
#define Serial SerialUSB  // for Macchina
void  setup (void) { }
void  loop  (void)
{
	uint16_t  end = 0x0100;
	uint8_t*  ep  = (uint8_t*)&end;
	Serial.print("Processor Endian'ness: ");  Serial.println(*ep ? "big" : "little");

	canData_t  x;
	x.md = 0x1122334455667788;
	Serial.print("mdh  (11223344) = ");  Serial.println(x.mdh , HEX);  // (expected) = actual
	Serial.print("mdl  (55667788) = ");  Serial.println(x.mdl , HEX);
	Serial.println("");
	Serial.print("mdhh (    1122) = ");  Serial.println(x.mdhh, HEX);
	Serial.print("mdhl (    3344) = ");  Serial.println(x.mdhl, HEX);
	Serial.print("mdlh (    5566) = ");  Serial.println(x.mdlh, HEX);
	Serial.print("mdll (    7788) = ");  Serial.println(x.mdll, HEX);
	Serial.println("");
	Serial.print("b7   (      88) = ");  Serial.println(x.b7  , HEX);
	Serial.print("b6   (      77) = ");  Serial.println(x.b6  , HEX);
	Serial.print("b5   (      66) = ");  Serial.println(x.b5  , HEX);
	Serial.print("b4   (      55) = ");  Serial.println(x.b4  , HEX);
	Serial.print("b3   (      44) = ");  Serial.println(x.b3  , HEX);
	Serial.print("b2   (      33) = ");  Serial.println(x.b2  , HEX);
	Serial.print("b1   (      22) = ");  Serial.println(x.b1  , HEX);
	Serial.print("b0   (      11) = ");  Serial.println(x.b0  , HEX);
	Serial.println("");

	x.md = 0x99AA;
	Serial.print("mdh  (        ) = ");  Serial.println(x.mdh , HEX);  // (expected) = actual
	Serial.print("mdl  (    99AA) = ");  Serial.println(x.mdl , HEX);
	Serial.println("");
	Serial.print("mdhh (        ) = ");  Serial.println(x.mdhh, HEX);
	Serial.print("mdhl (        ) = ");  Serial.println(x.mdhl, HEX);
	Serial.print("mdlh (        ) = ");  Serial.println(x.mdlh, HEX);
	Serial.print("mdll (    99AA) = ");  Serial.println(x.mdll, HEX);
	Serial.println("");
	Serial.print("b7   (      AA) = ");  Serial.println(x.b7  , HEX);
	Serial.print("b6   (      99) = ");  Serial.println(x.b6  , HEX);
	Serial.print("b5   (        ) = ");  Serial.println(x.b5  , HEX);
	Serial.print("b4   (        ) = ");  Serial.println(x.b4  , HEX);
	Serial.print("b3   (        ) = ");  Serial.println(x.b3  , HEX);
	Serial.print("b2   (        ) = ");  Serial.println(x.b2  , HEX);
	Serial.print("b1   (        ) = ");  Serial.println(x.b1  , HEX);
	Serial.print("b0   (        ) = ");  Serial.println(x.b0  , HEX);
	Serial.println("");
}

#endif // 0

#endif // CAN_ENDIAN_H_
