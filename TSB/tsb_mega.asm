; Modification of TinySafeBoot for RS-485
; (c) Karlis Goba 2016
; See below for licensing, copyright and credits
;***********************************************************************
;***********************************************************************
;***********************************************************************
; TinySafeBoot - The Universal Bootloader for AVR ATtinys and ATmegas
;***********************************************************************
;***********************************************************************
;***********************************************************************
;
;-----------------------------------------------------------------------
; Written in 2011-2015 by Julien Thomas
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 3
; of the License, or (at your option) any later version.
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty
; of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
; See the GNU General Public License for more details.
; You should have received a copy of the GNU General Public License
; along with this program; if not, see:
; http://www.gnu.org/licenses/
;-----------------------------------------------------------------------
;
;***********************************************************************
; FUSES
;***********************************************************************
;
; * BOOTRST activated lets the MCU jump into the Bootloader Section 
;     (rather than $0000) with every hardware Reset.
; * BODLEVEL should be set to avoid flash corruption during unsafe 
;     device power-up.
; * BOOTSZ=10 (512 words) or BOOTSZ=11 (256 words) to reserve 512 bytes 
;     for a Bootloader Section.
; * BLB to MODE 2 or 3 protects Bootloader Section from undesirable 
;     write access by firmware.
; * LOCKBITS may be set to Mode 3 (LB-Mode 3) in a security environment.
;
; ATMega328P: Low 0xFF High 0xDE Ext 0xFD
;
;***********************************************************************
; OVERVIEW
;***********************************************************************
;
; TSB assembly source is organized in 4 segments (approx. line numbers)
;
; ~   50 ... Global definitions
; ~  240 ... TSB Installer for ATtinys
; ~  470 ... TSB for ATtinys
; ~ 1180 ... TSB for ATmegas
;
;***********************************************************************
; ADJUSTMENTS FOR INDIVIDUAL ASSEMBLY
;***********************************************************************
;
; This Sourcecode is directly compatible to: AVRASM2, GAVRASM
;
.nolist
;
;-----------------------------------------------------------------------
; SPECIFY TARGET AVR
;-----------------------------------------------------------------------
;
.include "m328Pdef.inc"
;
;
.list
;
;-----------------------------------------------------------------------
; BUILD INFO
;-----------------------------------------------------------------------
; YY = Year - MM = Month - DD = Day
.set    YY      =       15
.set    MM      =       8
.set    DD      =       26
;
.set BUILDSTATE = $F0   ; version management option
;
;
;-----------------------------------------------------------------------
; PORTS
;-----------------------------------------------------------------------
;
; Important Note: B0/B1 are defaults for database templates
;
.equ    RXPORT  = PORTD
.equ    RXPIN   = PIND
.equ    RXDDR   = DDRD
.equ    RXBIT   = 0
.equ    TXPORT  = PORTD
.equ    TXDDR   = DDRD
.equ    TXBIT   = 1

.equ    TEPORT  = PORTD
.equ    TEDDR   = DDRD
.equ    TEBIT   = 2

.equ    REPORT  = PORTD
.equ    REDDR   = DDRD
.equ    REBIT   = 3


;-----------------------------------------------------------------------
; *** Changes below this line are on your own risk! ***
;-----------------------------------------------------------------------
;
;
;
;***********************************************************************
; AUTO-ADJUST FOR DIFFERENT ASSEMBLY OPTIONS
;***********************************************************************
;
; Autodetect ATtiny / ATmega and set TINYMEGA switch accordingly

.ifdef RWW_START_ADDR
        .if RWW_START_ADDR == RWW_STOP_ADDR
        .equ TINYMEGA = 0
        .message "DETECTED ATTINY DEFINITIONS"
        .else
        .equ TINYMEGA = 1
        .message "DETECTED ATMEGA DEFINITIONS"
        .endif
.else
        .equ TINYMEGA = 0
        .message "DETECTED ATTINY DEFINITIONS"
.endif

.if FLASHEND > ($7fff)
        .error "SORRY! DEVICES OVER 64 KB NOT SUPPORTED YET."
        .exit
