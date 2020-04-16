/********************************************************************************************************
 * @file     gpio_8258.c 
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *			 The information contained herein is confidential and proprietary property of Telink 
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in. 
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this 
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/

#include "../config/user_config.h"
#include "../mcu/config.h"


#if(__TL_LIB_8258__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8258))

#include "../common/compatibility.h"
#include "../common/utility.h"
#include "../mcu/compiler.h"
#include "../mcu/register.h"
#include "../mcu/analog.h"
#include "proj_lib/pm_8258.h"

#include "../mcu/gpio.h"

#ifdef PWM_FUNC_R
    #ifdef PWM_R
STATIC_ASSERT((PWM_FUNC_R == AS_PWM)||((PWM_FUNC_R == AS_PWM_SECOND)&&((PWM_R==GPIO_PC1)||(PWM_R==GPIO_PC4)||(PWM_R==GPIO_PD5))));
    #endif
#endif
#ifdef PWM_FUNC_G
    #ifdef PWM_G
STATIC_ASSERT((PWM_FUNC_G == AS_PWM)||((PWM_FUNC_G == AS_PWM_SECOND)&&((PWM_G==GPIO_PC1)||(PWM_G==GPIO_PC4)||(PWM_G==GPIO_PD5))));
    #endif
#endif
#ifdef PWM_FUNC_B
    #ifdef PWM_B
STATIC_ASSERT((PWM_FUNC_B == AS_PWM)||((PWM_FUNC_B == AS_PWM_SECOND)&&((PWM_B==GPIO_PC1)||(PWM_B==GPIO_PC4)||(PWM_B==GPIO_PD5))));
    #endif
#endif
#ifdef PWM_FUNC_W
    #ifdef PWM_W
STATIC_ASSERT((PWM_FUNC_W == AS_PWM)||((PWM_FUNC_W == AS_PWM_SECOND)&&((PWM_W==GPIO_PC1)||(PWM_W==GPIO_PC4)||(PWM_W==GPIO_PD5))));
    #endif
#endif


/************
 *
 * gpio:         indicate the pin
 * up_down:      1 need pull up, 0 need pull down
 */

/************
 *
 * gpio:         indicate the pin
 * up_down:      0	   : float
 * 				 1    : 1M   pullup
 * 				 2    : 100K pulldown
 * 				 3    : 10K  pullup

 *
 *     BIT(7.6)   BIT(5.4)   BIT(3.2)   BIT(1.0)
mask_not 0x3f       0xcf	  0xf3       0xfc

 0e		 PA3		 PA2	  PA1        PA0
 0f		 PA7         PA6      PA5        PA4		0
 10		 PB3         PB2      PA1        PB0
 11		 PB7         PB6      PB5        PB4		1
 12		 PC3         PC2      PC1        PC0
 13		 PC7         PC6      PC5        PC4		2
 14		 PD3         PD2      PD1        PD0
 15		 PD7         PD6      PD5        PD4		3

 */

//if GPIO_DP,please check usb_dp_pullup_en() valid or not first.
void gpio_setup_up_down_resistor(GPIO_PinTypeDef gpio, GPIO_PullTypeDef up_down)
{
	unsigned char r_val = up_down & 0x03;


	unsigned char base_ana_reg = 0x0e + ((gpio >> 8) << 1) + ( (gpio & 0xf0) ? 1 : 0 );  //group = gpio>>8;
	unsigned char shift_num, mask_not;

	if(gpio & 0x11){
		shift_num = 0;
		mask_not = 0xfc;
	}
	else if(gpio & 0x22){
		shift_num = 2;
		mask_not = 0xf3;
	}
	else if(gpio & 0x44){
		shift_num = 4;
		mask_not = 0xcf;
	}
	else if(gpio & 0x88){
		shift_num = 6;
		mask_not = 0x3f;
	}
	else{
		return;
	}


    if(GPIO_DP == gpio){
        usb_dp_pullup_en (0);
    }

	analog_write(base_ana_reg, (analog_read(base_ana_reg) & mask_not) | (r_val << shift_num));
}


void gpio_set_wakeup(GPIO_PinTypeDef pin, GPIO_LevelTypeDef level, int en)
{
	unsigned char bit = pin & 0xff;
    if (en) {
        BM_SET(reg_gpio_irq_wakeup_en(pin), bit);
    }
    else {
        BM_CLR(reg_gpio_irq_wakeup_en(pin), bit);
    }

    if(level == Level_High){
        BM_CLR(reg_gpio_pol(pin), bit);
    }else{
        BM_SET(reg_gpio_pol(pin), bit);
    }
}


