/*
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

#ifndef __MAX17043_H__
#define __MAX17043_H__


/* Slave address */
#define MAX17040_SLAVE_ADDR	0x6D

/* Register address */
#define VCELL0_REG			0x02
#define VCELL1_REG			0x03
#define SOC0_REG			0x04
#define SOC1_REG			0x05
#define MODE0_REG			0x06
#define MODE1_REG			0x07
#define RCOMP0_REG			0x0C
#define RCOMP1_REG			0x0D
#define CMD0_REG			0xFE
#define CMD1_REG			0xFF

uint32_t max17043_read_vcell(uint8_t addr);
uint32_t max17043_read_soc(uint8_t addr);


#endif //__MAX17043_H__