.endif

;-----------------------------------------------------------------------
; Workarounds for devices with renamed or missing definitions
;-----------------------------------------------------------------------
;
.ifndef SPMCSR                  ; SPMEN / PGERS / ...
        .equ SPMCSR = SPMCR
.endif

.ifndef MCUSR                   ; PORF / EXTRF / BORF / WDRF
        .equ MCUSR = MCUCSR
.endif

; Detect Attiny441/841 to amend missing pagesize and apply 4-page mode

.set FOURPAGES = 0

.if ((SIGNATURE_000 == $1E) && (SIGNATURE_002 == $15) && (SIGNATURE_001 == $92))
                .equ PAGESIZE = 32
                .set FOURPAGES = 1
                .message "ATTINY441: 4-PAGE-ERASE MODE"
.endif

.if ((SIGNATURE_000 == $1E) && (SIGNATURE_002 == $15) && (SIGNATURE_001 == $93))
                .equ PAGESIZE = 32
                .set FOURPAGES = 1
                .message "ATTINY841: 4-PAGE-ERASE MODE"
.endif

;-----------------------------------------------------------------------
; Universal Constants and Registers
;-----------------------------------------------------------------------

.equ    REQUEST         = '?'           ; request / answer / go on
.equ    CONFIRM         = '!'           ; confirm / attention

; Current bootloader date coded into 16-bit number
.equ    BUILDDATE   = YY * 512 + MM * 32 + DD

; Other
.equ    INFOLEN         = 8              ; *Words* of Device Info
.equ    BUFFER          = SRAM_START

; Registers (in use by TSB-Firmware and TSB-Installer for ATtinys)
.def    avecl   = r4                    ; application vector temp low
.def    avech   = r5                    ; application vector temp high
.def    bclkl   = r6                    ; baudclock low byte
.def    bclkh   = r7                    ; baudclock high byte
.def    tmp1    = r16                   ; these are
.def    tmp2    = r17                   ; universal
.def    tmp3    = r18                   ; temporary
.def    tmp4    = r19                   ; registers
.def    bcnt    = r20                   ; page bytecounter

;
;
;***********************************************************************
;***********************************************************************
;***********************************************************************
; START OF TSB FOR ATTINYS
;***********************************************************************
;***********************************************************************
;***********************************************************************
;
; TSB for ATmegas is always coded directly to target address.

.if TINYMEGA == 1

.message "ASSEMBLY OF TSB FOR ATMEGA"

.equ    BOOTSTART       = (FLASHEND+1)-512      ; = 1024 Bytes
;.equ    BOOTSTART       = (0x3FFF+1)-512      ; = 1024 Bytes
.equ    LASTPAGE        = BOOTSTART - PAGESIZE  ; = 1 page below TSB!

.org    BOOTSTART

RESET:
        cli

        in tmp4, MCUSR                  ; check reset condition
        sbrc tmp4, WDRF                 ; in case of a Watchdog reset
        rjmp APPJUMP                    ; immediately leave TSB

        ldi tmp1, low (RAMEND)          ; write ramend low
        out SPL, tmp1                   ; into SPL (stackpointer low)
.ifdef SPH
        ldi tmp1, high(RAMEND)          ; write ramend high for ATtinys
        out SPH, tmp1                   ; with SRAM > 256 bytes
.endif

;-----------------------------------------------------------------------
; ACTIVATION CHECK
;-----------------------------------------------------------------------

WaitRX:
        sbi TXDDR, TXBIT                ; if RX=TX (One-Wire), port is
        ;sbi RXDDR, RXBIT                ; driven open collector style,   [KG removed]
        sbi RXPORT, RXBIT               ; else RX is input with pullup
        sbi TXPORT, TXBIT               ; and TX preset logical High
        sbi REDDR, REBIT                ; configure receive enable as output [KG]
        sbi TEDDR, TEBIT                ; configure transmit enable as output [KG]

