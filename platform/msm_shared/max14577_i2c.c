#include <err.h>
#include <debug.h>
#include <reg.h>
#include <dev/gpio.h>
#include <sys/types.h>
#include <platform/timer.h>
#include "max14577_i2c.h"
#include <platform/gpio_hw.h>


#define ACK_DELAY	10
#define TX_DELAY	10
#define STOP_DELAY	10
#define RX_DELAY	10
#define START_DELAY	10	//30 


static uint8_t I2Cm_Get(void); //get MSB first
static uint8_t I2Cm_Put(unsigned char TxTemp);
static void I2Cm_Start(void);
static void I2Cm_Stop(void);
static void I2Cm_delay(uint8_t I2Cm_DlyCnt);
static void I2Cm_NAKSend(void);
static void I2Cm_ACKSend(void);

static void I2Cm_SetSCLHiZ(void);
static void I2Cm_SetSDAHiZ(void);
static void I2Cm_SCL_Low(void);
static void I2Cm_SCL_High(void);
static void I2Cm_SDA_Low();
static void I2Cm_SDA_High();
static uint8_t I2Cm_GetSCL(void);
static uint8_t I2Cm_GetSDA(void);
static void I2Cm_Pin_Init(void);

#define		GPIO_SCL	89
#define		GPIO_SDA	55
#define		FSA_ADDR	0x4A

static	uint8_t	prevPath;
static	uint8_t	newPath;

static	MG_AL25_ACCESSORY_T	prevAcc;
static	MG_AL25_ACCESSORY_T 	newAcc;		


// ============================================================================
// I2Cm_SetSCLKHiZ()
// Set SCL pin to High-Z drive mode.
// ============================================================================
static void I2Cm_SetSCLHiZ(void)
{
	gpio_direction_input(GPIO_SCL);
}

// ============================================================================
// I2Cm_SetSDAKHiZ()
// Set SDA pin to High-Z drive mode.
// ============================================================================
static void I2Cm_SetSDAHiZ(void)
{
	gpio_direction_input(GPIO_SDA);
}

// ============================================================================
// I2Cm_SCL_Low()
// Set SCL pin to Low.
// ============================================================================
static void I2Cm_SCL_Low(void)
{
	gpio_direction_output(GPIO_SCL, 0);
}

// ============================================================================
// I2Cm_SCL_High()
// Set SCL pin to High.
// ============================================================================
static void I2Cm_SCL_High(void)
{
	gpio_direction_output(GPIO_SCL, 1);
}

// ============================================================================
// I2Cm_SDA_Low()
// Set SCL pin to Low.
// ============================================================================
static void I2Cm_SDA_Low()
{	
	gpio_direction_output(GPIO_SDA, 0);
}

// ============================================================================
// I2Cm_SDA_High()
// Set SCL pin to High.
// ============================================================================
static void I2Cm_SDA_High()
{	
	gpio_direction_input(GPIO_SDA);
}

// ============================================================================
// I2Cm_GetSCL()
// read SCL pin whether it is high or low.
// ============================================================================
static uint8_t I2Cm_GetSCL(void)
{
	return gpio_get(GPIO_SCL);
}

// ============================================================================
// I2Cm_GetSDA()
// read SDA pin whether it is high or low.
// ============================================================================
static uint8_t I2Cm_GetSDA(void)
{
	return gpio_get(GPIO_SDA);
}

// ============================================================================
// I2Cm_Pin_Init()
// Set SCL/SDA pin to High-Z drive mode.
// ============================================================================
static void I2Cm_Pin_Init(void)
{
	I2Cm_SetSCLHiZ();
	I2Cm_SetSDAHiZ();
}


