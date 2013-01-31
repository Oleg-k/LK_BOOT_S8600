/*
 * Copyright (c) 2008, Google Inc.
 * All rights reserved.
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
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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
#include <sys/types.h>
#include <reg.h>
#include <dev/fbcon.h>
#include <kernel/thread.h>
#include <platform/debug.h>
#include <platform/iomap.h>
#include <platform/timer.h>
#include <dev/gpio.h>
#include <mddi_hw.h>
#include <msm_i2c.h>
#include <max14577_i2c.h>
#include <platform/gpio_hw.h>


void platform_init_interrupts(void);
void platform_init_timer();

void uart2_clock_init(void);
void uart_init(void);


struct fbcon_config *lcdc_init(void);
static uint32_t ticks_per_sec = 0;

#define ARRAY_SIZE(a) (sizeof(a)/(sizeof((a)[0])))

#define FUEL_I2C_CLK	168
#define FUEL_I2C_DAT	169


static struct msm_gpio uart2_config_data[] = {
//	{ GPIO_CFG(49, 2, GPIO_OUTPUT,  GPIO_PULL_DOWN, GPIO_2MA), "UART2_RFR"},
//	{ GPIO_CFG(50, 2, GPIO_INPUT,   GPIO_PULL_DOWN, GPIO_2MA), "UART2_CTS"},
	{ GPIO_CFG(51, 2, GPIO_INPUT,   GPIO_PULL_DOWN, GPIO_2MA), "UART2_Rx"},
	{ GPIO_CFG(52, 2, GPIO_OUTPUT,  GPIO_PULL_DOWN, GPIO_2MA), "UART2_Tx"},
};

static struct msm_gpio emmc_config_data[] = {
	{GPIO_CFG(64, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), "sdc2_clk"},
	{GPIO_CFG(65, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), "sdc2_cmd"},

	{GPIO_CFG(66, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), "sdc2_dat_3"},
	{GPIO_CFG(67, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), "sdc2_dat_2"},
	{GPIO_CFG(68, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), "sdc2_dat_1"},
	{GPIO_CFG(69, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), "sdc2_dat_0"},

	{GPIO_CFG(115, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), "sdc2_dat_4"},
	{GPIO_CFG(114, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), "sdc2_dat_5"},
	{GPIO_CFG(113, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), "sdc2_dat_6"},
	{GPIO_CFG(112, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), "sdc2_dat_7"},
};


static struct msm_gpio sdc4_config_data[] = {
    {GPIO_CFG(58, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), "sdc4_clk"},
    {GPIO_CFG(59, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), "sdc4_cmd"},
    {GPIO_CFG(60, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), "sdc4_dat_3"},
    {GPIO_CFG(61, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), "sdc4_dat_2"},
    {GPIO_CFG(62, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), "sdc4_dat_1"},
    {GPIO_CFG(63, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), "sdc4_dat_0"},

    {GPIO_CFG(116, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_8MA), "sdc4_det"},
};




/* CRCI - mmc slot mapping.
 * mmc slot numbering start from 1.
 * entry at index 0 is just dummy.
 */
uint8_t sdc_crci_map[5] = { 0, 6, 7, 12, 13 };


void platform_early_init(void)
{

#if WITH_DEBUG_UART
        platform_gpios_enable(uart2_config_data, ARRAY_SIZE(uart2_config_data));
	uart2_clock_init();
	uart_init();
#endif
        platform_gpios_enable(emmc_config_data, ARRAY_SIZE(emmc_config_data));
        platform_gpios_enable(sdc4_config_data, ARRAY_SIZE(sdc4_config_data));
	platform_init_interrupts();
	platform_init_timer();

	max14577_i2c_init();
}

void platform_init(void)
{
	struct fbcon_config *fb_cfg;

	dprintf(INFO, "platform_init()\n");
	acpu_clock_init();
	adm_enable_clock();
	ce_clock_init();

//	msm_i2c_init();
}



void display_init(void)
{
	struct fbcon_config *fb_cfg;

	mdp_lcdc_clock_init();               //
	mdp_clock_init(192000000);
	lcdc_clock_init(25528000);

	fb_cfg = lcdc_init();
	ASSERT(fb_cfg);
	panel_poweron();       
	fbcon_setup(fb_cfg , 0); //non-inverted
}



void display_shutdown(void)
{
	/* Turning off LCDC */
	lcdc_shutdown();
}


void platform_uninit(void)
{
#if DISPLAY_SPLASH_SCREEN
	display_shutdown();
#endif

	platform_uninit_timer();
}


/* Initialize DGT timer */
void platform_init_timer(void)
{
	uint32_t val = 0;

	/* Disable timer */
	writel(0, DGT_ENABLE);

	/* Check for the hardware revision */
	val = readl(HW_REVISION_NUMBER);
	val = (val >> 28) & 0x0F;
	if (val >= 1)
		writel(1, DGT_CLK_CTL);

#if _EMMC_BOOT
	ticks_per_sec = 19200000;	/* Uses TCXO (19.2 MHz) */
#else
	ticks_per_sec = 6144000;	/* Uses LPXO/4 (24.576 MHz / 4) */
#endif
}

/* Returns platform specific ticks per sec */
uint32_t platform_tick_rate(void)
{
	return ticks_per_sec;
}