; coldstart (power-up) - wait for RX to stabilize
WRX0To:
        sbic RXPIN, RXBIT               ; 1st stage - loop while RX = 0
        rjmp WRXSTo                     ; if RX = 1, start COM Timeout
        sbiw xl, 1                      ; use X for fast countdown
        brcc WRX0To                     ; if timed out with 0 level
        rjmp APPJUMP                    ; goto APPJUMP in LASTPAGE
WRXSTo:                                 ; else
        rcall ZtoLASTPAGE               ; set Z to start'o'LASTPAGE
        adiw zl, 2                      ; skip first 2 bytes
        lpm xh, z+                      ; load TIMEOUT byte
        ldi xl, 128

WRX1To:
        dec tmp1                        ; inner counter to delay
        brne WRX1To                     ; for debouncing/denoising
        sbis RXPIN, RXBIT               ; if serial startbit occurs
        rjmp Activate                   ; go Activate
        sbiw xl, 1                      ; use X for down counter
        brcc WRX1To                     ; to Timeout
        ;rjmp APPJUMP                   ; Timeout! Goto APPJUMP

;-----------------------------------------------------------------------
; ATMEGA APPJUMP = SIMPLE JUMP TO $0000 (ORIGINAL RESET VECTOR)
;-----------------------------------------------------------------------
; Boot Reset Vector (BOOTRST) must be activated for TSB on ATmegas.
; After timeout or executing commands, TSB for ATmegas will simply
; handover to the App by a (relative or absolute) jump to $0000.

APPJUMP:
        rcall SPMwait                   ; make sure everything's done
        clr tmp1
        out RXPORT, tmp1                ; reset PORT and DDR to zero
.if TXPORT != RXPORT
        out TXPORT, tmp1
.endif
        out TXDDR, tmp1
.if REDDR != TXDDR
        out REDDR, tmp1
.endif
.if TEDDR != TXDDR
        out TEDDR, tmp1
.endif

.if FLASHEND >= ($1fff)
        jmp  $0000                      ; absolute jump
.else
        rjmp $0000                      ; relative jump
.endif

;-----------------------------------------------------------------------
; BAUDRATE CALIBRATION CYCLE
;-----------------------------------------------------------------------

Activate:
        clr xl                          ; clear temporary
        clr xh                          ; baudrate counter
        ldi tmp1, 6                     ; number of expected bit-changes
actw1:
        sbic RXPIN, RXBIT               ; idle 1-states (stopbits, ones)
        rjmp actw1
actw2:
        adiw xl, 1                      ; precision measuring loop
        sbis RXPIN, RXBIT               ; count clock cycles
        rjmp actw2                      ; while RX is 0-state
        dec tmp1
        brne actw1
actwx:
        movw bclkl, xl                  ; save result in bclk

;-----------------------------------------------------------------------
; CHECK PASSWORD / EMERGENCY ERASE
;-----------------------------------------------------------------------

CheckPassword:
chpw0:  ser tmp4                        ; tmp4 = 255 enables comparison
chpw1:  lpm tmp3, z+                    ; load pw character from Z
        and tmp3, tmp4                  ; tmp3 = 0 disables comparison
        cpi tmp3, 255                   ; byte value 255 indicates
        breq chpwx                      ; end of password -> success
chpw2:  rcall Receivebyte               ; else receive next character
        cpi tmp1, 0                     ; rxbyte = 0 will branch
        breq chpwee                     ; to confirm emergency erase
        cp  tmp1, tmp3                  ; compare password with rxbyte
        breq chpw0                      ; if equal check next character
        clr  tmp4                       ; tmp4 = 0 to loop forever
        rjmp chpw1                      ; all to smoothen power profile
chpwee:
        rcall RequestConfirm            ; request confirmation
        brts chpa                       ; not confirmed, leave
        rcall RequestConfirm            ; request 2nd confirmation
        brts chpa                       ; can't be mistake now
        rcall EmergencyErase            ; go, emergency erase!
        rjmp  Mainloop
chpa:
        rjmp APPJUMP                    ; start application
