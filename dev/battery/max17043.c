/*
 * Copyright (c) 2011, Shantanu Gupta <shans95g@gmail.com>
 */

#include <msm_i2c.h>
#include <dev/max17043.h>

static int is_reset_soc = 0;

uint32_t max17043_read_vcell(uint8_t addr)
{
	uint8_t data[2];
	uint32_t vcell = 0;


	msm_i2c_read(addr, VCELL0_REG, &data[0], 1); 
	msm_i2c_read(addr, VCELL1_REG, &data[1], 1); 

//#if defined(CONFIG_BATT_MICROVOLT_UNIT)
//	vcell = (((data[0] << 4) & 0xFF0) | ((data[1] >> 4) & 0xF)) * 1250;       //mVolt
//#else
	vcell = ((((data[0] << 4) & 0xFF0) | ((data[1] >> 4) & 0xF)) * 125)/100;  //Volt
//#endif

	return vcell;
}


uint32_t max17043_read_raw_vcell(uint8_t addr)
{
	uint8_t data[2];
	uint32_t vcell = 0;

	msm_i2c_read(addr, VCELL0_REG, &data[0], 1); 
	msm_i2c_read(addr, VCELL1_REG, &data[1], 1); 


	vcell = data[0] << 8 | data[1];
	vcell = (vcell >> 4) * 125 * 1000;

	dprintk(INFO "[MAX17043] VCELL=%d\n", vcell);

	return vcell;
}



uint32_t max17043_read_soc(uint8_t addr)
{
	uint8_t data[2];
	int32_t FGPureSOC = 0;
	int32_t FGAdjustSOC = 0;
	int32_t FGSOC = 0;

	msm_i2c_read(addr, SOC0_REG, &data[0],1);
	msm_i2c_read(addr, SOC1_REG, &data[1],1);

	if (is_reset_soc) {
		pr_info("%s: Reseting SOC\n", __func__);
		return -1;
	}

	/*Adjusted SOC(Ancora)
	**RCOMP : B0h, FULL : 96.7, EMPTY : 0.0
	**Adj_soc = (SOC%-EMPTY)/(FULL-EMPTY)*100
	*/

	// calculating soc
	FGPureSOC = data[0]*100 + ((data[1]*100)/256);

	// hsil for get Adjusted SOC%
	if (FGPureSOC >= 0)
//#ifdef CONFIG_MACH_APACHE
	FGAdjustSOC = ((FGPureSOC*10000)-0)/9500;
//#else		
//	FGAdjustSOC = ((FGPureSOC*10000)-0)/9670;
//#endif
	else
		FGAdjustSOC = 0;

	// rounding off and Changing to percentage.
	FGSOC=FGAdjustSOC / 100;

	if(FGAdjustSOC % 100 >= 50 && FGSOC > 1)
		FGSOC+=1;

	if(FGSOC>=100)
		FGSOC=100;

	if (FGSOC < 0)
		FGSOC = 0;

	dprintk(INFO "[MAX17043] FGPureSOC = %d (%d.%d)\tFGAdjustSOC = %d\tFGSOC = %d\n", FGPureSOC, data[0], (data[1]*100)/256, FGAdjustSOC, FGSOC); 

	return FGSOC;
}



/*
  //min,	max
	7150,	15500,	// Sony 1300mAh (Formosa)	7.1k ~ 15k
	27500,	49500,	// Sony 1300mAh (HTE)		28k  ~ 49.5k
	15500,	27500,	// Sanyo 1300mAh (HTE)		16k  ~ 27k
	100,	7150,	// Samsung 1230mAh			0.1k ~ 7k
	0,		100,	// HTC Extended 2300mAh		0k   ~ 0.1k
*/
/*
// The resistances in mOHM
const uint16_t rsns[] = {
	25,			// 25 mOHM
	20,			// 20 mOHM
	15,			// 15 mOHM
	10,			// 10 mOHM
	5,			// 5 mOHM
};
*/
int16_t ds2746_current(uint8_t addr, uint16_t resistance) {
	int16_t cur;
	int8_t s0, s1;

	msm_i2c_read(addr, DS2746_CURRENT_MSB, &s0, 1);
	msm_i2c_read(addr, DS2746_CURRENT_LSB, &s1, 1);

	cur = s0 << 8;
	cur |= s1;
	
	return (((cur >> 2) * DS2746_CURRENT_ACCUM_RES) / resistance);
}

int16_t ds2745_temperature(uint8_t addr) {
	int16_t temp;
	int8_t s0, s1;

	msm_i2c_read(addr, DS2745_TEMPERATURE_MSB, &s0, 1);
	msm_i2c_read(addr, DS2745_TEMPERATURE_LSB, &s1, 1);

	temp = s0 << 8;
	temp |= s1;
	
	return ((temp >> 5) * DS2745_TEMPERATURE_RES);
}
