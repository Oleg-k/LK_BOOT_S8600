/*
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define LDO08_CNTRL            0x011
#define LDO12_CNTRL            0x015
#define LDO15_CNTRL            0x089
#define LDO17_CNTRL            0x08B
#define LDO20_CNTRL            0x11F	/* PM8058 only */
#define LDO_LOCAL_EN_BMSK      0x80

#define SPI_SCLK    45
#define SPI_CS      46
#define SPI_MOSI    47
#define SPI_MISO    48
#define LCD_RESET  129


#define SLEEPMSEC		0x10000000
#define ENDDEF			0x20000000
#define	DEFMASK			0xF0000000


const unsigned TL2796_SEQ_DISPLAY_ON[] = {
	0x029,
	ENDDEF, 0
};

const unsigned TL2796_SEQ_DISPLAY_OFF[] = {
	0x028,
	ENDDEF, 0
};

const unsigned TL2796_SEQ_STANDBY_ON[] = {
	0x010,	/* Stand-by On Command */
	SLEEPMSEC + 120,
	ENDDEF, 0
};

const unsigned TL2796_SEQ_STANDBY_OFF[] = {
	0x011,	/* Stand-by Off Command */
	SLEEPMSEC + 120,
	ENDDEF, 0
};


const unsigned TL2796_SEQ_DISPLAY_SETTING[] = {
	SLEEPMSEC + 21,
	0x0F8,	/* Panel Condition Set Command*/
	0x101,	/* DOCT */
	0x127,	/* CLWEA */
	0x127,	/* CLWEB*/
	0x107,	/* CLTE */
	0x107,	/* SHE */
	0x154,	/* FLTE */
	0x19F,	/* FLWE */
	0x163,	/* SCTE */
	0x186,	/* SCWE */
	0x11A,	/* INTE */
	0x133,	/* INWE */
	0x10D,	/* EMPS */
	0x100,	/* E_INTE */
	0x100,	/* E_INWE */

	0x0F2,	/* Display Condition Set Command*/
	0x102,	/* Number of Line */
	0x103,	/* VBP */
	0x11C,	/* VFP */
	0x110,	/* HBP */
	0x110,	/* HFP */

	0x0F7,	/* Command */
	0x103,	/* GTCON */
	0x100,	/* Display Mode */  
	0x100,	/* Vsync/Hsync, DOCCLK, RGB mode */
	ENDDEF, 0
};


const unsigned TL2796_SEQ_ETC_SETTING[] = {
	/* ETC Condition Set Command  */
	0x0F6,
	0x100,	0x18E,
	0x107,

	0x0B3,
	0x16C,

	0x0B5,
	0x12C,	0x112,
	0x10C,	0x10A,
	0x110,	0x10E,
	0x117,	0x113,
	0x11F,	0x11A,
	0x12A,	0x124,
	0x11F,	0x11B,
	0x11A,	0x117,
	0x12B,	0x126,
	0x122,	0x120,
	0x13A,	0x134,
	0x130,	0x12C,
	0x129,	0x126,
	0x125,	0x123,
	0x121,	0x120,
	0x11E,	0x11E,

	0x0B6,
	0x100,	0x100,
	0x111,	0x122,
	0x133,	0x144,
	0x144,	0x144,
	0x155,	0x155,
	0x166,	0x166,
	0x166,	0x166,
	0x166,	0x166,

	0x0B7,
	0x12C,	0x112,
	0x10C,	0x10A,
	0x110,	0x10E,
	0x117,	0x113,
	0x11F,	0x11A,
	0x12A,	0x124,
	0x11F,	0x11B,
	0x11A,	0x117,
	0x12B,	0x126,
	0x122,	0x120,
	0x13A,	0x134,
	0x130,	0x12C,
	0x129,	0x126,
	0x125,	0x123,
	0x121,	0x120,
	0x11E,	0x11E,

	0x0B8,
	0x100,	0x100,
	0x111,	0x122,
	0x133,	0x144,
	0x144,	0x144,
	0x155,	0x155,
	0x166,	0x166,
	0x166,	0x166,
	0x166,	0x166,

	0x0B9,
	0x12C,	0x112,
	0x10C,	0x10A,
	0x110,	0x10E,
	0x117,	0x113,
	0x11F,	0x11A,
	0x12A,	0x124,
	0x11F,	0x11B,
	0x11A,	0x117,
	0x12B,	0x126,
	0x122,	0x120,
	0x13A,	0x134,
	0x130,	0x12C,
	0x129,	0x126,
	0x125,	0x123,
	0x121,	0x120,
	0x11E,	0x11E,

	0x0BA,
	0x100,	0x100,
	0x111,	0x122,
	0x133,	0x144,
	0x144,	0x144,
	0x155,	0x155,
	0x166,	0x166,
	0x166,	0x166,
	0x166,	0x166,

	0x0FA,		//Default gammaset - 22gamma_260cd
        0x102,	0x118,
	0x108,	0x124,
	0x170,	0x16E,
	0x14E,	0x1BC,
	0x1C0,	0x1AF,
	0x1B3,	0x1B8,
	0x1A5,	0x1C5,
	0x1C7,	0x1BB, 
	0x100,	0x1B9,
	0x100,	0x1B8,
	0x100,	0x1FC,

	0xFA,	0x101,

	0x011,
	SLEEPMSEC + 120,
	0x029,
	ENDDEF, 0
};