chpwx:
;       rjmp SendDeviceInfo             ; go on to SendDeviceInfo

;-----------------------------------------------------------------------
; SEND DEVICEINFO
;-----------------------------------------------------------------------

SendDeviceInfo:        
        ldi zl, low (DEVICEINFO*2)      ; load address of deviceinfo
        ldi zh, high(DEVICEINFO*2)      ; low and highbyte
        ldi bcnt, INFOLEN*2
        rcall SendFromFlash

;-----------------------------------------------------------------------
; MAIN LOOP TO RECEIVE AND EXECUTE COMMANDS
;-----------------------------------------------------------------------

Mainloop:
        clr zl                          ; clear Z pointer
        clr zh                          ; which is frequently used
        rcall SendConfirm               ; send CONFIRM via RS232
        rcall Receivebyte               ; receive command via RS232
        rcall CheckCommands             ; check command letter
        rjmp Mainloop                   ; and loop on

;-----------------------------------------------------------------------
; CHANGE USER DATA IN LASTPAGE
;-----------------------------------------------------------------------

ChangeSettings:
        rcall GetNewPage                ; get new LASTPAGE contents
        brtc ChangeS0                   ; from Host (if confirmed)
        ret
ChangeS0:
        rcall ZtoLASTPAGE               ; re-write LASTPAGE
        rcall EraseFlashPage
        rcall WritePage                 ; erase and write LASTPAGE

;-----------------------------------------------------------------------
; SEND USER DATA FROM LASTPAGE
;-----------------------------------------------------------------------

ControlSettings:
        rcall ZtoLASTPAGE               ; point to LASTPAGE
;       rcall SendPageFromFlash

;-----------------------------------------------------------------------
; SEND DATA FROM FLASH MEMORY
;-----------------------------------------------------------------------

SendPageFromFlash:
        ldi bcnt, low (PAGESIZE*2)      ; whole Page to send
SendFromFlash:
        rcall SPMwait                   ; (re)enable RWW read access
        lpm tmp1, z+                    ; read directly from flash
        rcall Transmitbyte              ; and send out to RS232
        dec bcnt                        ; bcnt is number of bytes
        brne SendFromFlash
        ret

;-----------------------------------------------------------------------
; READ APPLICATION FLASH
;-----------------------------------------------------------------------
; read and transmit application flash area (pagewise)

ReadAppFlash:
RAF0:
        rcall RwaitConfirm
        brts RAFx
        rcall SendPageFromFlash
RAF1:
        cpi zl, low (LASTPAGE*2)        ; count up to last byte
        brne RAF0                       ; below LASTPAGE
        cpi zh, high(LASTPAGE*2)
        brne RAF0
RAFx:
        ret

;-----------------------------------------------------------------------
; WRITE APPLICATION FLASH
;-----------------------------------------------------------------------
; Write Appflash pagewise, don't modify anything for ATmegas

WriteAppFlash:
        rcall EraseAppFlash             ; Erase whole app flash
Flash2:
        rcall GetNewPage                ; get next page from host
        brts FlashX                     ; stop on user's behalf
Flash3:
        rcall WritePage                 ; write page data into flash
Flash4:
        cpi zh, high(LASTPAGE*2-1)      ; end of available Appflash?
        brne Flash2                     ; if Z reached last location
        cpi zl, low (LASTPAGE*2-1)      ; then we are finished
        brne Flash2                     ; else go on
FlashX:
        ret                             ; we're already finished!

;-----------------------------------------------------------------------
; WRITE FLASH PAGE FROM BUFFER, VERIFYING AND VERIFY-ERROR-HANDLING
;-----------------------------------------------------------------------

WritePage:
        rcall YtoBUFFER                 ; Y=BUFFER, bcnt=PAGESIZE*2
        rcall SPMwait
