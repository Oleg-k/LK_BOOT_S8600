/*
 * Copyright (c) 2007, Google Inc.
 * All rights reserved.
 *
 * Copyright (c) 2009-2011, Code Aurora Forum. All rights reserved.
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
 *  * Neither the name of Code Aurora nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
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

#include <debug.h>
#include <dev/gpio.h>
#include <kernel/thread.h>
#include <sys/types.h>
#include <platform/gpio_hw.h>
#include <dev/pm8058.h>
#include "panel.h"
#include <platform/machtype.h>




static struct msm_gpio lcd_panel_gpios[]= {
	{GPIO_CFG(SPI_SCLK, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "spi_clk"},
	{GPIO_CFG(SPI_CS, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "spi_cs0"},
	{GPIO_CFG(SPI_MOSI, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "spi_mosi"},
//	{GPIO_CFG(48, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA), "spi_miso"},
	{GPIO_CFG(LCD_RESET, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcd_reset"},

	{GPIO_CFG(90, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_pclk"},
	{GPIO_CFG(91, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_en"},
	{GPIO_CFG(92, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_vsync"},
	{GPIO_CFG(93, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_hsync"},


	{GPIO_CFG(20, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_blu0"},
	{GPIO_CFG(21, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_blu1"},
	{GPIO_CFG(22, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_blu2"},
	{GPIO_CFG(100, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_blu3"},
	{GPIO_CFG(101, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_blu4"},
	{GPIO_CFG(102, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_blu5"},
	{GPIO_CFG(103, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_blu6"},
	{GPIO_CFG(104, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_blu7"},

	{GPIO_CFG(18, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_grn0"},
	{GPIO_CFG(19, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_grn1"},
	{GPIO_CFG(94, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_grn2"},
	{GPIO_CFG(95, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_grn3"},
	{GPIO_CFG(96, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_grn4"},
	{GPIO_CFG(97, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_grn5"},
	{GPIO_CFG(98, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_grn6"},
	{GPIO_CFG(99, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_grn7"},


	{GPIO_CFG(23, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_red0"},
	{GPIO_CFG(24, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_red1"},
	{GPIO_CFG(25, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_red2"},
	{GPIO_CFG(105, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_red3"},
	{GPIO_CFG(106, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_red4"},
	{GPIO_CFG(107, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_red5"},
	{GPIO_CFG(108, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_red6"},
	{GPIO_CFG(109, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), "lcdc_red7"},
};



static void tl2796_spi_write(unsigned short data)
{
	signed i;


        gpio_set(SPI_SCLK, 1);
	gpio_set(SPI_CS, 0);	/* cs* low */

	udelay(10);

	for (i = 8 ; i >= 0; i--)  //9 bit is cmd/data, 0 - cmd, 1- data
	{
                gpio_set(SPI_SCLK, 0);
                udelay(10);
		if ((data >> i) & 0x1)
			gpio_set(SPI_MOSI, 1);
		else
			gpio_set(SPI_MOSI, 0);

		udelay(20);
		gpio_set(SPI_SCLK, 1);
		udelay(10);
	}
	gpio_set(SPI_CS, 1);                 
	gpio_set(SPI_MOSI, 1);   //idle state of MOSI is 0

}




static void tl2796_panel_send_sequence(const unsigned *wbuf)
{
	unsigned i;
	i=0;
	while ((wbuf[i] & DEFMASK) != ENDDEF)
	{
		if ((wbuf[i] & DEFMASK) != SLEEPMSEC) 
		{
			tl2796_spi_write((unsigned short)wbuf[i]);
                } 
		else 
		{
			mdelay((unsigned short)wbuf[i]);
		}
		i++;
	}
}




void panel_poweron(void)
{
	unsigned int vreg_ldo15, vreg_ldo17;

//	mdp_clock_init(122880000);	
//        lcdc_clock_init(25423000);

	//configure gpios for lcd
	platform_gpios_enable(lcd_panel_gpios, ARRAY_SIZE(lcd_panel_gpios));



        /* Set the output so that we dont disturb the slave device */
        gpio_set(SPI_SCLK, 1);
        gpio_set(SPI_MOSI, 1);

        /* Set the Chip Select De-asserted */
        gpio_set(SPI_CS, 1);


        gpio_set(LCD_RESET, 1); /* bring reset line high */

//        mdelay(5);

	// Set LD017 to 1.8V
	pmic_write(LDO17_CNTRL, 0x06 | LDO_LOCAL_EN_BMSK);
	vreg_set_level(36, 1800);	

//        mdelay(5);

	// Set LD015 to 3.0V
	pmic_write(LDO15_CNTRL, 0x1E | LDO_LOCAL_EN_BMSK);
	vreg_set_level(23, 3000);

	vreg_enable(36);
	vreg_enable(23);

	mdelay(10);

	gpio_set(LCD_RESET, 0);	/* bring reset line low to hold reset */
	mdelay(25);
	gpio_set(LCD_RESET, 1);	/* bring reset line high */
	mdelay(40);		/* 10 msec before IO can be accessed */


        tl2796_panel_send_sequence(TL2796_SEQ_DISPLAY_SETTING);
        tl2796_panel_send_sequence(TL2796_SEQ_ETC_SETTING);
        dprintf(INFO, "Panel is power on\n");
}



