=========
 Licence
=========

MIT License

Copyright (c) 2018 BlueChip

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


========================
 Code Design Philosophy
========================

This driver code has VERY limited error checking it.
Yep, one could easily argue this is a security risk - and as such, you are STRONGLY ADVISED
         === DO *NOT* USE THIS CODE IN COMMERCIAL OR CRITICAL APPLICATIONS ===
...unless you perform a FULL and PROFESSIONAL Code Audit ...And add a LOT of error checking to it!
That's not a license restriction - it's just common sense!
The license merely says if you ignore good advice and common sense, it's not my problem.
Then again, if you're putting code you found on the internet in critical/commercial applications
  without a FULL and PROFESSIONAL Code Audit, you're a moron and this advice is probably worthless anyway.

Why didn't I add shitloads of error checking?
'Cos I want super-fast code for my specific use-case!
I.E. Code written by an experienced programmer, not an Arduino hobbyist.
Checks you would want to make include, but are not necessarily limited to:
	# Always check the bus number passed to a (public) function is validated
	# Mailbox numbers
	# Ring buffer numbers
	# NULL pointers
	# basically, all your input prams
	# ...probably a shitload more stuff, but AIS, that's your problem, not mine

Moreover, this code is designed for SPEED - there are a (very) LOT of 'static inline' functions.
This is going to make for quite bloated code. So:
	If you start to run out of space for your executable,
	you will need to convert these functions back in to /actual/ functions!
		1. Copy them in to their respective C file
		2. Change the function prototype in the H file
		3. Clean up the code formatting - yeah, I know, but I'd be remiss not to at least say it, eh?

You will probably notice that the file extension does not match the language.
The code is (excepting the use of // commenting) 100.0% C, but the file extension is .cpp
This use of .cpp is purely to make the Arduino IDE compile things properly.

It compiles with ZERO Errors and ZERO Warnings
Note to other developers: A warning is a bug waiting to happen - fix it!
IE. gcc -Werror ... "Make all warnings into errors"

================
 Commenting Key
================

C++ commenting *is* used throughout, because it is supported by pretty much every compiler.
This is to achieve multiple levels of commenting.  IE:
	#if 0
		/*
			// Comment
		*/
	#endif

Beyond that:
	//-   definition(s)
	//!   alert
	//+   function
	//~   code section
	//@   static inline function
	///   delete on next code tidyup


==============
 Bibliogrpahy
==============

Chipset
	http://ww1.microchip.com/downloads/en/devicedoc/atmel-11057-32-bit-cortex-m3-microcontroller-sam3x-sam3a_datasheet.pdf
	pp1185..1242 - CAN Controller

Macchina M2
Specification
	https://docs.macchina.cc/m2/
Schematics
	https://github.com/macchina/m2-hardware/tree/master/M2

Bosch CAN Spec
	https://www.kvaser.com/software/7330130980914/V1/can2spec.pdf

Interesting comments on RTR frames
	https://community.st.com/community/partner-community/microcontrol/blog/2018/02/04/can-remote-frames-hands-off

Texas Instruments Introduction to CAN bus
	http://www.ti.com/lit/an/sloa101b/sloa101b.pdf

Wikipedia Article on CAN bus
	https://en.wikipedia.org/wiki/CAN_bus


==========================
 Packet Rate & TimeStamps
==========================

https://en.wikipedia.org/wiki/CAN_bus#Data_frame

Smallest frame: standard frame, no data,  no stuff bits, including frame interspace = 47
Test     frame: extended frame, no data,  no stuff bits, including frame interspace = 67
Largest  frame: extended frame, 8  data, max stuff bits, including frame interspace = 160
                                         (67 + (8*8)) + ( ((67-13) + (8*8) -1) / 4)

At 1000Kbps (1Mbps), you can (theoretcially) receive (6.3 < N < 21.3) frames/mS
...that's one frame every n uS, where n is the number of bits/frame
Even at 250Kbps, that's still (~1.6 < N < 5.6) frames/mS

A millisecond accuracy timer is just not good enough!
And even if it were, millis() runs on an interrupt.
As this code is heavily interrupt driven, we cannot rely on millis() to be accurate!

The micros() function seems more useful - but it will overflow after ~71.5 minutes (0xffffffff/1000000/60)
Inside an interrupt things get worse, the actual timer rolls over every 256uS,
...then hooks an interrupt to count the overflows.
We could probably use this - being careful that
  a) Our ISR is <255uS
  b) We allow for rollover during the interrupt