WrPa1:
        ld r0, y+                       ; fill R0/R1 with word
        ld r1, y+                       ; from buffer position Y / Y+1
        ldi tmp1, 0b00000001            ; set only SPMEN in SPMCSR
        out SPMCSR, tmp1                ; to activate page buffering
        spm                             ; store word in page buffer
        adiw zl, 2                      ; and forward to next word
        subi bcnt, 2
        brne WrPa1
        ; Z = start of next page now
        subi zl, low (PAGESIZE*2)       ; point back Z to
        sbci zh, high(PAGESIZE*2)       ; start of current page
        ; Z = back on current page's start
WrPa2:
        ldi tmp1, 0b00000101            ; enable PRWRT + SPMEN
        out SPMCSR, tmp1                ; in SPMCSR
        spm                             ; write whole page to flash
WrPa3:
        in tmp1, SPMCSR                 ; wait for flash write finished
        sbrc tmp1, 0                    ; skip if SPMEN (bit0) cleared
        rjmp WrPa3                      ; ITS BEEN WRITTEN
        subi zl, low (65536-PAGESIZE*2)      ; same effect as
        sbci zh, high(65536-PAGESIZE*2)      ; Z = Z + PAGESIZE*2
        rjmp SPMwait
        ;ret

;-----------------------------------------------------------------------
; CHECK COMMANDS
;-----------------------------------------------------------------------

CheckCommands:
        cpi tmp1, 'c'                   ; read LASTPAGE
        breq ControlSettings
        cpi tmp1, 'C'                   ; write LASTPAGE
        breq ChangeSettings
        cpi tmp1, 'f'                   ; read Appflash
        breq ReadAppFlash
        cpi tmp1, 'F'                   ; write Appflash
        breq WriteAppFlash
        cpi tmp1, 'e'                   ; read EEPROM
        breq EepromRead
        cpi tmp1, 'E'                   ; write EEPROM
        breq EEpromWrite
        rjmp APPJUMP                    ; else start application

;-----------------------------------------------------------------------
; EEPROM READ/WRITE ACCESS
;-----------------------------------------------------------------------

EepromWrite:
EEWr0:
        rcall GetNewPage                ; get EEPROM datablock
        brts EERWFx                     ; or abort on host's demand
EEWr1:
        rcall YtoBUFFER                 ; Y = Buffer and Bcnt = blocksize
EEWr2:
        ld tmp1, y+                     ; read EEPROM byte from buffer
        rcall EEWriteByte
        dec bcnt                        ; count down block byte counter
        brne EEWr2                      ; loop on if block not finished
        rjmp EeWr0

;-----------------------------------------------------------------------

EEpromRead:
EeRe1:
        rcall RwaitConfirm              ; wait to confirm
        brts EERWFx                     ; else we are finished
        ldi bcnt, low(PAGESIZE*2)       ; again PAGESIZE*2 is blocksize
EERe2:
        out EEARL, zl                   ; current EEPROM address low
        .ifdef  EEARH
        out EEARH, zh                   ; current EEPROM address high
        .endif
        sbi EECR, 0                     ; set EERE - EEPROM read enable
        in tmp1, EEDR                   ; read byte from current address
        rcall Transmitbyte              ; send out to RS232
        adiw zl,1                       ; count up EEPROM address
        dec bcnt                        ; count down block byte counter
        brne EERe2                      ; loop on if block not finished
        rjmp EERe1
EERWFx:
        ret

;-----------------------------------------------------------------------

EEWriteByte:
        out EEDR, tmp1                  ; write to EEPROM data register
        out EEARL, zl                   ; current EEPROM address low
        .ifdef  EEARH
        out EEARH, zh                   ; high EEARH for some attinys
        .endif
        sbi EECR, 2                     ; EEPROM master prog enable
        sbi EECR, 1                     ; EEPE initiate prog cycle
EeWB:
        sbic EECR, 1                    ; wait write cycle to complete
        rjmp EeWB                       ; before we can go on
        adiw zl,1                       ; count up EEPROM address
        ret

;-----------------------------------------------------------------------
; GET NEW PAGE
;-----------------------------------------------------------------------

GetNewPage:
        rcall RequestConfirm            ; check for Confirm
        brts GNPx                       ; abort if not confirmed