void gpio_set_input_en(GPIO_PinTypeDef pin, unsigned int value)
{
	unsigned char	bit = pin & 0xff;
	unsigned short group = pin & 0xf00;

	if(group == GPIO_GROUPA || group == GPIO_GROUPD || group == GPIO_GROUPE)
	{
		if(value){
			BM_SET(reg_gpio_ie(pin), bit);
		}else{
			BM_CLR(reg_gpio_ie(pin), bit);
		}
	}
    else if(group == GPIO_GROUPB)
    {
    	if(value){
    		analog_write(anaReg_gpioPB_ie, analog_read(anaReg_gpioPB_ie)|bit);
    	}
    	else
    	{
    		analog_write(anaReg_gpioPB_ie, analog_read(anaReg_gpioPB_ie)&(~bit));
    	}
    }
    else if(group == GPIO_GROUPC)
    {
    	if(value){
    		analog_write(anaReg_gpioPC_ie, analog_read(anaReg_gpioPC_ie)|bit);
    	}
    	else
    	{
    		analog_write(anaReg_gpioPC_ie, analog_read(anaReg_gpioPC_ie)&(~bit));
    	}
    }

}



void gpio_set_data_strength(GPIO_PinTypeDef pin, unsigned int value)
{
	unsigned char bit = pin & 0xff;
	unsigned short group = pin & 0xf00;
    if( (group == GPIO_GROUPA) || (group==GPIO_GROUPD) || (group == GPIO_GROUPE))
    {
    	if(value){
    		BM_SET(reg_gpio_ds(pin), bit);
    	}else{
    		BM_CLR(reg_gpio_ds(pin), bit);
    	}
    }
    else if(group == GPIO_GROUPB)
    {
    	if(value){
    		analog_write(anaReg_gpioPB_ds, analog_read(anaReg_gpioPB_ds)|bit);
    	}
    	else
    	{
    		analog_write(anaReg_gpioPB_ds, analog_read(anaReg_gpioPB_ds)&(~bit));
    	}
    }
    else if(group == GPIO_GROUPC)
    {
    	if(value){
    		analog_write(anaReg_gpioPC_ds, analog_read(anaReg_gpioPC_ds)|bit);
    	}
    	else
    	{
    		analog_write(anaReg_gpioPC_ds, analog_read(anaReg_gpioPC_ds)&(~bit));
    	}
    }
}