Then there's the "timer" on the CAN controller itself.
But many issues here:
	1) We have no idea the length of 1 tick
	   ...Actually, given this is code specifically for the Macchina M2, we could probably work this out.
	2) We still need to implement an overflow interrupt
	3) There are several conditions which will break it
		a) Using Time-Triggered mode
		b) Using Timer-Freeze mode
	   Both of which stop the timer from advancing

On the winning side, our Rx interrupt loop MUST be <47uS to allow for the maximum frame-rate.
So it looks like micros() will have to be the solution!

Realistically, we're not achieving any form of realtime timestamp with an RTC.

===========================
 pp1211 - Register Mapping
===========================

I really should get around to completing this section :s

Rst : Reset value
RW  : Read/Write
WP  : Write Protect'able - pp1210

	Reserved dwords:
	~~~~~~~~~~~~~~~~
		0x002C - 0x00E0
		0x00EC - 0x01FC

	CAN : 0x0000 + ...
	~~~~~~~~~~~~~~~~~~
		Offset   Rst   RW   WP   Ref Pages     Name          Description
		------- ----- ---- ---- ------------- ------------- --------------------------------
		0x0000    0    RW   **   pp1212        CAN_MR        Mode Register
		                                                     : CANEN  - CAN bus ENable
		                         pp1187                      : LPM    - Low Power Mode
		                                                     : ABM    - Auto Baud Mode (Listen)
		                                                     : OVL    - Generate OVerLoad Frame
		                                                     : TEOF   - (CAN_TIM = CAN-TIMESTP) @ EOF (not SOF)
		                                                     : TTM    - Time Trigger Mode
		                         pp1208                      : TIMFRZ - Freeze Timer [pp1208]
		                         pp1213                      : DRPT   - Disable Tx Repeat (CAN_MSR = MABT | MRDT)
		                                                     : RXSYNC -

		0x0004    -    -W        pp            CAN_IER       Interrupt Enable Register
		0x0008    -    -W        pp            CAN_IDR       Interrupt Disable Register
		0x000C    0    R-        pp            CAN_IMR       Interrupt Mask Register

		0x0010    0    R-        pp            CAN_SR        Status Register
		0x0014    0    RW   **   pp1192-1195   CAN_BR        Baudrate Register

		0x0018    0    R-        pp            CAN_TIM       Timer Register
		0x001C    0    R-        pp            CAN_TIMESTP   Timestamp Register
		0x0020    0    R-        pp            CAN_ECR       Error Counter Register

		0x0024    -    -W        pp            CAN_TCR       Transfer Command Register
		0x0028    -    -W        pp            CAN_ACR       Abort Command Register

		0x00E4    0    RW        pp            CAN_WPMR      Write Protect Mode Register
		0x00E8    0    R-        pp            CAN_WPSR      Write Protect Status Register

	Message : 0x0200 + (MB * 0x20) + ...
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		  0x00    0    RW   **   pp            CAN_MMR       Mode Register
		  0x04    0    RW   **   pp1189        CAN_MAM       Message Acceptance Mask Register
		  0x08    0    RW   **   pp1189        CAN_MID       Message ID Register

		  0x0C    0    R-        pp1189        CAN_MFID      Family ID Register
		  0x10    0    R-        pp            CAN_MSR       Status Register
		  0x14    0    RW        pp            CAN_MDL       Data Low Register
		  0x18    0    RW        pp            CAN_MDH       Data High Register
		  0x1C    -    -W        pp            CAN_MCR       Control Register


====================================================
 Atmel SAM cannot generate RTR packets with DLC > 0
====================================================
...with a "Transmitter" Mailbox

Wikipedia says:

