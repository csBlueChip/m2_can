#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE
#include <stdlib.h>

#include "can_rb.h"
#include "can_bus.h"
#include "can_intr.h"
#include "can_intr_si.h"

//+============================================================================= =======================================
// Here we are playing with the ring buffers from userland when interrupts may be enabled
// We must disable the relevant interrupts while playing
//
static inline
void  _rb_reset (ringBuffer_t* ring)
{
	ring->len  = 0;
	ring->head = 0;
	ring->tail = 0;
}

//+=============================================================================
// Delete everything in the ring buffer - useful when changing mailbox modes
//
bool  rb_reset (int bus,  int rb)
{
	if (intr_get(bus, intr_mask_mbox[rb])) {  // Interrupts are running for this mailbox
		intr_disable(bus, intr_mask_mbox[rb]);
		{
			_rb_reset(&_can[bus].rb[rb]);
		}
		intr_enable(bus, intr_mask_mbox[rb]);

	} else {
		_rb_reset(&_can[bus].rb[rb]);
	}

	return true;
}

//+=============================================================================
// Delete everything in ALL ring buffers
//
bool  rb_reset_all (int bus)
{
	uint32_t  intr = intr_get(bus, intr_mask_mbox[MBOX_CNT]);  // All mailboxes

	intr_disable(bus, intr);
	{
		for (int rb = 0;  rb < RB_CNT;  rb++)  _rb_reset(&_can[bus].rb[rb]) ;
	}
	intr_enable(bus, intr);

	return true;
}

//+=============================================================================
// Initialise a buffer to an EMPTY buffer of SIZE elements
// ...This will happily handle resizing an existing buffer
//
// Don't overuse this function - it uses alloc() which may cause RAM fragmentation
//   and ultimately, if the alloc() is badly implemented [it's Arduino!],
//   you will run out of RAM and horrible (undefined) things will happen
//
bool  rb_set (int bus,  int rb,  rbSize_t size)
{
	bool           rv   = true;  // presume success
	ringBuffer_t*  ring = &_can[bus].rb[rb];
	uint32_t       intr = intr_get(bus, intr_mask_mbox[rb]);

	intr_disable(bus, intr);
	{
		_rb_reset(ring);                   // Empty the ring

		if (size != ring->size) {          // Size changed
			if (size == RBSIZ_DISABLE) {   //   New size is 0
				free(ring->frame);
				ring->size = RBSIZ_DISABLE;

			} else {                       //   New size is non-0
				int  allocSize = sizeof(*(ring->frame)) * (size + 1); // yes, size+1, see notes for rbSize_t

				if (ring->size == RBSIZ_DISABLE)  // Only in fkdup Arduino world do you have to cast malloc()
					ring->frame = (canFrame_t*) malloc(             allocSize) ;  // Old size  = 0
				else
					ring->frame = (canFrame_t*)realloc(ring->frame, allocSize) ;  // Old size != 0

				if (!ring->frame) {        //     The alloc() failed
					ring->size = RBSIZ_DISABLE;
					rv         = false;
				} else {                   //     The alloc() went OK
					ring->size = size;
				}
			}
		}
	}
	intr_enable(bus, intr);

	return rv ;
}

//+============================================================================= =======================================
// An Rx ring buffer should only ever be added to via an interrupt
// ...a given mailbox will not generate another interrupt until we say we're done with the last one
// ...If you try to add a frame to a receive buffer from userland ...you're a dick!
// A Tx ring buffer will ONLY ever be used from userland
// ...You can't call ANY two functions at the same time from userland
// One 'thread' can safely add to the head of the ring buffer,
// while another 'thread' removes from the tail of that same ring buffer
// ...If there's a space available at the first check, nothing is going to change that!
// ...Again, if you try to add a frame to a receive buffer from userland ...you're a dick!
// So the only place we have any locking issues is getting an interrupt during setup/reconfiguration
// ...For the rare occasion which that code gets called from userland - it has locking in it!
//
canFrame_t*  _rb_frame_add (int bus,  int rb,  const canFrame_t* frame)
{
	ringBuffer_t*  ring = &_can[bus].rb[rb];
	unsigned int   next = (ring->head + 1) & ring->size ;  // Find next slot

	if (next != ring->tail) {                              // If next slot is available
		canFrame_t*  fp = &ring->frame[ring->head];        // Pointer to the 'head' frame
		if (frame)  memcpy(fp, frame, sizeof(*fp)) ;       // (IF SUPPLIED) Copy data to buffer
		ring->head = next;                                 // head = next_free slot
		ring->len++;                                       // Track the length
		return fp;                                         // Success
	}

	return NULL;
}

//+============================================================================= =======================================
// An Rx ring buffer will NEVER have entries retrieved by an interrupt
// ...And, you can't call ANY two functions at the same time from userland
// A Tx ring buffer will ONLY ever be used from userland
// ...Again, you can't call ANY two functions at the same time from userland
// One 'thread' can safely remove from the tail of the ring buffer,
// while another 'thread' adds to the head of that same ring buffer.
// ...If there's a frame available at the first check, nothing is going to change that!
// So locking is never going to be an issue here
//
bool  rb_frame_get (int bus,  int rb,  canFrame_t* frame)
{
	ringBuffer_t*  ring = &_can[bus].rb[rb];

	if (ring->len) {
		memcpy(frame, &(ring->frame[ring->tail]), sizeof(*frame));
		ring->tail = (ring->tail + 1) & ring->size;
		ring->len--;
		return true;
	}

	return false;
}