#if (PM_DEEPSLEEP_RETENTION_ENABLE)
_attribute_ram_code_
#endif
void gpio_init()
{

	reg_gpio_pa_setting1 =
		(PA0_INPUT_ENABLE<<8) 	| (PA1_INPUT_ENABLE<<9)	| (PA2_INPUT_ENABLE<<10)	| (PA3_INPUT_ENABLE<<11) |
		(PA4_INPUT_ENABLE<<12)	| (PA5_INPUT_ENABLE<<13)	| (PA6_INPUT_ENABLE<<14)	| (PA7_INPUT_ENABLE<<15) |
		((PA0_OUTPUT_ENABLE?0:1)<<16)	| ((PA1_OUTPUT_ENABLE?0:1)<<17) | ((PA2_OUTPUT_ENABLE?0:1)<<18)	| ((PA3_OUTPUT_ENABLE?0:1)<<19) |
		((PA4_OUTPUT_ENABLE?0:1)<<20)	| ((PA5_OUTPUT_ENABLE?0:1)<<21) | ((PA6_OUTPUT_ENABLE?0:1)<<22)	| ((PA7_OUTPUT_ENABLE?0:1)<<23) |
		(PA0_DATA_OUT<<24)	| (PA1_DATA_OUT<<25)	| (PA2_DATA_OUT<<26)	| (PA3_DATA_OUT<<27) |
		(PA4_DATA_OUT<<28)	| (PA5_DATA_OUT<<29)	| (PA6_DATA_OUT<<30)	| (PA7_DATA_OUT<<31) ;
	reg_gpio_pa_setting2 =
		(PA0_DATA_STRENGTH<<8)		| (PA1_DATA_STRENGTH<<9)| (PA2_DATA_STRENGTH<<10)	| (PA3_DATA_STRENGTH<<11) |
		(PA4_DATA_STRENGTH<<12)	| (PA5_DATA_STRENGTH<<13)	| (PA6_DATA_STRENGTH<<14)	| (PA7_DATA_STRENGTH<<15) |
		(PA0_FUNC==AS_GPIO ? BIT(16):0)	| (PA1_FUNC==AS_GPIO ? BIT(17):0)| (PA2_FUNC==AS_GPIO ? BIT(18):0)| (PA3_FUNC==AS_GPIO ? BIT(19):0) |
		(PA4_FUNC==AS_GPIO ? BIT(20):0)	| (PA5_FUNC==AS_GPIO ? BIT(21):0)| (PA6_FUNC==AS_GPIO ? BIT(22):0)| (PA7_FUNC==AS_GPIO ? BIT(23):0);




	//PB group
	//ie
	analog_write(anaReg_gpioPB_ie, 	(PB0_INPUT_ENABLE<<0) 	| (PB1_INPUT_ENABLE<<1)	| (PB2_INPUT_ENABLE<<2)	| (PB3_INPUT_ENABLE<<3) |
									(PB4_INPUT_ENABLE<<4)	| (PB5_INPUT_ENABLE<<5) | (PB6_INPUT_ENABLE<<6)	| (PB7_INPUT_ENABLE<<7) );
	//oen
	reg_gpio_pb_oen =
		((PB0_OUTPUT_ENABLE?0:1)<<0)	| ((PB1_OUTPUT_ENABLE?0:1)<<1) | ((PB2_OUTPUT_ENABLE?0:1)<<2)	| ((PB3_OUTPUT_ENABLE?0:1)<<3) |
		((PB4_OUTPUT_ENABLE?0:1)<<4)	| ((PB5_OUTPUT_ENABLE?0:1)<<5) | ((PB6_OUTPUT_ENABLE?0:1)<<6)	| ((PB7_OUTPUT_ENABLE?0:1)<<7);
	//dataO
	reg_gpio_pb_out =
		(PB0_DATA_OUT<<0)	| (PB1_DATA_OUT<<1)	| (PB2_DATA_OUT<<2)	| (PB3_DATA_OUT<<3) |
		(PB4_DATA_OUT<<4)	| (PB5_DATA_OUT<<5)	| (PB6_DATA_OUT<<6)	| (PB7_DATA_OUT<<7) ;

	//ds
	analog_write(anaReg_gpioPB_ds, 	(PB0_DATA_STRENGTH<<0) 	| (PB1_DATA_STRENGTH<<1)  | (PB2_DATA_STRENGTH<<2)	| (PB3_DATA_STRENGTH<<3) |
									(PB4_DATA_STRENGTH<<4)	| (PB5_DATA_STRENGTH<<5)  | (PB6_DATA_STRENGTH<<6)	| (PB7_DATA_STRENGTH<<7) );
	//func
	reg_gpio_pb_gpio =
		(PB0_FUNC==AS_GPIO ? BIT(0):0)	| (PB1_FUNC==AS_GPIO ? BIT(1):0)| (PB2_FUNC==AS_GPIO ? BIT(2):0)| (PB3_FUNC==AS_GPIO ? BIT(3):0) |
		(PB4_FUNC==AS_GPIO ? BIT(4):0)	| (PB5_FUNC==AS_GPIO ? BIT(5):0)| (PB6_FUNC==AS_GPIO ? BIT(6):0)| (PB7_FUNC==AS_GPIO ? BIT(7):0);




	//PC group
	//ie
	analog_write(anaReg_gpioPC_ie, 	(PC0_INPUT_ENABLE<<0) 	| (PC1_INPUT_ENABLE<<1)	| (PC2_INPUT_ENABLE<<2)	| (PC3_INPUT_ENABLE<<3) |
									(PC4_INPUT_ENABLE<<4)	| (PC5_INPUT_ENABLE<<5) | (PC6_INPUT_ENABLE<<6)	| (PC7_INPUT_ENABLE<<7) );

	//oen
	reg_gpio_pc_oen =
		((PC0_OUTPUT_ENABLE?0:1)<<0)	| ((PC1_OUTPUT_ENABLE?0:1)<<1) | ((PC2_OUTPUT_ENABLE?0:1)<<2)	| ((PC3_OUTPUT_ENABLE?0:1)<<3) |
		((PC4_OUTPUT_ENABLE?0:1)<<4)	| ((PC5_OUTPUT_ENABLE?0:1)<<5) | ((PC6_OUTPUT_ENABLE?0:1)<<6)	| ((PC7_OUTPUT_ENABLE?0:1)<<7);
	//dataO
	reg_gpio_pc_out =
		(PC0_DATA_OUT<<0)	| (PC1_DATA_OUT<<1)	| (PC2_DATA_OUT<<2)	| (PC3_DATA_OUT<<3) |
		(PC4_DATA_OUT<<4)	| (PC5_DATA_OUT<<5)	| (PC6_DATA_OUT<<6)	| (PC7_DATA_OUT<<7) ;

	//ds
	analog_write(anaReg_gpioPC_ds, 	(PC0_DATA_STRENGTH<<0) 	| (PC1_DATA_STRENGTH<<1)  | (PC2_DATA_STRENGTH<<2)	| (PC3_DATA_STRENGTH<<3) |
									(PC4_DATA_STRENGTH<<4)	| (PC5_DATA_STRENGTH<<5)  | (PC6_DATA_STRENGTH<<6)	| (PC7_DATA_STRENGTH<<7) );

	reg_gpio_pc_gpio =
		(PC0_FUNC==AS_GPIO ? BIT(0):0)	| (PC1_FUNC==AS_GPIO ? BIT(1):0)| (PC2_FUNC==AS_GPIO ? BIT(2):0)| (PC3_FUNC==AS_GPIO ? BIT(3):0) |
		(PC4_FUNC==AS_GPIO ? BIT(4):0)	| (PC5_FUNC==AS_GPIO ? BIT(5):0)| (PC6_FUNC==AS_GPIO ? BIT(6):0)| (PC7_FUNC==AS_GPIO ? BIT(7):0);




	//PD group
	reg_gpio_pd_setting1 =
		(PD0_INPUT_ENABLE<<8) 	| (PD1_INPUT_ENABLE<<9)	| (PD2_INPUT_ENABLE<<10)	| (PD3_INPUT_ENABLE<<11) |
		(PD4_INPUT_ENABLE<<12)	| (PD5_INPUT_ENABLE<<13)| (PD6_INPUT_ENABLE<<14)	| (PD7_INPUT_ENABLE<<15) |
		((PD0_OUTPUT_ENABLE?0:1)<<16)	| ((PD1_OUTPUT_ENABLE?0:1)<<17) | ((PD2_OUTPUT_ENABLE?0:1)<<18)	| ((PD3_OUTPUT_ENABLE?0:1)<<19) |
		((PD4_OUTPUT_ENABLE?0:1)<<20)	| ((PD5_OUTPUT_ENABLE?0:1)<<21) | ((PD6_OUTPUT_ENABLE?0:1)<<22)	| ((PD7_OUTPUT_ENABLE?0:1)<<23) |
		(PD0_DATA_OUT<<24)	| (PD1_DATA_OUT<<25)	| (PD2_DATA_OUT<<26)	| (PD3_DATA_OUT<<27) |
		(PD4_DATA_OUT<<28)	| (PD5_DATA_OUT<<29)	| (PD6_DATA_OUT<<30)	| (PD7_DATA_OUT<<31) ;
	reg_gpio_pd_setting2 =
		(PD0_DATA_STRENGTH<<8)	| (PD1_DATA_STRENGTH<<9)	| (PD2_DATA_STRENGTH<<10)	| (PD3_DATA_STRENGTH<<11) |
		(PD4_DATA_STRENGTH<<12)	| (PD5_DATA_STRENGTH<<13)	| (PD6_DATA_STRENGTH<<14)	| (PD7_DATA_STRENGTH<<15) |
		(PD0_FUNC==AS_GPIO ? BIT(16):0)	| (PD1_FUNC==AS_GPIO ? BIT(17):0)| (PD2_FUNC==AS_GPIO ? BIT(18):0)| (PD3_FUNC==AS_GPIO ? BIT(19):0) |
		(PD4_FUNC==AS_GPIO ? BIT(20):0)	| (PD5_FUNC==AS_GPIO ? BIT(21):0)| (PD6_FUNC==AS_GPIO ? BIT(22):0)| (PD7_FUNC==AS_GPIO ? BIT(23):0);


	//PE group
	reg_gpio_pe_ie = (PE0_INPUT_ENABLE<<0)	| (PE1_INPUT_ENABLE<<1)| (PE2_INPUT_ENABLE<<2)	| (PE3_INPUT_ENABLE<<3);
	reg_gpio_pe_oen = ((PE0_OUTPUT_ENABLE?0:1)<<0)	| ((PE1_OUTPUT_ENABLE?0:1)<<1) | ((PE2_OUTPUT_ENABLE?0:1)<<2)	| ((PE3_OUTPUT_ENABLE?0:1)<<3);
	reg_gpio_pe_out = (PE0_DATA_OUT<<0)	| (PE1_DATA_OUT<<1)	| (PE2_DATA_OUT<<2)	| (PE3_DATA_OUT<<3);
	reg_gpio_pe_ds = (PE0_DATA_STRENGTH<<0)	| (PE1_DATA_STRENGTH<<1)	| (PE2_DATA_STRENGTH<<2)	| (PE3_DATA_STRENGTH<<3);
	reg_gpio_pe_gpio = (PE0_FUNC==AS_GPIO ? BIT(0):0)	| (PE1_FUNC==AS_GPIO ? BIT(1):0)| (PE2_FUNC==AS_GPIO ? BIT(2):0)| (PE3_FUNC==AS_GPIO ? BIT(3):0);



    if(!pm_is_MCU_deepRetentionWakeup()){
	analog_write (0x0e,  PULL_WAKEUP_SRC_PA0 |
						(PULL_WAKEUP_SRC_PA1<<2) |
						(PULL_WAKEUP_SRC_PA2<<4) |
						(PULL_WAKEUP_SRC_PA3<<6));

	analog_write (0x0f,  PULL_WAKEUP_SRC_PA4 |
						(PULL_WAKEUP_SRC_PA5<<2) |
						(PULL_WAKEUP_SRC_PA6<<4) |
						(PULL_WAKEUP_SRC_PA7<<6));


	analog_write (0x10,  PULL_WAKEUP_SRC_PB0 |
						(PULL_WAKEUP_SRC_PB1<<2) |
						(PULL_WAKEUP_SRC_PB2<<4) |
						(PULL_WAKEUP_SRC_PB3<<6));

	analog_write (0x11,  PULL_WAKEUP_SRC_PB4 |
						(PULL_WAKEUP_SRC_PB5<<2) |
						(PULL_WAKEUP_SRC_PB6<<4) |
						(PULL_WAKEUP_SRC_PB7<<6));


	analog_write (0x12,  PULL_WAKEUP_SRC_PC0 |
						(PULL_WAKEUP_SRC_PC1<<2) |
						(PULL_WAKEUP_SRC_PC2<<4) |
						(PULL_WAKEUP_SRC_PC3<<6));

	analog_write (0x13,  PULL_WAKEUP_SRC_PC4 |
						(PULL_WAKEUP_SRC_PC5<<2) |
						(PULL_WAKEUP_SRC_PC6<<4) |
						(PULL_WAKEUP_SRC_PC7<<6));


	analog_write (0x14,  PULL_WAKEUP_SRC_PD0 |
						(PULL_WAKEUP_SRC_PD1<<2) |
						(PULL_WAKEUP_SRC_PD2<<4) |
						(PULL_WAKEUP_SRC_PD3<<6));

	analog_write (0x15,  PULL_WAKEUP_SRC_PD4 |
						(PULL_WAKEUP_SRC_PD5<<2) |
						(PULL_WAKEUP_SRC_PD6<<4) |
						(PULL_WAKEUP_SRC_PD7<<6));
    }
}