//-----------------------------------------------------------------------------
//Function:	I2Cm_Put
//-----------------------------------------------------------------------------
//
//Description:
//	writes 1unsigned char data and checks ACK
//
//  ARGUMENTS:
//      TxData - Contains data to be transmitted.
//
//Returns:
//	0: Send and get ACK informs Success
//      1: Send and get NAK informs fail
//
static uint8_t I2Cm_Put(uint8_t TxTemp) //put MSB first
{
	uint8_t	i;
	uint8_t TxData;
	
	TxData = TxTemp;

	//for unsigned char Sending
	for(i=0; i<8; i++) // __|-- times 8
	{
		I2Cm_SCL_Low();		
		I2Cm_delay(TX_DELAY/2);		
		if(TxData & 0x80) 
		{
			I2Cm_SDA_High();
		}
		else 
		{
			I2Cm_SDA_Low(); 
		}
		TxData = TxData<<1;
		I2Cm_delay(TX_DELAY/2);		
		I2Cm_SCL_High();
//		while(!I2Cm_GetSCL()); //Clock Stretch	
		I2Cm_delay(TX_DELAY);		
	}	
	
	//for ACK Checking
	I2Cm_SCL_Low();
	I2Cm_delay(TX_DELAY/2);		
	I2Cm_SDA_High(); //release SDA pin open for reading ACK
	I2Cm_delay(TX_DELAY/2);		
	I2Cm_SCL_High();
//	while(!I2Cm_GetSCL()); //Clock Stretch	
	I2Cm_delay(ACK_DELAY);

	if(I2Cm_GetSDA())
		return 1;
	else 
		return 0;
//	I2Cm_SCL_Low();
//	I2Cm_delay(ACK_DELAY);
}
    
//-----------------------------------------------------------------------------
//Function:	I2Cm_get
//Description:
//	Reads 1unsigned char data
//
//  ARGUMENTS:
//      TxData - Contains data to be transmitted.
//
//
//Returns:
//	read 1unsigned char data
//
static uint8_t I2Cm_Get(void) //get MSB first
{
	uint8_t i,RxTemp;
	//for unsigned char Reading	
	
	RxTemp = 0;
	for(i=0; i<8; i++) // __|-- times 8
	{		
		I2Cm_SCL_Low();		
		I2Cm_delay(RX_DELAY/2);
		RxTemp = RxTemp<<1;	
		I2Cm_SDA_High(); 	//release SDA pin open for reading one bit		
		I2Cm_delay(RX_DELAY/2);		
		I2Cm_SCL_High();
//		while(!I2Cm_GetSCL()); //Clock Stretch	
		if(I2Cm_GetSDA())
		{
			RxTemp |= 0x01;			
		}
		I2Cm_delay(RX_DELAY);
	}	
	return RxTemp;
}






//-----------------------------------------------------------------------------
//  FUNCTION NAME: I2Cm_ACKSend
//
//  DESCRIPTION:
//    Send acknowledge to slave. 
//
//-----------------------------------------------------------------------------
//  ARGUMENTS:
//    void
//
//  RETURNS:
//    void
//
static void I2Cm_ACKSend(void)
{
	I2Cm_SCL_Low();	
	I2Cm_delay(ACK_DELAY/2);	
	I2Cm_SDA_Low();	
	I2Cm_delay(ACK_DELAY/2);	
	I2Cm_SCL_High();
//	while(!I2Cm_GetSCL()); //Clock Stretch	
	I2Cm_delay(ACK_DELAY);		
	I2Cm_SCL_Low();	
  	I2Cm_delay(ACK_DELAY/2);	
}




//-----------------------------------------------------------------------------
//  FUNCTION NAME: I2Cm_NACKSend
//
//  DESCRIPTION:
//    Send acknowledge to slave. 
//
//-----------------------------------------------------------------------------
//  ARGUMENTS:
//    void
//
//  RETURNS:
//    void
//
static void I2Cm_NAKSend(void)
{
	I2Cm_SCL_Low();	
	I2Cm_delay(ACK_DELAY/2);	
	I2Cm_SDA_High();	
	I2Cm_delay(ACK_DELAY/2);	
	I2Cm_SCL_High();
//	while(!I2Cm_GetSCL()); //Clock Stretch	
	I2Cm_delay(ACK_DELAY);		
	I2Cm_SCL_Low();	
  	I2Cm_delay(ACK_DELAY/2);	
	
}

//-----------------------------------------------------------------------------
//  FUNCTION NAME: I2Cm_Start
//
//  DESCRIPTION:
//
//-----------------------------------------------------------------------------
//
//  ARGUMENTS: none
//
//  RETURNS: none

//-----------------------------------------------------------------------------
static void I2Cm_Start(void)
 {    
    // Set pins to drive mode OPEN DRAIN LOW
    I2Cm_Pin_Init();   
    
    
    // Setup port for normal operation    
    I2Cm_delay(START_DELAY/2);
    I2Cm_SDA_Low();
    I2Cm_delay(START_DELAY/2);
    I2Cm_SCL_Low();    
    I2Cm_delay(START_DELAY/2);
}