GNP0:
        rcall YtoBUFFER                 ; Y = BUFFER, bcnt = PAGESIZE*2
GNP1:
        rcall ReceiveByte               ; receive serial byte
        st y+, tmp1                     ; and store in buffer
        dec bcnt                        ; until full page loaded
        brne GNP1                       ; loop on
GNPx:
        ret                             ; finished
;-----------------------------------------------------------------------
; REQUEST TO CONFIRM / AWAIT CONFIRM COMMAND
;-----------------------------------------------------------------------

RequestConfirm:
        ldi tmp1, REQUEST               ; send request character
        rcall Transmitbyte              ; prompt to confirm (or not)

RwaitConfirm:
        rcall ReceiveByte               ; get host's reply
        clt                             ; set T=0 for confirmation
        cpi tmp1, CONFIRM               ; if host HAS sent CONFIRM
        breq RCx                        ; return with the T=0
        set                             ; else set T=1 (NOT CONFIRMED)
RCx:
        ret                             ; whether confirmed or not

;-----------------------------------------------------------------------
; FLASH ERASE TOP-TO-BOTTOM ( (BOOTSTART-1) ... $0000)
;-----------------------------------------------------------------------

EraseAppFlash:
        rcall ZtoLASTPAGE               ; point Z to LASTPAGE, directly
EAF0:
        subi zl, low (PAGESIZE*2)
        sbci zh, high(PAGESIZE*2)
        rcall EraseFlashPage
        brne EAF0                       ; until first page reached
EAFx:   ret                             ; and leave with Z = $0000

;-----------------------------------------------------------------------
; EMERGENCY ERASE OF FLASH / EEPROM / USERDATA
;-----------------------------------------------------------------------

EmergencyErase:
        rcall EraseAppFlash             ; erase Application Flash
        ser tmp1                        ; byte value for EEPROM writes
EEE0:
        rcall EEWriteByte               ; write EEPROM byte, Z = Z + 1
        cpi zh, high(EEPROMEND+1)+2     ; EEPROMEND
        brne EEE0                       ; and loop on until finished

        rcall ZtoLASTPAGE               ; LASTPAGE is to be erased
;        rcall EraseFlashPage

;-----------------------------------------------------------------------
; ERASE ONE FLASH PAGE
;-----------------------------------------------------------------------

EraseFlashPage:
        ldi tmp1, 0b00000011            ; enable PGERS + SPMEN
        out SPMCSR, tmp1                ; in SPMCSR and erase current
        spm                             ; page by SPM (MCU halted)

; Waiting for SPM to be finished is *obligatory* on ATmegas!
SPMwait:
        in tmp1, SPMCSR
        sbrc tmp1, 0                    ; wait previous SPMEN
        rjmp SPMwait
        ldi tmp1, 0b00010001            ; set RWWSRE and SPMEN
        out SPMCSR, tmp1
        spm
        ret

;-----------------------------------------------------------------------
; OTHER SUBROUTINES
;-----------------------------------------------------------------------

YtoBUFFER:
        ldi yl, low (BUFFER)            ; reset pointer
        ldi yh, high(BUFFER)            ; to programming buffer
        ldi bcnt, low(PAGESIZE*2)       ; and often needed
        ret

;-----------------------------------------------------------------------

ZtoLASTPAGE:
        ldi zl, low (LASTPAGE*2)        ; reset Z to LASTPAGE start
        ldi zh, high(LASTPAGE*2)
        ret

;-----------------------------------------------------------------------
; RS232 RECEIVE BYTE
;-----------------------------------------------------------------------

ReceiveByte:
        ;sbi RXPORT, RXBIT               ; again set pullup for RX [KG removed]
Recb1:
        sbic RXPIN, RXBIT               ; wait for startbit (0)
        rjmp Recb1                      ; loop while stop state (1)
Recb2:
        ldi tmp2, 8                     ; bitcounter
        rcall Waithalfbitcell           ; tune to center of startbit