/*********************************************************************************
 1. PWM function gpio

    PWM0   :  PB0  PB5	PC2	 PD0  PD4
    PWM1   :  PB1  PC0	PD1
    PWM2   :  PB2  PD2
    PWM3   :  PB3  PD3
    PWM4   :  PB4  PC0  PD4
    PWM5   :  PB5  PC1  PD5
    PWM0_N :  PA0  PB1	PB4	PD5
    PWM1_N :  PB0  PC3
    PWM2_N :  PC4
    PWM3_N :  PC5
    PWM4_N :  PB6
    PWM5_N :  PB7


 *
 *
 *
 *
 *
 *********************************************************************************/
void gpio_config_special_func(GPIO_PinTypeDef pin, GPIO_FuncTypeDef func)
{
	//config gpio special func
	unsigned char val = 0;
	unsigned char mask = 0xff;
	switch(pin)
	{
		case GPIO_PA0:
		{
			//0x5a8[1:0]
			//0. DMIC_DI
			//1. PWM0_N
			//2. UART_RX
			mask = (unsigned char)~(BIT(1)|BIT(0));
			if(func == AS_DMIC){

			}else if(func == AS_PWM){
				val = BIT(0);
			}
			else if(func == AS_UART){
				val = BIT(1);
			}
		}
		break;

		case GPIO_PA1:
		{
			//0x5a8[3:2]
			//0. DMIC_CLK
			//1. UART_7816_CLK
			//2. I2S_CLK
			mask= (unsigned char)~(BIT(3)|BIT(2));
			if(func == AS_DMIC){

			}else if(func == AS_UART){
				val = BIT(2);
			}
			else if(func == AS_I2S){
				val = BIT(3);
			}
		}
		break;

		case GPIO_PA2:
		{
			//0x5a8[5:4]
			//0. DO
			//1. UART_TX
			//2. PWM0
			mask= (unsigned char)~(BIT(5)|BIT(4));
			if(func == AS_SPI){

			}else if(func == AS_UART){
				val = BIT(4);
			}
			else if(func == AS_PWM){
				val = BIT(5);
			}
		}
		break;


		case GPIO_PA3:
		{
			//0x5a8[7:6]
			//0. SPI_DI / I2C_SDA
			//1. UART_CTS
			//2. PWM1
			mask= (unsigned char)~(BIT(7)|BIT(6));
			if(func == AS_SPI || func == AS_I2C){

			}
			else if(func == AS_UART){
				val = BIT(6);
			}
			else if(func == AS_PWM){
				val = BIT(7);
			}
		}
		break;


		case GPIO_PA4:
		{
			//0x5a9[1:0]
			//0. SPI_DO / I2C_SCL
			//1. UART_TX
			//2. PWM0
			mask= (unsigned char)~(BIT(1)|BIT(0));
			if(func == AS_SPI || func == AS_I2C){

			}
			else if(func == AS_UART){
				val = BIT(0);
			}
			else if(func == AS_PWM){
				val = BIT(1);
			}
		}
		break;


		case GPIO_PA5:
		case GPIO_PA6:
		{
			//only USB, no need set special func, but we can DM/DP input enable here
			 gpio_set_input_en(pin, 1);
		}
		break;


		case GPIO_PA7:
		{
			//0x5a9[7:6]
			//0. SWS:
			//1. UART_RTS
			mask = (unsigned char)~(BIT(7)|BIT(6));
			if(func == AS_SWIRE){

			}else if(func == AS_UART){
				val = BIT(6);
			}
		}
		break;


		case GPIO_PB0:
		{
			//0x5aa[1:0]
			//0. PWM3
			//1. UART_RX
			//2. ATSEL1
			mask = (unsigned char)~(BIT(1)|BIT(0));
			if(func == AS_PWM){

			}else if(func == AS_UART){
				val = BIT(0);
			}
			else if(func == AS_ATS){
				val = BIT(1);
			}
		}
		break;


		case GPIO_PB1:
		{
			//0x5aa[3:2]
			//0. PWM4
			//1. UART_TX
			//2. ATSEL2
			mask = (unsigned char)~(BIT(3)|BIT(2));
			if(func == AS_PWM){

			}else if(func == AS_UART){
				val = BIT(2);
			}
			else if(func == AS_ATS){
				val = BIT(3);
			}
		}
		break;


		case GPIO_PB2:
		{
			//0x5aa[5:4]
			//0. PWM5
			//1. UART_CTS
			//2. RX_CYC2LNA
			mask = (unsigned char)~(BIT(5)|BIT(4));
			if(func == AS_PWM){

			}else if(func == AS_UART){
				val = BIT(4);
			}
//			else if(func == AS_CYC){
//				val = BIT(5);
//			}
		}
		break;


		case GPIO_PB3:
		{
			//0x5aa[7:6]
			//0. PWM0_N
			//1. CMP_DF
			//2. TX_CYC2PA
			mask = (unsigned char)~(BIT(7)|BIT(6));
			if(func == AS_PWM){

			}else if(func == AS_UART){
				val = BIT(6);
			}
//			else if(func == AS_CYC){
//				val = BIT(7);
//			}
		}
		break;



		case GPIO_PB4:
		{
			//0x5ab[1:0]
			//0. SDM_P0
			//1. PWM4
			//2. CMP_D
			mask = (unsigned char)~(BIT(1)|BIT(0));
			if(func == AS_SDM){

			}else if(func == AS_PWM){
				val = BIT(0);
			}
			else if(func == AS_CMP){
				val = BIT(1);
			}
		}
		break;


		case GPIO_PB5:
		{
			//0x5ab[3:2]
			//0. SDM_N0
			//1. PWM5
			//2. CMP_DF
			mask = (unsigned char)~(BIT(3)|BIT(2));
			if(func == AS_SDM){

			}else if(func == AS_PWM){
				val = BIT(2);
			}
			else if(func == AS_CMP){
				val = BIT(3);
			}
		}
		break;



		case GPIO_PB6:
		{
			//0x5ab[5:4]
			//0. SDM_P1
			//1. SPI_DI/I2C_SDA
			//2. UART_RTS
			mask = (unsigned char)~(BIT(5)|BIT(4));
			if(func == AS_SDM){

			}
			else if(func == AS_SPI || func == AS_I2C){
				val = BIT(4);
			}
			else if(func == AS_UART){
				val = BIT(5);
			}
		}
		break;


		case GPIO_PB7:
		{
			//0x5ab[7:6]
			//0. SDM_N1
			//1. SPI_DO
			//2. UART_RX
			mask = (unsigned char)~(BIT(7)|BIT(6));
			if(func == AS_SDM){

			}else if(func == AS_SPI){
				val = BIT(6);
			}
			else if(func == AS_UART){
				val = BIT(7);
			}
		}
		break;


		case GPIO_PC0:
		{
			//0x5ac[1:0]
			//0. I2C_SDA
			//1. PWM4_N
			//2. UART_RTS
			mask = (unsigned char)~(BIT(1)|BIT(0));
			if(func == AS_I2C){

			}else if(func == AS_PWM){
				val = BIT(0);
			}
			else if(func == AS_UART){
				val = BIT(1);
			}
		}
		break;


		case GPIO_PC1:
		{
			//0x5ac[3:2]
			//0. I2C_SCK
			//1. PWM1_N
			//2. PWM0
			mask = (unsigned char)~(BIT(3)|BIT(2));
			if(func == AS_I2C){
			}else if(func == AS_PWM_SECOND){// GPIO_PWM1NC1
				val = BIT(2);
			}
			else if(func == AS_PWM){        // GPIO_PWM0C1
				val = BIT(3);
			}
		}
		break;


		case GPIO_PC2:
		{
			//0x5ac[5:4]
			//0. PWM0
			//1. 7816_TRX
			//2. I2C_SDA
			mask = (unsigned char)~(BIT(5)|BIT(4));
			if(func == AS_PWM){

			}
			else if(func == AS_UART){
				val = BIT(4);
			}
			else if(func == AS_I2C){
				val = BIT(5);
			}
		}
		break;


		case GPIO_PC3:
		{
			//0x5ac[7:6]
			//0. PWM1
			//1. UART_RX
			//2. I2C_SCK
			mask = (unsigned char)~(BIT(7)|BIT(6));
			if(func == AS_PWM){

			}else if(func == AS_UART){
				val = BIT(6);
			}
			else if(func == AS_I2C){
				val = BIT(7);
			}
		}
		break;


		case GPIO_PC4:
		{
			//0x5ad[1:0]
			//0. PWM2
			//1. UART_CTS
			//2. PWM0_N
			mask = (unsigned char)~(BIT(1)|BIT(0));
			if(func == AS_PWM){             // GPIO_PWM2C4

			}else if(func == AS_UART){
				val = BIT(0);
			}
			else if(func == AS_PWM_SECOND){ // GPIO_PWM0NC4
				val = BIT(1);
			}
		}
		break;


		case GPIO_PC5:
		{
			//0x5ad[3:2]
			//0. PWM3_N
			//1. UART_RX
			//2. ATSEL_0
			mask = (unsigned char)~(BIT(3)|BIT(2));
			if(func == AS_PWM){

			}else if(func == AS_UART){
				val = BIT(2);
			}
			else if(func == AS_ATS){
				val = BIT(3);
			}
		}
		break;


		case GPIO_PC6:
		{
			//0x5ad[5:4]
			//0. RX_CYC2LNA
			//1. ATSEL1
			//2. PWM4_N
			mask = (unsigned char)~(BIT(5)|BIT(4));
//			if(func == ){
//
//			}
//			else
			if(func == AS_ATS){
				val = BIT(4);
			}
			else if(func == AS_PWM){
				val = BIT(5);
			}

		}
		break;


		case GPIO_PC7:
		{
			//0x5ad[7:6]
			//0. TX_CYC2PA
			//1. ATSEL2
			//2. PWM5_N
			mask = (unsigned char)~(BIT(7)|BIT(6));
//			if(func == ){
//
//			}
//			else
			if(func == AS_ATS){
				val = BIT(6);
			}
			else if(func == AS_PWM){
				val = BIT(7);
			}
		}
		break;


		case GPIO_PD0:
		{
			//0x5ae[1:0]
			//0. RX_CYC2LNA
			//1. CMP_D
			//2. 7816_TRX
			mask = (unsigned char)~(BIT(1)|BIT(0));
//			if(func == ){
//
//			}
//			else
			if(func == AS_CMP){
				val = BIT(0);
			}
			else if(func == AS_UART){
				val = BIT(1);
			}
		}
		break;


		case GPIO_PD1:
		{
			//0x5ae[3:2]
			//0. TX_CYC2PA
			//1. CMP_DF
			//2. UART_CTS
			mask = (unsigned char)~(BIT(3)|BIT(2));
//			if(func == ){
//
//			}
//			else
			if(func == AS_CMP){
				val = BIT(0);
			}
			else if(func == AS_UART){
				val = BIT(2);
			}
		}
		break;


		case GPIO_PD2:
		{
			//0x5ae[5:4]
			//0. SPI_CN
			//1. I2S_LR
			//2. PWM3
			mask = (unsigned char)~(BIT(5)|BIT(4));
			if(func == AS_SPI){

			}
			else if(func == AS_I2S){
				val = BIT(4);
			}
			else if(func == AS_PWM){
				val = BIT(5);
			}
		}
		break;


		case GPIO_PD3:
		{
			//0x5ae[7:6]
			//0. PWM1_N
			//1. I2S_SDI
			//2. UART_TX��7816��
			mask = (unsigned char)~(BIT(7)|BIT(6));
			if(func == AS_PWM){

			}
			else if(func == AS_I2S){
				val = BIT(6);
			}
			else if(func == AS_UART){
				val = BIT(7);
			}
		}
		break;


		case GPIO_PD4:
		{
			//0x5af[1:0]
			//0. SWM
			//1. I2S_SDO
			//2. PWM2_N
			mask = (unsigned char)~(BIT(1)|BIT(0));
			if(func == AS_SWIRE){

			}
			else if(func == AS_I2S){
				val = BIT(0);
			}
			else if(func == AS_PWM){
				val = BIT(1);
			}
		}
		break;


		case GPIO_PD5:
		{
			//0x5af[3:2]
			//0. PWM0
			//1. CMP_D
			//2. PWM0_N
			mask = (unsigned char)~(BIT(3)|BIT(2));
			if(func == AS_PWM){             // GPIO_PWM0D5

			}
			else if(func == AS_CMP){
				val = BIT(2);
			}
			else if(func == AS_PWM_SECOND){ // GPIO_PWM0ND5
				val = BIT(3);
			}
		}
		break;


		case GPIO_PD6:
		{
			//0x5af[5:4]
			//0. CN
			//1. UART_RX
			//2. ATSEL0
			mask = (unsigned char)~(BIT(5)|BIT(4));
			if(func == AS_SPI){

			}
			else if(func == AS_UART){
				val = BIT(4);
			}
			else if(func == AS_ATS){
				val = BIT(5);
			}
		}
		break;


		case GPIO_PD7:
		{
			//0x5af[7:6]
			//0. SPI_CK/I2C_SCL
			//1. I2S_BCK
			//2. 7816_TRX
			mask = (unsigned char)~(BIT(7)|BIT(6));
			if(func == AS_SPI || func == AS_I2C){

			}
			else if(func == AS_I2S){
				val = BIT(6);
			}
			else if(func == AS_UART){
				val = BIT(7);
			}
		}
		break;


		default:
			break;

	}


	unsigned short reg = 0x5a8 + ((pin>>8)<<1) + ((pin&0x0f0) ? 1 : 0 );
	write_reg8(reg, ( read_reg8(reg) & mask) | val);
}