//-----------------------------------------------------------------------------
//  FUNCTION NAME: I2Cm_Stop
//
//  DESCRIPTION:
//    This function performs no operation and is used for future
//    module compatibility.
//
//-----------------------------------------------------------------------------
//
//  ARGUMENTS: none
//
//  RETURNS: none
//
static void I2Cm_Stop(void)
 {      
    // Setup port for normal operation   
    I2Cm_SCL_Low();
    I2Cm_delay(STOP_DELAY/2); 
    I2Cm_SDA_Low();
    I2Cm_delay(STOP_DELAY/2);
    I2Cm_SCL_High();     
    //while(!I2Cm_GetSCL()); //Clock Stretch	
    I2Cm_delay(STOP_DELAY/2);
    I2Cm_SDA_High();  
    I2Cm_delay(STOP_DELAY/2); 
}    




//-----------------------------------------------------------------------------
//  FUNCTION NAME: I2Cm_delay
//
//  DESCRIPTION:
//    Create delays for I2Cm routines.
//
//-----------------------------------------------------------------------------
//
//  ARGUMENTS: 
//	Delay_Counter
//
//  RETURNS: none
//      
static void I2Cm_delay(uint8_t I2Cm_DlyCnt)
{
	udelay(I2Cm_DlyCnt);
}

#if 0
/*-----------------------------------------------------------------------
 * Send a reset sequence consisting of 9 clocks with the data signal high
 * to clock any confused device back into an idle state.  Also send a
 * <stop> at the end of the sequence for belts & suspenders.
*/ 
void fsa_send_reset(void)
{
	int j;

	I2C_SCL(1);
        I2C_SDA(1);

	I2C_TRISTATE;
	for(j = 0; j < 9; j++) {
		I2C_SCL(0);
		I2C_DELAY;
		I2C_DELAY;
		I2C_SCL(1);
		I2C_DELAY;
		I2C_DELAY;
	}
	send_stop();
	I2C_TRISTATE;
}
#endif

/*-----------------------------------------------------------------------
 * Probe to see if a chip is present.  Also good for checking for the
 * completion of EEPROM writes since the chip stops responding until
 * the write completes (typically 10mSec).
 */
int fsa_i2c_probe(void)
{
	int rc;	
	uint8_t	SlaveAdr = FSA_ADDR;

	SlaveAdr &= (~0x01);	//for write 	// and the address with the Write bit.       

	/*
	 * perform 1 byte write transaction with just address byte
	 * (fake write)
	 */
	I2Cm_Start();

	rc = I2Cm_Put(SlaveAdr);
	I2Cm_Stop();

	return (rc ? 1 : 0);

}

//-----------------------------------------------------------------------------
//  FUNCTION NAME: I2Cm_Readunsigned chars
//
//  DESCRIPTION:
//
//-----------------------------------------------------------------------------
//
//  ARGUMENTS:
//  SlaveAdr   => Address of slave
//  RxArray    => Array address to get data in.
//  SubAdr     => Sub address 
//  /*Rxunsigned*/ charCount=> Count of unsigned chars to read.

//  RETURNS:
//   	  0 if a slave responds. 
//	  1 if first slave address sending error. 
//	  2 if sub address sending error. 
//	  4 if 2nd slave address sending error. 

uint8_t fsa_i2c_read_bytes(unsigned char Reg, unsigned char *RxArray, unsigned char RxByteCount)
{
	uint8_t	i=0;
	uint8_t	SlaveAdr = FSA_ADDR;	

	SlaveAdr &= (~0x01);	//for write 	// and the address with the Write bit.       

	I2Cm_Start();
	if(I2Cm_Put(SlaveAdr))
	{
		I2Cm_Stop();
		return 1; //exit with SlaveAddress error bit on  
	} 

	if(I2Cm_Put(Reg))
	{
		I2Cm_Stop();
		return 2; //exit with SubAddress0 error bit on
	} 

	
	I2Cm_Stop(); 

	udelay(50);	// 50us
	
	I2Cm_Start();
	
	SlaveAdr |= 0x01;	//for read		//OR the address with the Read bit.

	if(I2Cm_Put(SlaveAdr)) 
		return 3; 	//exit with SlaveAddress error bit on    

	udelay(50);	// 50us
	
	for(i=0; i<RxByteCount; i++)
	{
		RxArray[i] = I2Cm_Get();

		if(i!=(RxByteCount-1)) I2Cm_ACKSend(); 
		else I2Cm_NAKSend(); 
	}

	I2Cm_Stop();

    return 0;
}

