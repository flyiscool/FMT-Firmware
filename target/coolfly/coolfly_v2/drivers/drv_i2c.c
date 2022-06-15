/******************************************************************************
 * Copyright 2020-2021 The Firmament Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/


#include "drv_i2c.h"
#include "hal/i2c/i2c.h"

#include "ar1002_hal.h"
#include "i2c.h"

// #define DRV_DBG(...) console_printf(__VA_ARGS__)
#define DRV_DBG(...)

/* We want to ensure the real-time performace, so the i2c timeout here is
 * relatively short */
#define I2C_TIMEOUT_US (1000)


#ifndef BIT
#define BIT(n)    ((uint32_t)1 << (n))
#endif/* BIT */

struct ar1002_i2c_bus {
    struct rt_i2c_bus parent;
    ENUM_HAL_I2C_COMPONENT I2C;
};


static void put_reg32_mask(uint32_t regAddr, uint32_t regData, uint32_t regDataMask)
{
	uint32_t u32_regDataTmp;

	u32_regDataTmp = *((volatile uint32_t *)regAddr);
	u32_regDataTmp &= ~regDataMask;
	u32_regDataTmp |= regData & regDataMask;

	*((volatile uint32_t *)regAddr) = u32_regDataTmp;
}



extern void cf_delay_ms(uint32_t num);

// i2c2  prevent lock fucntion
static void reset_i2c2(void)
{
	uint8_t i = 0;

	for (i = 0; i < 10; i++) {
		HAL_GPIO_OutPut(120);	// fix the i2c 2  lock bug
		HAL_GPIO_SetPin(120, HAL_GPIO_PIN_RESET);
		cf_delay_ms(10);
		put_reg32_mask(SFR_PAD_CTRL7_REG, 0, BIT(14) | BIT(15) | BIT(16) | BIT(17));
		cf_delay_ms(10);
	}

	HAL_GPIO_OutPut(119);	// fix the i2c 2  lock bug
	HAL_GPIO_SetPin(119, HAL_GPIO_PIN_RESET);
	cf_delay_ms(10);
	HAL_GPIO_OutPut(120);	// fix the i2c 2  lock bug
	HAL_GPIO_SetPin(120, HAL_GPIO_PIN_RESET);
	cf_delay_ms(10);

	put_reg32_mask(SFR_PAD_CTRL7_REG, 0, BIT(16) | BIT(17));
	cf_delay_ms(10);

	put_reg32_mask(SFR_PAD_CTRL7_REG, 0, BIT(14) | BIT(15));
	cf_delay_ms(10);

}



static fmt_err_t wait_TXIS_flag_until_timeout(ENUM_HAL_I2C_COMPONENT I2Cx, uint32_t status, uint32_t timeout)
{
    // uint32_t time_start = systime_now_ms();

    // while (((READ_BIT(I2Cx->ISR, I2C_ISR_TXIS) == I2C_ISR_TXIS) ? 1UL : 0UL) == status) {
    //     /*  TXIS bit is not set when a NACK is received */
    //     if (LL_I2C_IsActiveFlag_NACK(I2Cx)) {
    //         return FMT_ERROR;
    //     }

    //     if ((systime_now_ms() - time_start) > timeout) {
    //         return FMT_ETIMEOUT;
    //     }
    // }
    return FMT_EOK;
}

static fmt_err_t wait_flag_until_timeout(ENUM_HAL_I2C_COMPONENT I2Cx, uint32_t flag, uint32_t status, uint64_t timeout_us)
{
    // uint64_t time_start = systime_now_us();

    // while (((READ_BIT(I2Cx->ISR, flag) == flag) ? 1UL : 0UL) == status) {
    //     if ((systime_now_us() - time_start) > timeout_us) {
    //         return FMT_ETIMEOUT;
    //     }
    // }
    return FMT_EOK;
}

static void i2c_flush_TXDR(ENUM_HAL_I2C_COMPONENT I2Cx)
{
    // /* If a pending TXIS flag is set */
    // /* Write a dummy data in TXDR to clear it */
    // if (LL_I2C_IsActiveFlag_TXIS(I2Cx)) {
    //     LL_I2C_TransmitData8(I2Cx, 0x00);
    // }

    // /* Flush TX register if not empty */
    // if (!LL_I2C_IsActiveFlag_TXE(I2Cx)) {
    //     LL_I2C_ClearFlag_TXE(I2Cx);
    // }
}