void gpio_set_func(GPIO_PinTypeDef pin, GPIO_FuncTypeDef func)
{
    if(func > AS_GPIO){
		gpio_config_special_func(pin, func);
    }
    
	unsigned char	bit = pin & 0xff;
	if(func == AS_GPIO){
		BM_SET(reg_gpio_gpio_func(pin), bit);
		return;
	}else{
		BM_CLR(reg_gpio_gpio_func(pin), bit);
	}
}

//debug baseband
void gpio_config_baseband_debug_mode(void)
{
	//1. 5b6[0]: Aura_dbg input mode     0
	//2. 5b6[1]: A[3:0] output BB_dbg    1
	//3. 5b6[2]: PB-PD output BB_dbg	 1
	REG_ADDR8(0x5b6) &= ~BIT(0);
	REG_ADDR8(0x5b6) |= BIT(1);
	REG_ADDR8(0x5b6) |= BIT(2);


	reg_gpio_pa_gpio &= 0xE0; //PA[0..4]   not  gpio
	PAD_FUNC_MUX_1PA = 0xFF;  //PA[0..4] dbg
	PAD_FUNC_MUX_2PA = 0x03;

	reg_gpio_pc_gpio &= 0xF8; //PC[0..2]   not  gpio
	PAD_FUNC_MUX_1PC = 0x3F;  //PC[0..2]   dbg
}