//-----------------------------------------------------------------------------
//  FUNCTION NAME: I2Cm_Writeunsigned chars
//
//  DESCRIPTION:
//
//-----------------------------------------------------------------------------
//
//  ARGUMENTS:
//  SlaveAdr   => Address of slave
//  TxArray    => Array address to put data out.
//  SubAdr     => Sub address 
//  /*Txunsigned*/ charCount=> Count of unsigned chars to read.

//  RETURNS:
//    0 if a slave responds. 
//	  1 if first slave address sending error. 
//	  8 if data sending error. 
//------------------------------------------------------------------------------ 
uint8_t fsa_i2c_write_bytes(unsigned char Reg, unsigned char *TxArray, unsigned char TxByteCount)
{    
	unsigned char	i;
	uint8_t	SlaveAdr = FSA_ADDR;
	
	SlaveAdr &= (~0x01);	//for write   //and the address with the Write bit.
	I2Cm_Start();
	if(I2Cm_Put(SlaveAdr)) 
	{	
		I2Cm_Stop();
		return 1; 	//exit with SlaveAddress error bit on   
	}  

	if(I2Cm_Put(Reg)) 
	{
		I2Cm_Stop();
		return 2; //exit with SubAddress0 error bit on
	}    


	udelay(20);	// 50us
	
	for(i=0; i<TxByteCount; i++)
	{
		if(I2Cm_Put(TxArray[i]))    
		{
			I2Cm_Stop();
			return 8;  //exit with write data error bit on
		}   	
	}
	I2Cm_Stop();
	return 0;
}

int fsa_i2c_write(uint8_t Reg, uint8_t Data)
{
	uint8_t SlaveAdr = FSA_ADDR;

	SlaveAdr &= (~0x01);	//for write   //and the address with the Write bit.

	I2Cm_Start();
	if(I2Cm_Put(SlaveAdr)) 
	{	
		I2Cm_Stop();
		return 1; 	//exit with SlaveAddress error bit on   
	}  

	if(I2Cm_Put(Reg)) 
	{
		I2Cm_Stop();
		return 2; //exit with SubAddress0 error bit on
	}    

	
	if(I2Cm_Put(Data))    
	{
		I2Cm_Stop();
		return 3;  //exit with write data error bit on
	}   	
	I2Cm_Stop();

	return 0;	
}


int fsa_i2c_read(uint8_t Reg, uint8_t *Data)
{
	uint8_t SlaveAdr = FSA_ADDR;

	SlaveAdr &= (~0x01);	//for write 	// and the address with the Write bit.       

	I2Cm_Start();
	if(I2Cm_Put(SlaveAdr))
	{
		I2Cm_Stop();
		return 1; //exit with SlaveAddress error bit on  
	} 

	if(I2Cm_Put(Reg))
	{
		I2Cm_Stop();
		return 2; //exit with SubAddress0 error bit on
	} 
	
	I2Cm_Stop(); 

	udelay(20);	// 50us
	
	I2Cm_Start();
	
	SlaveAdr |= 0x01;	//for read		//OR the address with the Read bit.

	if(I2Cm_Put(SlaveAdr)) 
		return 3; 	//exit with SlaveAddress error bit on    

	udelay(10);	// 50us
	
	*Data = I2Cm_Get();

	I2Cm_NAKSend(); 

	I2Cm_Stop();

	return 0;
}


