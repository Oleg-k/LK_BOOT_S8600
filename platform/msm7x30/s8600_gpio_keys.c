/* gpio-keys.c - code for handling keys connected directly to gpios
 *
 * Copyright (C) 2011 Alexander Tarasikov <alexander.tarasikov@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <debug.h>
#include <kernel/thread.h>
#include <dev/gpio.h>
#include <dev/keys.h>
#include <platform/interrupts.h>
#include <platform/timer.h>
#include "gpio_hw.h"


#define GPIO_KEY_HOME	180

static struct gpio_key s8600_gpio_keys[] = {
	{GPIO_KEY_HOME, KEY_HOME, true, "Home"},
};

static struct gpio_keys_pdata s8600_gpio_keys_pdata = {
	.keys = s8600_gpio_keys,
	.nkeys = ARRAY_SIZE(s8600_gpio_keys),
};


static enum handler_return gpio_key_irq(void *arg)
{
	struct gpio_key *key = arg;
	int i,state,state2;

	mdelay(25);

	state = !!gpio_get(key->gpio);

	//some debouncing
	for (int i = 0; i < 5; i++) {
		mdelay(10);
		state2 = !!gpio_get(key->gpio);
		if (state2 != state) {
//			printf("%s: failed to reach stable state after %d ms\n",
//					__func__, i * 10);
			return INT_RESCHEDULE;
		}
	}
	keys_post_event(key->keycode, state ^ key->active_low);
	return INT_RESCHEDULE;
}

void gpio_keys_init(struct gpio_keys_pdata *pdata) 
{
	int i, gpio, cfg;
	dprintf(DEBUG, "+%s\n", __func__);
	if (!pdata) {
		dprintf(CRITICAL, "%s: pdata is NULL\n", __func__);
	}

	for (i = 0; i < pdata->nkeys; i++) 
	{
		gpio = pdata->keys[i].gpio;
		gpio_tlmm_config(GPIO_CFG(gpio, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA), 0);

		cfg = gpio_config(gpio, GPIO_INPUT);
		if (cfg < 0) {
			dprintf(CRITICAL, "%s: cannot set gpio %d as input\n", __func__, gpio);
			continue;
		}
		register_gpio_int_handler(gpio, gpio_key_irq, &pdata->keys[i]);
		unmask_gpio_interrupt(gpio, GPIO_IRQF_BOTH);
	}
	dprintf(DEBUG, "-%s\n", __func__);
}