/********************************************************
*
*	@brief		gpio_set_interrupt_init
*
*	@param		pin - gpio.
*				up_down - PM_PIN_PULLUP_10K; PM_PIN_PULLDOWN_100K.
*				falling - 0:rising edge; 1:falling edge.
*				irq_mask - set interrupt mask:FLD_IRQ_GPIO_EN,FLD_IRQ_GPIO_RISC0_EN,FLD_IRQ_GPIO_RISC1_EN,	FLD_IRQ_GPIO_RISC2_EN 
*	@return		None
*/
void gpio_set_interrupt_init(u32 pin, u32 up_down, u32 falling, u32 irq_mask)
{
	if(irq_mask&(~(FLD_IRQ_GPIO_EN|FLD_IRQ_GPIO_RISC0_EN|FLD_IRQ_GPIO_RISC1_EN))){//8258 don't have FLD_IRQ_GPIO_RISC2_EN
		return;
	}
	gpio_core_irq_enable_all(1);  //must
   /***step1. set pin as gpio and enable input********/
	gpio_set_func(pin, AS_GPIO);           //enable GPIO func
	gpio_set_input_en(pin, 1);             //enable input
	gpio_set_output_en(pin, 0);            //disable output
	
	/***step2.      set the polarity and open pullup ***/
	gpio_setup_up_down_resistor(pin, up_down);  //open pull up resistor
	gpio_set_interrupt_pol(pin, falling);    //falling edge
	
	/***step3.      set irq enable  ***/
	reg_irq_src = irq_mask; //clean irq status
	reg_irq_mask |= irq_mask;
	if(FLD_IRQ_GPIO_EN == irq_mask){
		gpio_en_interrupt(pin, 1);
	}
	else if(FLD_IRQ_GPIO_RISC0_EN == irq_mask){
		gpio_en_interrupt_risc0(pin, 1);
	}
	else if(FLD_IRQ_GPIO_RISC1_EN == irq_mask){
		gpio_en_interrupt_risc1(pin, 1);
	}

}

#endif

