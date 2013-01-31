/*
 * Copyright (c) 2012, Shantanu Gupta <shans95g@gmail.com>
 *
 * Based on google's gpio keypad driver, Adds support for
 * keys which are directly mapped to gpio pin.
 *
 * TODO: 
 *
 * BUGS:
 *		If board changes pdata->gpio_keys after init, strange stuff can happen
 *			This can be fixed by making a local copy of pdata, but no board
 *			would do that anyway or using const modifier.
 */

#include <bits.h>
#include <stdlib.h>
#include <string.h>
#include <dev/keys.h>
#include <dev/gpio.h>
#include <dev/ssbi.h>
#include <dev/pm8058.h>
#include <kernel/timer.h>
#include <dev/gpio_keys.h>
#include <platform/gpio_hw.h>


#define GPIO_KEY_HOME		180	//MSM_GPIO(180)
#define PMIC_KEY_VOLUMEUP 	0	//PMIC_GPIO(1)
#define PMIC_KEY_VOLUMEDOWN	1	//PMIC_GPIO(2)

#define GPIO_MSM		0
#define GPIO_PMIC		1


static int some_key_pressed;
static unsigned long gpio_keys_bitmap;

static struct timer gpio_keys_poll;
static struct gpio_keys_pdata *pdata;



static enum handler_return  gpio_keys_poll_fn(struct timer *timer, time_t now, void *arg)
{
	uint32_t i = 0;
	int changed,state, gpio_value;
	struct gpio_key *key = pdata->gpio_key;
	
	for(; ((i < pdata->nr_keys) && (key != NULL)); i++, key++) 
	{
		changed = 0;

		if (key->gpio_type == GPIO_MSM)
			gpio_value = gpio_get(key->gpio_nr);
		else
			gpio_value = pm8058_gpio_get(key->gpio_nr);

		if (gpio_value ^ !!key->polarity) 
		{
			changed = !bitmap_set(&gpio_keys_bitmap, i);
		} 
		else 
		{
			changed = bitmap_clear(&gpio_keys_bitmap, i);
		}

		if (changed) 
		{
			dprintf(INFO, "KEY: State of key is changed!\n");

			state = bitmap_test(&gpio_keys_bitmap, i);

			keys_post_event(key->key_code, state);
			if (key->notify_fn) 
			{
				key->notify_fn(key->key_code, state);
			}
			if(state) 
			{
				some_key_pressed++;
			} 
			else if(some_key_pressed) 
			{
				some_key_pressed--;
			}
		}
	}

	if (some_key_pressed) {
		timer_set_oneshot(timer, pdata->debounce_time, gpio_keys_poll_fn, NULL);
	} else {
		timer_set_oneshot(timer, pdata->poll_time, gpio_keys_poll_fn, NULL);		
	}
	
	return 1;
}

void gpio_keys_init(struct gpio_keys_pdata *kpdata)
{

	int i=0,gpio,cfg;

	if(kpdata == NULL) 
	{
		dprintf(INFO,"FAILED: platform data is NULL!\n");
		return;
	} 
	else if (pdata != NULL) 
	{
		dprintf(INFO,"FAILED: platform data is set already!\n");
		return;
	} 
	else if (!kpdata->nr_keys || (!(kpdata->nr_keys < BITMAP_BITS_PER_WORD)) ) 
	{
		dprintf(INFO,"WARN: %d keys not supported, First 32 keys will be served.\n", (unsigned)kpdata->nr_keys);
		kpdata->nr_keys = 32;
	}

	pdata = kpdata;
	struct gpio_key *key = pdata->gpio_key;

	some_key_pressed = 0;
	memset(&gpio_keys_bitmap, 0, sizeof(unsigned long));

	/* Initialaze MSM GPIO's & PMIC GPIO's pins  */
	for(; ((i < pdata->nr_keys) && (key != NULL)); i++, key++)
	{
		gpio = key->gpio_nr;
		if (key->gpio_type == GPIO_MSM)
		{
	  		gpio_tlmm_config(GPIO_CFG(gpio, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA), 0);
			cfg = gpio_config(gpio, GPIO_INPUT);
			if (cfg < 0) 
			{
				dprintf(CRITICAL, "%s: cannot set MSM GPIO %d as input\n", __func__, gpio);
				continue;
			}
		}
		else 
			pm8058_gpio_config_keys_input(gpio);
		
	}


	timer_initialize(&gpio_keys_poll);
	timer_set_oneshot(&gpio_keys_poll, 0, gpio_keys_poll_fn, NULL);
}

int pm8058_gpio_config_keys_input(int gpio)
{
	int	rc;
	struct pm8058_gpio pmic_keys_param = {
		.direction	= PM_GPIO_DIR_IN,
		.pull		= PM_GPIO_PULL_UP_1P5,
		.vin_sel	= 2,
		.out_strength	= PM_GPIO_STRENGTH_NO,
		.function	= PM_GPIO_FUNC_NORMAL,
		.inv_int_pol	= 1,
	};

		rc = pm8058_gpio_config(gpio, &pmic_keys_param);
		if (rc) 
		{
		        dprintf(INFO, "FAIL pm8058_gpio_config(): rc=%d.\n", rc);
			return rc;
		}

	return 0;
}


static struct gpio_key gpio_keys_tbl[] = {
	{
		.polarity 		= 1,
		.key_code 		= KEY_HOME,
		.gpio_nr 		= GPIO_KEY_HOME,
		.gpio_type		= GPIO_MSM,
		.notify_fn 		= NULL,
	},
	{
		.polarity 		= 1,
		.key_code 		= KEY_VOLUMEUP,
		.gpio_nr 		= PMIC_KEY_VOLUMEUP,
		.gpio_type		= GPIO_PMIC,
		.notify_fn 		= NULL,
	},
	{
		.polarity 		= 1,
		.key_code 		= KEY_VOLUMEDOWN,
		.gpio_nr 		= PMIC_KEY_VOLUMEDOWN,
		.gpio_type		= GPIO_PMIC,
		.notify_fn 		= NULL,
	},
	
};

static struct gpio_keys_pdata s8600_gpio_key_pdata = {
	.nr_keys = 3,
	.poll_time = 20,
	.debounce_time = 40,
	.gpio_key = &gpio_keys_tbl[0],
};

void s8600_keys_init(void)
{
	keys_init();
	gpio_keys_init(&s8600_gpio_key_pdata);
        dprintf(INFO, "Keyboard init OK\n");
}
