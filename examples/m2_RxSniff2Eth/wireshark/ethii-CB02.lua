------------------------------------------------------------------------------- ---------------------------------------
-- There are probably "better" ways to do this,
-- but I'm happy with this as my first ever attempt at a Wireshard Dissector
-- ...and my first fight with LUA ...I mean, really guys, WTF? Just HOW badly can you screw up a scripting language?
--

------------------------------------------------------------------------------- ---------------------------------------
-- create cb02 protocol
--
p_cb02 = Proto("CAN02", "CANbus")

------------------------------------------------------------------------------- ---------------------------------------
-- create cb02 fields
--
local f_fr1    = ProtoField.bytes ("cb02.fr1" , "Frame #1 "     , base.SPACE)
local f_fr2    = ProtoField.bytes ("cb02.fr2" , "Frame #2 "     , base.SPACE)

local f_dev    = ProtoField.uint8 ("cb02.dev" , "Device Info "  , base.HEX)
  local f_dir  = ProtoField.uint8 ("cb02.dir" , "Direction"     , base.BIN, NULL, 0x80)
  local f_note = ProtoField.uint8 ("cb02.note", "Note     "     , base.BIN, NULL, 0x40)
  local f_bus  = ProtoField.uint8 ("cb02.bus" , "Bus      "     , base.DEC, NULL, 0x38)
  local f_mbox = ProtoField.uint8 ("cb02.mbox", "Mailbox  "     , base.DEC, NULL, 0x07)

local f_ts     = ProtoField.uint32("cb02.ts"  , "Timestamp   "  , base.DEC)

local f_mid    = ProtoField.uint32("cb02.mid" , "Message ID  "  , base.HEX)
  local f_mx1  = ProtoField.uint32("cb02.mx1" , "-Unused-      ", base.DEC, NULL, 0xC0000000)
  local f_ext  = ProtoField.uint32("cb02.ext" , "CANBus Version", base.DEC, NULL, 0x20000000)
  local f_aide = ProtoField.uint32("cb02.aide", "Arbitration ID", base.HEX, NULL, 0x1FFFFFFF)
  local f_mx2  = ProtoField.uint32("cb02.mx2" , "-Unused-      ", base.HEX, NULL, 0x1FFFF800)
  local f_aids = ProtoField.uint32("cb02.aids", "Arbitration ID", base.HEX, NULL, 0x000007FF)

local f_fid    = ProtoField.uint32("cb02.fid" , "Family ID   "  , base.HEX)

local f_dinf   = ProtoField.uint8 ("cb02.dinf", "Data Info   "  , base.HEX)
  local f_rtr  = ProtoField.uint8 ("cb02.rtr" , "Remote Tx Req" , base.DEC, NULL, 0x80)
  local f_dxx  = ProtoField.uint8 ("cb02.dxx" , "-Unused-     " , base.DEC, NULL, 0x70)
  local f_dlc  = ProtoField.uint8 ("cb02.dlc" , "Data Length  " , base.DEC, NULL, 0x0F)

local f_data   = ProtoField.bytes ("cb02.data" , "Data        " , base.SPACE)
  local f_dact = ProtoField.bytes ("cb02.dact" , "Active   "    , base.SPACE)
  local f_dpad = ProtoField.bytes ("cb02.dpad" , "Padding  "    , base.SPACE)

local f_epad   = ProtoField.bytes ("cb02.epad" , "Eth Pad  " , base.SPACE)

------------------------------------------------------------------------------- ---------------------------------------
-- Add fields to dissector
--
p_cb02.fields = {f_fr1, f_fr2,
                 f_dev, f_dir, f_note, f_bus, f_mbox,
                 f_ts,
				 f_mid, f_mx1, f_ext, f_aide, f_mx2, f_aids,
				 f_fid,
				 f_dinf, f_rtr, f_dxx, f_dlc,
				 f_data, f_dact, f_dpad,
				 f_epad}