static rt_size_t i2c_master_transfer(struct rt_i2c_bus* bus, rt_uint16_t slave_addr, struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    struct rt_i2c_msg* msg;
    uint32_t msg_idx = 0;
    struct ar1002_i2c_bus* ar1002_i2c = (struct ar1002_i2c_bus*)bus;


    // console_println("bus = %d slave_addr = %04x  num = %d \r\n", ar1002_i2c->I2C, slave_addr, num);

    for (msg_idx = 0; msg_idx < num; msg_idx++) {
        msg = &msgs[msg_idx];
        // uint16_t nbytes = msg->len;

        /* TODO: add support for 10bit addr */
        RT_ASSERT(!(msg->flags & RT_I2C_ADDR_10BIT));
        

        if(ar1002_i2c->I2C == HAL_I2C_COMPONENT_2)
        {
            // console_println("msg_idx = %d flags = %d  len = %d ", msg_idx, msg->flags, msg->len);
            
            // for(int i = 0; i< msg->len; i++)
            // {
            //     console_println("i = %d  =  %02x \r\n", i, msg->buf[i]);
            // }
        }

        

        if(msg->len != 1)
        {
            console_println("!!!!! error msg-len msg_idx = %d flags = %d  len = %d \r\n!!!!!", msg_idx, msg->flags, msg->len);
        }


        if (msg->flags & RT_I2C_RD) {
            /* start/restart read operation */


            // LL_I2C_HandleTransfer(ar1002_i2c->I2C, slave_addr, LL_I2C_ADDRESSING_MODE_7BIT, msg->len,
            //     LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_READ);

            // while (nbytes--) {
            //     /* wait data received */
            //     if (wait_flag_until_timeout(ar1002_i2c->I2C, I2C_ISR_RXNE, 0, I2C_TIMEOUT_US) != FMT_EOK) {
            //         DRV_DBG("I2C wait RXNE timeout\n");
            //         goto _stop;
            //     }
            //     /* receive data */
            //     *(msg->buf++) = LL_I2C_ReceiveData8(ar1002_i2c->I2C);
            // }

            // /* Wait transmit complete */
            // if (wait_flag_until_timeout(ar1002_i2c->I2C, I2C_ISR_TC, 0, I2C_TIMEOUT_US) != FMT_EOK) {
            //     DRV_DBG("I2C wait RX TC timeout\n");
            //     goto _stop;
            // }
        } else {

            // only support msg = 1 just for now

            HAL_RET_T ret = HAL_I2C_MasterWriteData(ar1002_i2c->I2C,
						slave_addr >> 1,
						msg->buf,
						msg->len,
						I2C_TIMEOUT_US);
            if(ret != HAL_OK)
            {
                console_println("--------ret = %d  \r\n", ret);
            }
            

        }
    }


    return msg_idx;
}

static const struct rt_i2c_bus_device_ops i2c_bus_ops = {
    i2c_master_transfer,
    RT_NULL,
    RT_NULL
};

// /* i2c bus instances */
// static struct ar1002_i2c_bus ar1002_i2c0 = {
//     .parent.ops = &i2c_bus_ops,
//     .I2C = HAL_I2C_COMPONENT_0
// };

// static struct ar1002_i2c_bus ar1002_i2c1 = {
//     .parent.ops = &i2c_bus_ops,
//     .I2C = HAL_I2C_COMPONENT_1
// };

static struct ar1002_i2c_bus ar1002_i2c2 = {
    .parent.ops = &i2c_bus_ops,
    .I2C = HAL_I2C_COMPONENT_2
};

static struct ar1002_i2c_bus ar1002_i2c3 = {
    .parent.ops = &i2c_bus_ops,
    .I2C = HAL_I2C_COMPONENT_3
};



// /* i2c device instances */
// static struct rt_i2c_device i2c1_dev1 = {
//     .slave_addr = IST8310_ADDRESS, /* 7 bit address */
//     .flags = 0
// };



static struct rt_i2c_device i2c2_dev1 = {
    .slave_addr = IST8310_ADDRESS, /* 7 bit address */
    .flags = 0
};

static struct rt_i2c_device i2c3_dev1 = {
    .slave_addr = NCP5623C_ADDRESS, /* 7 bit address */
    .flags = 0
};

rt_err_t drv_i2c_init(void)
{
    /* i2c low-level initialization */

    reset_i2c2();
    HAL_I2C_MasterInit( ar1002_i2c2.I2C, i2c2_dev1.slave_addr, HAL_I2C_FAST_SPEED);
    
    HAL_I2C_MasterInit( ar1002_i2c3.I2C, i2c3_dev1.slave_addr, HAL_I2C_FAST_SPEED);
    
    /* register i2c bus */
    
    RT_TRY(rt_i2c_bus_device_register(&ar1002_i2c2.parent, "i2c2"));
    RT_TRY(rt_i2c_bus_device_register(&ar1002_i2c3.parent, "i2c3"));

    /* attach i2c devices */

    RT_TRY(rt_i2c_bus_attach_device(&i2c2_dev1, "i2c2_dev1", "i2c2", RT_NULL));
    RT_TRY(rt_i2c_bus_attach_device(&i2c3_dev1, "i2c3_dev1", "i2c3", RT_NULL));

    return RT_EOK;
}