static uint8_t get_real_usbic_state(void)
{
	uint8_t	status[3],rc;
	MD_AL25_ADC_T newAdc;
	MD_AL25_CHGTYP_T newChgTyp;
	uint8_t ret = 0;


	rc = fsa_i2c_read_bytes(MD_AL25_REG_INTSTAT1, status, 3);
	if (rc != 0)
		dprintf(INFO, "[MAX14577] i2c_read failed, error N:%02X\n", rc);	

	mdelay(5);

	newAdc = (MD_AL25_ADC_T)(( status[0] & MD_AL25_M_INTSTAT1_ADC) >> MD_AL25_B_INTSTAT1_ADC);
	newChgTyp = ( MD_AL25_CHGTYP_T )(( status[1] & MD_AL25_M_INTSTAT2_CHGTYP) >> MD_AL25_B_INTSTAT2_CHGTYP);

	dprintf(INFO, "[MAX14577] newAdc=0x%02x, newChgTyp=0x%02x\n" , newAdc, newChgTyp);

	if ( ( newChgTyp > MD_AL25_CHGTYP_DEDICATED_CHGR )  && ( newAdc != MD_AL25_ADC_OPEN) )
	{
		newAcc = MG_AL25_ACCESSORY_ILLEGAL;
		dprintf(INFO, "[MAX14577] MG_AL25_ACCESSORY_ILLEGAL \n");
		ret = MICROUSBIC_NO_DEVICE;
	}
	else if ( newAdc == MD_AL25_ADC_OPEN )
	{
		switch ( newChgTyp )
		{
			case  MD_AL25_CHGTYP_NO_VOLTAGE:
			{
				newAcc = MG_AL25_ACCESSORY_NONE;
				dprintf(INFO, "[MAX14577] Nothing Attached \n");
				ret = MICROUSBIC_NO_DEVICE;				
			}
			break;
			case  MD_AL25_CHGTYP_USB:
			{
				newAcc = MG_AL25_ACCESSORY_USB;
				dprintf(INFO,"[MAX14577]  USB cable attached \n" );
				ret = MICROUSBIC_USB_CABLE;
			}
			break;
			case  MD_AL25_CHGTYP_DOWNSTREAM_PORT:
			{
				newAcc = MG_AL25_ACCESSORY_USBCHGR;
				dprintf(INFO,"[MAX14577]  Charging Downstream port \n" );
				ret = MICROUSBIC_USB_CHARGER;
				
			}
			break;
			case  MD_AL25_CHGTYP_DEDICATED_CHGR:
			{
				newAcc = MG_AL25_ACCESSORY_DEDCHGR_1P8A;
				dprintf(INFO,"[MAX14577]  Dedicated charger detected \n" );
				ret = MICROUSBIC_TA_CHARGER;
			}
			break;
			case  MD_AL25_CHGTYP_500MA:
			{
				newAcc = MG_AL25_ACCESSORY_DEDCHGR_500MA;
				dprintf(INFO,"[MAX14577]  Special 500MA charger detected \n" );
			}
			break;
			case  MD_AL25_CHGTYP_1A:
			{
				newAcc = MG_AL25_ACCESSORY_DEDCHGR_1A;
				dprintf(INFO,"[MAX14577]  Special 1A charger detected \n" );
			}
			break;
			case  MD_AL25_CHGTYP_RFU:
			{
				newAcc = MG_AL25_ACCESSORY_ILLEGAL;
				dprintf(INFO,"[MAX14577]  Illegal accessory detected \n" );
				ret = MICROUSBIC_NO_DEVICE;
			}
			break;
			case  MD_AL25_CHGTYP_DB_100MA:
			{
				newAcc = MG_AL25_ACCESSORY_DEDCHGR_100MA;
				dprintf(INFO,"[MAX14577] Dead Battery Charging detected \n" );
			}
			break;
			default :
			{
				newAcc = MG_AL25_ACCESSORY_ILLEGAL;
				dprintf(INFO,"[MAX14577]  Default Acc\n" );
				ret = MICROUSBIC_NO_DEVICE;
			}
		}
	}
	else if ( newAdc == MD_AL25_ADC_255K )	
	{
		newAcc = MG_AL25_ACCESSORY_FACTORY_USB_BOOT_OFF;
		dprintf(INFO,"[MAX14577]  MICRO_JIG_USB_OFF detected \n" );
		ret = MICROUSBIC_JIG_USB_OFF;
	}
	else if ( newAdc == MD_AL25_ADC_301K )	
	{
		newAcc = MG_AL25_ACCESSORY_FACTORY_USB_BOOT_ON;
		dprintf(INFO,"[MAX14577]  MICRO_JIG_USB_ON detected \n" );
		ret = MICROUSBIC_JIG_USB_ON;
	}
	else if ( newAdc == MD_AL25_ADC_523K )
	{
		newAcc = MG_AL25_ACCESSORY_FACTORY_UART_BOOT_OFF;
		dprintf(INFO,"[MAX14577]  MICRO_JIG_UART_OFF detected \n" );
		ret = MICROUSBIC_JIG_UART_OFF;
	}
	else if ( newAdc == MD_AL25_ADC_619K )
	{
		newAcc = MG_AL25_ACCESSORY_FACTORY_UART_BOOT_ON;
		dprintf(INFO,"[MAX14577]  MICRO_JIG_UART_ON detected \n" );
		ret = MICROUSBIC_JIG_UART_ON;
	}
	else
	{
		newAcc = MG_AL25_ACCESSORY_ILLEGAL;
		ret = MICROUSBIC_NO_DEVICE;
	}


	return ret;
}