Recb3:
        rcall Waitbitcell               ; tune to center of bitcell
        lsr tmp1                        ; right shift 0 into bit 7
        sbic RXPIN, RXBIT               ; if RXD bit is 1
        sbr tmp1, 0b10000000            ; set bit 7
Recb4:
        dec tmp2                        ; count down bitcounter
        brne Recb3                      ; loop until 8 bits collected
        rjmp Waitbitcell                ; wait into center of stopbit

;-----------------------------------------------------------------------
; RS232 SEND CONFIRM CHARACTER
;-----------------------------------------------------------------------

SendConfirm:
        ldi tmp1, CONFIRM
        ;rjmp Transmitbyte

;-----------------------------------------------------------------------
; RS232 TRANSMIT BYTE
;-----------------------------------------------------------------------

TransmitByte:
        sbi REPORT, REBIT               ; disable receive [KG]
        sbi TEPORT, TEBIT               ; enable transmit [KG]
        rcall Waitbitcell               ; ensure safe RX-TX transition
        rcall Trx0                      ; transmit 0 = startbit
        ldi tmp2, 8                     ; set bitcounter
Trxbit:                                 ; transmit byte loop
        sbrc tmp1, 0
        rcall Trx1                      ; sent logical 1 bitcell
        sbrs tmp1, 0                    ; or
        rcall Trx0                      ; sent logical 0 bitcell
        lsr tmp1                        ; shift out that bit
        dec tmp2                        ; count down
        brne Trxbit                     ; loop until all bits sent
        rcall Trx1
        cbi TEPORT, TEBIT               ; disable transmit [KG]
        cbi REPORT, REBIT               ; enable receive [KG]
        ret
Trx1:
        ;sbi TXDDR, TXBIT                ; if RX=TX (One-Wire), result is [KG removed]
        ;cbi RXDDR, RXBIT                ; pullup to Vcc for "1" (high-Z) [KG removed]
        sbi TXPORT, TXBIT               ; else portbit actively driven
        rjmp Waitbitcell
Trx0:
        ;sbi TXDDR, TXBIT                ; set TX driver for output [KG removed]
        cbi TXPORT, TXBIT               ; set portbit to active "0"
;       rjmp Waitbitcell                ; continue with Waitbitcell

;-----------------------------------------------------------------------
; RS232 PRECISION TIMING
;-----------------------------------------------------------------------

Waitbitcell:
        movw xl, bclkl                  ; load bitcell clock timer
wbc1:
        sbiw xl, 24                     ; same number of clocks
        nop                             ; as in calibration loop
        brcc wbc1
wbcx:   
        ret

Waithalfbitcell:
        movw xl, bclkl                  ; load bitcell clock timer
        lsr xh                          ; shiftout bit 0 of xh to carry
        ror xl                          ; carry shifted in bit 7 of xl
        rjmp wbc1                       ; run timer with 1/2 divider

DebugTest:
        ldi tmp1, REQUEST               ; send request character
        rcall Transmitbyte              ; prompt to confirm (or not)
        rjmp  DebugTest


;-----------------------------------------------------------------------
; DEVICE INFO BLOCK = PERMANENT DATA
;-----------------------------------------------------------------------

DEVICEINFO:
.message "DEVICE INFO BLOCK FOR ATMEGA"
.db "TSB", low (BUILDDATE), high (BUILDDATE), BUILDSTATE
.db SIGNATURE_000, SIGNATURE_001, SIGNATURE_002, low (PAGESIZE)
.dw BOOTSTART-PAGESIZE
.dw EEPROMEND
.db $AA, $AA

.message "ASSEMBLY OF TSB FOR ATMEGA SUCCESSFULLY FINISHED!"

.endif               ; closing TSB for ATmega sourcecode;

;***********************************************************************
; END OF TSB FOR ATMEGAS
;***********************************************************************

.exit

;***********************************************************************
;***********************************************************************
;***********************************************************************
; END OF CONDITIONAL ASSEMBLY SOURCE OF TSB FOR ATTINYS AND ATMEGAS
;***********************************************************************
;***********************************************************************
;***********************************************************************