https://en.wikipedia.org/wiki/CAN_bus#Remote_frame
"There are two differences between a Data Frame and a Remote Frame.
Firstly the RTR-bit is transmitted as a dominant bit in the Data Frame
and secondly in the Remote Frame there is no Data Field.
The DLC field indicates the data length of the requested message (not the transmitted one)"

Bosch says:

https://www.kvaser.com/software/7330130980914/V1/can2spec.pdf
page 15 : 3.1.2
"Contrary to DATA FRAMEs, the RTR bit of REMOTE FRAMEs is ’recessive’. There is
no DATA FIELD, independent of the values of the DATA LENGTH CODE which may
be signed any value within the admissible range 0...8. The value is the DATA LENGTH
CODE of the corresponding DATA FRAME."
...which could well say the same thing as Wikipedia - just not in English.

However Atmel says:

http://ww1.microchip.com/downloads/en/devicedoc/atmel-11057-32-bit-cortex-m3-microcontroller-sam3x-sam3a_datasheet.pdf
page 1204 : 40.8.3.2
"It is also possible to send a remote frame setting the MRTR bit **instead of** setting the MDLC field"

So, I ask:

If you can set RTR **or** DLC
...how do you specify the number of bytes you wish to receive in response in the RTR reply ?

Then I found this:

https://community.st.com/community/partner-community/microcontrol/blog/2018/02/04/can-remote-frames-hands-off
"Another - still present - issue are incorrect implementations by silicon vendors.
Either the chip does not respect the DLC value (forcing it to 0 for every CAN Remote frame)
or it starts transmission of CAN Error frames upon reception of certain CAN Remote frame."


=================
 MID / MAM / FID
=================

Referring to SAM datasheet...