static uint8_t	switch_path(uint8_t newDev)
{
	uint8_t	ctrl1Reg, rc;

	switch (newDev)
	{ 
		case	MICROUSBIC_USB_CABLE:
		case	MICROUSBIC_USB_CHARGER:
		case	MICROUSBIC_JIG_USB_OFF:
		case	MICROUSBIC_JIG_USB_ON:
		{
			newPath	= USB;
		}	
		break;
		case	MICROUSBIC_JIG_UART_OFF:
		case	MICROUSBIC_JIG_UART_ON:
		{
			newPath	= UART;
		}
		break;
		case	MICROUSBIC_NO_DEVICE:
		default:
		{
			newPath = OPEN;
		}
	}
		
	if (newPath != prevPath)
	{
		if (newPath == USB)
		{
			ctrl1Reg =  (MD_AL25_REG_CTRL1_IDBEN_OPEN | MD_AL25_REG_CTRL1_MICEN_OPEN | \
			MD_AL25_REG_CTRL1_COMP2SW_DP2 | MD_AL25_REG_CTRL1_COMN1SW_DN1 );
		}
		else if (newPath == UART)
		{
			ctrl1Reg = (MD_AL25_REG_CTRL1_IDBEN_OPEN | MD_AL25_REG_CTRL1_MICEN_OPEN | \
			MD_AL25_REG_CTRL1_COMP2SW_UR2 | MD_AL25_REG_CTRL1_COMN1SW_UT1 );
		}

		else // newPath is OPEN or AUDIO
		{
			ctrl1Reg = ( MD_AL25_REG_CTRL1_IDBEN_OPEN    |	MD_AL25_REG_CTRL1_MICEN_OPEN    | \
			MD_AL25_REG_CTRL1_COMP2SW_HIZ   | MD_AL25_REG_CTRL1_COMN1SW_HIZ	);
		}

		prevPath = newPath;
		rc = fsa_i2c_write(MD_AL25_REG_CTRL1, ctrl1Reg);

		if (rc != 0)
			dprintf(INFO, "[MAX14577] i2c_write failed, error N:%02X\n", rc);	
		
		dprintf(INFO, "[MAX14577] Switch to new path :%02X\n", newPath);	
		

	}
		
return newPath;
}