--+============================================================================ =======================================
-- Dissect one frame
--
--  [-] CANbus, A pair of CANbus frames
--     
--      [-] Frame #1 : xx xx xx xx xx xx xx xx xx xx xx ...                                     cb02.fr1
--          [-] Device Info : 0xC0 [dnbb'bmmm]                                                    cb02.dev
--                1... .... = Direction: 1 [Tx]                                                     cb02.dir
--                .1.. .... = Note     : 1 [Generated]                                              cb02.note
--                ..00 0... = Bus      : 0                                                          cb02.bus
--                .... .000 = Mailbox  : 0                                                          cb02.mbox
--              Timestamp   : 0 [unused]                                                          cb02.ts
--          [-] Message ID  : 0x00000155                                                          cb02.mid
--                ..0. .... .... .... .... .... .... .... = CANBus Version: 0 [2.0a/standard]       cb02.ext
--                .... .... .... .... .... .101 0101 0101 = Arbitration ID: 0x555                   cb02.aids
--              Family ID   : 0x00000555 [SAM]                                                    cb02.fid
--          [-] Data Info   : 0x80 [rxxx'llll]                                                    cb02.dinf
--                0... .... = Remote Tx Req: 1                                                      cb02.rtr
--                .000 .... = -Unused-     : 0                                                      cb02.dxx
--                .... 0000 = Data Length  : 0                                                      cb02.dlc
--          [-] Data        : 01 02 03 04 05 06 07 08                                             cb02.data
--                Active    : <MISSING>                                                             cb02.dact
--                Padding   : 01 02 03 04 05 06 07 08                                               cb02.dpad
--                                                                                              
--      [-] Frame #2 : xx xx xx xx xx xx xx xx xx xx xx ...                                     cb02.fr2
--          [-] Device Info : 0x00 [dnbb'bmmm]                                                    cb02.dev
--                0... .... = Direction: 0 [Rx]                                                     cb02.dir
--                .0.. .... = Note     : 0 [Not Acked]                                              cb02.note
--                ..00 0... = Bus      : 0                                                          cb02.bus
--                .... .000 = Mailbox  : 0                                                          cb02.mbox
--              Timestamp   : 0 [unused]                                                          cb02.ts
--          [-] Message ID  : 0x35555555                                                          cb02.mid
--                ..1. .... .... .... .... .... .... .... = CANBus Version: 1 [2.0b/extended]       cb02.ext
--                ...1 0101 0101 0101 0101 0101 0101 0101 = Arbitration ID: 0x15555555              cb02.aide
--              Family ID   : 0x15555555 [SAM]                                                    cb02.fid
--          [-] Data Info   : 0x03 [rxxx'llll]                                                    cb02.dinf
--                0... .... = Remote Tx Req: 0                                                      cb02.rtr
--                .000 .... = -Unused-     : 0                                                      cb02.dxx
--                .... 0000 = Data Length  : 3                                                      cb02.dlc
--          [-] Data        : 01 02 03 04 05 06 07 08                                             cb02.data
--                Active    : 01 02 03                                                              cb02.dact
--                Padding   : 04 05 06 07 08                                                        cb02.dpad
--                                                                                              
--          Eth Pad  : 00 00  [Ethernet Frame Padding]                                          cb02.epad
--

local function  frame (buf, pkt, t_fr)
	local  n  -- values read from buffer
	
	-- indentation matches TREE structure, NOT code strcuture!
	
	t_dev = t_fr:add(f_dev, buf(0, 1)):append_text(" [dnbb'bmmm]")         -- cb02.dev
		n = buf(0, 1):uint()                                               -- 
		if bit.band(bit.rshift(n, 7), 1) == 1 then                         --   
			t_dev:add(f_dir, buf(0, 1)):append_text(" [Tx]")               --   cb02.dir = Tx
			if bit.band(bit.rshift(n, 6), 1) == 1 then                     --     
				t_dev:add(f_note, buf(0, 1)):append_text(" [Generated]")   --     cb02.note = Generated
			else                                                           --     
				t_dev:add(f_note, buf(0, 1)):append_text(" [Forwarded]")   --     cb02.note = Forwarded
			end                                                            -- 
		else                                                               --   
			t_dev:add(f_dir, buf(0, 1)):append_text(" [Rx]")               --   cb02.dir = Rx
			if bit.band(bit.rshift(n, 6), 1) == 1 then                     --     
				t_dev:add(f_note, buf(0, 1)):append_text(" [ACKed]")       --     cb02.note = Acked
			else                                                           --     
				t_dev:add(f_note, buf(0, 1)):append_text(" [NOT ACKed]")   --     cb02.note = NOT Acked
			end                                                            -- 
		end                                                                -- 
		t_dev:add(f_bus , buf(0, 1))                                       --   cb02.bus
		t_dev:add(f_mbox, buf(0, 1))                                       --   cb02.mbox

	t_ts = t_fr:add(f_ts, buf(1, 4)):append_text(" [unused]")              -- cb02.ts

	--
	-- What I actually want to do here is add EITHER 'aids' or 'aide' to the screen (as below)
	-- BUT also create a field called 'aid' which is NOT displayed in the Dissector window of Wireshark
	-- ...but CAN be displayed in the Packet window
	-- 'aid' will be set equal to 'aids' or 'aide' appropriately
	-- This will allow you to display 'aid[1]' and 'aid[2]' in the Packet window
	--
	t_mid = t_fr:add(f_mid, buf(5, 4)):append_text("")                     -- cb02.mid
--		t_mid:add(f_mx1, buf(5, 4))                                        --   cb02_mx1
		n = buf(5, 4):uint()                                               -- 
		if bit.band(bit.rshift(n, 29), 1) == 1 then                        --   
			t_mid:add(f_ext, buf(5, 4)):append_text(" [2.0b/extended]")    --   cb02.ext = extended
			t_mid:add(f_aide, buf(5, 4))                                   --   cb02.aide
		else                                                               --   
			t_mid:add(f_ext, buf(5, 4)):append_text(" [2.0a/standard]")    --   cb02.ext = standard
--			t_mid:add(f_mx2, buf(5, 4))                                    --   cb02_mx2
			t_mid:add(f_aids, buf(5, 4))                                   --   cb02.aids
		end                                                                --   

	t_fid = t_fr:add(f_fid , buf(9, 4)):append_text(" [SAM]")              -- cb02.fid

	n = buf(13, 1):uint()                                                  -- 
	t_dinf = t_fr:add(f_dinf, buf(13, 1)):append_text(" [rxxx'llll]")      -- cb02.dinf  
		t_dinf:add(f_rtr, buf(13, 1))                                      --   cb02.rtr
--		t_dinf:add(f_dxx, buf(13, 1))                                      --   cb02.dxx
		t_dinf:add(f_dlc, buf(13, 1))                                      --   cb02.dlc

	t_data = t_fr:add(f_data, buf(14, 8))                                  -- cb02.data
		local  dlc = bit.band(n, 0xF)                                      -- 
		t_dact = t_data:add(f_dact, buf(14,  dlc   ))                      --   cb02.dact
		t_dpad = t_data:add(f_dpad, buf(14 + dlc, 8 - dlc))                --   cb02.dpad
end

--+============================================================================ =======================================
-- cb02 dissector function
-- Call Frame dissector for each of the two CANBus Frames in the Ethernet Packet
--
function  p_cb02.dissector (buf, pkt, root)

	-- Fill in the protocol column in the packet window
	pkt.cols.protocol = p_cb02.name

	-- create subtree for cb02
	t_main  = root:add(p_cb02, buf(0)):append_text(", A pair of CANbus frames")

		-- 1 Frame is 22 bytes long
		t_fr1 = t_main:add(f_fr1, buf(0*22, 22))
		frame(buf(0*22), pkt, t_fr1);

		t_fr2 = t_main:add(f_fr2, buf(1*22, 22))
		frame(buf(1*22), pkt, t_fr2);

		-- Ethernet Packets must be >=64 bytes [including the (hidden) 4 byte CRC]
		--   64 - sizeof(SMAC) - sizeof(DMAC) - sizeof(TYPE) - (2 * 22) - sizeof(CRC)
		--   64 -     6        -     6        -     2        -    44    -     4       = 2 bytes of Eth Padding
		t_epad = t_main:add(f_epad, buf(2*22, 2)):append_text(" [Ethernet Frame Padding]")

end

--+============================================================================ =======================================
-- Initialization routine
--
function  p_cb02.init ()
end

------------------------------------------------------------------------------- ---------------------------------------
-- subscribe for Ethernet packets of type CB02
--
local eth_table = DissectorTable.get("ethertype")
eth_table:add(0xCB02, p_cb02)