# pp1232 .. Building a MessageAcceptanceMask
	if (frame_type == extended)  MAM = (1 << 29) | (0x1FFF'FFFF)
	else                         MAM = (0 << 29) | (0x3FF << 18)
...where the 'literal hex values' [hex literals] are your mask

# pp1189 .. Accepting a message
if (mailbox_MID & mailbox_MAM) == (message_MID & mailbox_MAM)
   frame is a match for this mailbox

Now the family ID...
This is a badly explained stroke of genius
...and it's only in the SAM as far as I can tell

The clue was in the block of code at the bottom of pp1189
which calls a function from an array of function pointers
When you understand, the example on page-1234 is a dead giveaway

	CAN_MIDx  = 0x305A4321
	CAN_MAMx  = 0x3FF0F0FF
	CAN_MFIDx = 0x000000A3

basically all the bits you ignored to get the match
are grouped up and pushed to the right
this creates and "ID" *within* the "Family" specified by the mask

A less obvious example might be
CAN_MIDx  = 0x305A4321
CAN_MAMx  = 0x3FC9F7F8
CAN_MFIDx = 0x00000051

let's convert that to binary:
   MID: 305A'4321 = 0011 0000 0101 1010 ' 0100 0011 0010 0001
   MAM: 3FC9'F7F8 = 0011 1111 1100 1001 ' 1111 0111 1111 1000

We can completely ignore the top two bits 'cos they're not used
The next bit (2^29) is actually used as the standard/extended-frame flag
If you're working with standard frames you may want to deal with the << 18 before you continue
So we are left with only the 29bits of the MID ...ie the Arbitration ID (AID)

These bits are grouped in two:
	"Family"    bits ...ie. bits that must match to accept the frame       ...ie. the 1's in MAM
	"Family ID" bits ...ie. bits that identify the frame within the family ...ie. the 0's in MAM

	MID: 105A'4321 = ---1 0000 0101 1010 ' 0100 0011 0010 0001
	MAM: 1FC9'F7F8 = ---1 1111 1100 1001 ' 1111 0111 1111 1000
		FID Mask :      . .... ..** .**.   .... *... .... .***
		FID bits :               01  01         0          001  =>  FID = 0101'0001 == 0x51 == 0x00000051
...that would be a VERY expensive calculation in software

The end result is the smallest number (of values) possible to represent the
ID of the frame you have just received within the Acceptance Group/Family


=========================
 Producer/Consumer Model
=========================

Reference
---------
http://ww1.microchip.com/downloads/en/devicedoc/atmel-11057-32-bit-cortex-m3-microcontroller-sam3x-sam3a_datasheet.pdf
Page 1205, Section 40.8.3.3

Scenario
--------
In this example we have two devices,
each is sending data to the other
EG. The Dash-Computer wants to make sure the Dash Display accurately reflects the state of the Lamps
	Dash-Computer queries the state of the Lamps
		Lamp-Computer receives the query
		Lamp-Computer sends Lamp Status
	Dash-Computer receives the reply
	...Dash-Computer updates the Dash Display

Push Model
----------
We are used to operating in the "Push Model"
...AKA. "Broadcast Model"
...AKA. "!RTR" [!"Remote Transmission Request"]:

In the Push Model
	A "Producer" Broadcasts a frame
	A "Consumer" receives a frame

So, for our simple sceanrio, BOTH the Dash-Computer AND the Lamp-Computer require:
	1 @ Mailbox as Tx
	1 @ Mailbox as Rx

I should guess it's all pretty obvious from there.  In brevity:
	Dash-Computer:
		send_lights(query_state) ;
		if (frame_received())  setDashState(frame.data) ;
	Lamp-Computer:
		if (frame_received(query_state)) send_dash(lamps_state) ;

We have a total of EIGHT Mailboxes (in each Controller)
So we can replicate this scenario FOUR times within a single controller.
Eg.  Dash -1-> Lights ... 0=Rx, 1=Tx
          -2-> Doors  ... 2=Rx, 3=Tx
          -3-> Seats  ... 4=Rx, 5=Tx
          -4-> AirCon ... 6=Rx, 7=Tx
          -5-> Radio  ... Sorry, we're out of Mailboxes. Please add more Controllers!

Bad Jargon
----------
If I ask for a coffee, and you supply one - I am the Consumer and you are the Producer.
Here, the Dash-Computer makes a request of the Lamp-Computer, and is suitably patronised.
So the Dash-Computer is the "Consumer" and the Lamp-Computer is the "Producer"
-HOWEVER-
We are running in the Push Model
So really, they are both JUST Transmitting or Receiving frames,
...The *terms* "Producer" and "Consumer" are 'conceptual' at best.

Pull Model
----------
The *words* "Producer" and "Consumer" take on 'proper-noun' value when we consider the "Pull Model"
...AKA. "Producer/Consumer Model"
...AKA. "RTR" ["Remote Transmission Request"]:

In the Pull Model everything works the same as the Push Model
...with ONE *significant* exception:

We can instruct ONE Mailbox to act as both Transmitter AND Receiver
More specifically, we could set the:
	Dash-Computer:Mailbox[1] to be a Consumer [Transmit-then-Receive]
	Lamp-Computer:Mailbox[1] to be a Producer [Receive-then-Transmit]

The Consumer, will put a frame in a/the "Consumer" Mailbox
...and issue the Transmit command
The Controller will Transmit the [RTR] frame
...and immediately set the Mailbox to be a Receiver
Once the reply has been received, I *think* you just goto 10 - the datasheet doesn't make that clear

For reasons explained in the paragraph over pages 1206/1207
You MAY be wise to ensure your Consumer Mailboxes are ALWAYS the lowest numbered Mailboxes!

The Producer, will put a reply frame in to a/the "Producer" Mailbox
...And put the that Mailbox in Ready/Listen mode
When that Mailbox Receives/Sees a frame
...It will immediately send the pre-prepared frame back to the Consumer

Obviously this will mean careful configuration of the MID and MAM fields
...to make sure mailboxes only respond to appropriate frames

With the Pull Model, we can replicate this scenario EIGHT times within a single controller
...Potentially halving the number of Controllers required for a single Computer

==========
 Test Rig
==========

                  [120]-.-[BREAKOUT]-----+-----[120]
                        |                |
[MACCHINA-M2]------.    |                |
[MACCHINA M2]------->---'                `-----[ECU]
[OSCILLOSCOPE]-----'                     