void max14577_i2c_init(void)
{
	uint8_t	rc, hw_rev;
	uint8_t	status[3],IntMaskReg[3], CdetCtrlReg, CtrlReg[3];
	uint8_t newDev, ChrCtrl2Reg, path;

	
	gpio_tlmm_config(GPIO_CFG(GPIO_SDA, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA),GPIO_ENABLE);
	gpio_tlmm_config(GPIO_CFG(GPIO_SCL, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA),GPIO_ENABLE);    

	rc = fsa_i2c_probe();
        if (rc == 0)
		dprintf(INFO, "[MAX14577] i2c_probe OK\n");
	else
		dprintf(INFO, "[MAX14577] i2c_probe failed, address not <ACK>\n");

	rc = fsa_i2c_read(MD_AL25_REG_DEVICEID, &hw_rev);
        dprintf(INFO,"[MAX14577] MD_AL25_REG_DEVICEID %02X\n", hw_rev);

	rc = fsa_i2c_read_bytes(MD_AL25_REG_INTSTAT1, status, 3);
	if (rc == 0)
	{
		dprintf(INFO,"[MAX14577] MD_AL25_REG_INTSTAT1 %02X\n", status[0]);
		dprintf(INFO,"[MAX14577] MD_AL25_REG_INTSTAT2 %02X\n", status[1]);
		dprintf(INFO,"[MAX14577] MD_AL25_REG_INTSTAT3 %02X\n", status[2]);	
	}
	else
		dprintf(INFO, "[MAX14577] i2c_read failed, error N:%02X\n", rc);	


	// 
	// Setup Default register values
	//
	IntMaskReg[0] =
	(
		MD_AL25_REG_INTMASK1_ADCERR_ENABLE   |
		MD_AL25_REG_INTMASK1_ADCLOW_ENABLE   |
		MD_AL25_REG_INTMASK1_ADC_ENABLE
	);

	IntMaskReg[1] =
	(
		MD_AL25_REG_INTMASK2_VBVOLT_ENABLE       |
		MD_AL25_REG_INTMASK2_DBCHG_ENABLE        |
		MD_AL25_REG_INTMASK2_DCDTMR_ENABLE       |
		MD_AL25_REG_INTMASK2_CHGDETRUN_DISABLE   |
		MD_AL25_REG_INTMASK2_CHGTYP_ENABLE
	);

	IntMaskReg[2] =
	(
		MD_AL25_REG_INTMASK3_MBCCHGERR_ENABLE   |
		MD_AL25_REG_INTMASK3_OVP_ENABLE         |
		MD_AL25_REG_INTMASK3_CGMBC_DISABLE      |		//Changed 14.01.2013
		MD_AL25_REG_INTMASK3_EOC_ENABLE
	);

	CdetCtrlReg =
	(
		MD_AL25_REG_CDETCTRL_CDPDET_VDP_SRC    |
		MD_AL25_REG_CDETCTRL_DBEXIT_DISABLE    |
		MD_AL25_REG_CDETCTRL_DCHK_50MS         |
		MD_AL25_REG_CDETCTRL_DCD2SCT_EXIT      |
		MD_AL25_REG_CDETCTRL_DCDEN_ENABLE      |
		MD_AL25_REG_CDETCTRL_CHGTYPM_DISABLE   |
		MD_AL25_REG_CDETCTRL_CHGDETEN_ENABLE
	);

	CtrlReg[0] =
	(
		MD_AL25_REG_CTRL1_IDBEN_OPEN    |
		MD_AL25_REG_CTRL1_MICEN_OPEN    |
		MD_AL25_REG_CTRL1_COMP2SW_HIZ   |
		MD_AL25_REG_CTRL1_COMN1SW_HIZ
	);

	CtrlReg[1] =
	(
		MD_AL25_REG_CTRL2_RCPS_DISABLE       |
		MD_AL25_REG_CTRL2_USBCPLNT_DISABLE   |
		MD_AL25_REG_CTRL2_ACCDET_ENABLE|
		MD_AL25_REG_CTRL2_SFOUTORD_NORMAL    |
		MD_AL25_REG_CTRL2_SFOUTASRT_NORMAL   |
		MD_AL25_REG_CTRL2_CPEN_DISABLE       |
		MD_AL25_REG_CTRL2_ADCEN_ENABLE       |
		MD_AL25_REG_CTRL2_LOWPWR_DISABLE
	);

	CtrlReg[2] =
	(
		MD_AL25_REG_CTRL3_WBTH_3P7V        |
		MD_AL25_REG_CTRL3_ADCDBSET_25MS   |
		MD_AL25_REG_CTRL3_BOOTSET_AUTO     |
		MD_AL25_REG_CTRL3_JIGSET_AUTO
	);


	ChrCtrl2Reg = (MD_AL25_REG_CHGCTRL2_VCHGR_RC_DISABLE | MD_AL25_REG_CHGCTRL2_MBHOSTEN_DISABLE );


	//Write registers
	rc = fsa_i2c_write_bytes(MD_AL25_REG_INTMASK1, IntMaskReg, 3);

	if (rc != 0)
		dprintf(INFO, "[MAX14577] i2c_write IntMaskReg failed, error N:%02X\n", rc);	

	rc = fsa_i2c_write(MD_AL25_REG_CDETCTRL, CdetCtrlReg); 

	if (rc != 0)
		dprintf(INFO, "[MAX14577] i2c_write CDETCTRL failed, error N:%02X\n", rc);	


	rc = fsa_i2c_write_bytes(MD_AL25_REG_CTRL1, CtrlReg, 3);

	if (rc != 0)
		dprintf(INFO, "[MAX14577] i2c_write CtrlReg failed, error N:%02X\n", rc);	


	rc = fsa_i2c_write(MD_AL25_REG_CHGCTRL2, ChrCtrl2Reg);  //in bootloader state charge is disabled

	if (rc != 0)
		dprintf(INFO, "[MAX14577] i2c_write failed, error N:%02X\n", rc);	
        

	prevPath = OPEN;

	newDev = get_real_usbic_state();

	path = switch_path(newDev);
	
}