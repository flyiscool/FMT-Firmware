/******************************************************************************
 * Copyright 2022 The Firmament Authors. All Rights Reserved.
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
#include "driver/mag/qmc5883l.h"
#include "hal/i2c/i2c.h"
#include "hal/i2c/i2c_dev.h"
#include "hal/mag/mag.h"

#define DRV_DBG(...)                   console_printf(__VA_ARGS__)
// #define DRV_DBG(...)

// #define QMC5883_7BITI2C_ADDRESS		0x30

#define QMC5883_PRODUCT_ID 0xFF

#define QMC5883_REG_DATA       0x00
#define QMC5883_REG_TEMP       0x07
#define QMC5883_REG_STATUS     0x08
#define QMC5883_REG_CTRL0      0x09
#define QMC5883_REG_CTRL1      0x0A
#define QMC5883_REG_CTRL2      0x0B
#define QMC5883_REG_CTRL3      0x0C
#define QMC5883_REG_CHIPID1    0x0D

/* Bit definition for status register 0x08 */
#define QMC5883_MM_DONE       0x01
#define QMC5883_MT_DONE       0x02
#define QMC5883_OTP_READ_DONE 0x10

/* Bit definition for control register 0 0x09 */
#define QMC5883_CMD_TMM        0x01
#define QMC5883_CMD_TMT        0x02
#define QMC5883_CMD_INT_MD_EN  0x04
#define QMC5883_CMD_SET        0x08
#define QMC5883_CMD_RESET      0x10
#define QMC5883_CMD_AUTO_SR_EN 0x20
#define QMC5883_CMD_OTP_READ   0x40

/* Bit definition for control register 1 0x0A */
#define QMC5883_CMD_BW00      0x00
#define QMC5883_CMD_BW01      0x01
#define QMC5883_CMD_BW10      0x02
#define QMC5883_CMD_BW11      0x03
#define QMC5883_CMD_X_INHIBIT 0x04
#define QMC5883_CMD_Y_INHIBIT 0x08
#define QMC5883_CMD_Z_INHIBIT 0x10
#define QMC5883_CMD_SW_RST    0x80

/* Bit definition for control register 2 0x0B */
#define QMC5883_CMD_CM_FREQ_OFF    0x00
#define QMC5883_CMD_CM_FREQ_1HZ    0x01
#define QMC5883_CMD_CM_FREQ_10HZ   0x02
#define QMC5883_CMD_CM_FREQ_20HZ   0x03
#define QMC5883_CMD_CM_FREQ_50HZ   0x04
#define QMC5883_CMD_CM_FREQ_100HZ  0x05
#define QMC5883_CMD_CM_FREQ_200HZ  0x06
#define QMC5883_CMD_CM_FREQ_1000HZ 0x07
#define QMC5883_CMD_CMM_EN         0x08

#define QMC5883_CMD_PART_SET1    0x00
#define QMC5883_CMD_PART_SET25   0x10
#define QMC5883_CMD_PART_SET75   0x20
#define QMC5883_CMD_PART_SET100  0x30
#define QMC5883_CMD_PART_SET250  0x40
#define QMC5883_CMD_PART_SET500  0x50
#define QMC5883_CMD_PART_SET1000 0x60
#define QMC5883_CMD_PART_SET2000 0x70
#define QMC5883_CMD_EN_PART_SET  0x80

//18-bit mode, null field output (32768)
#define QMC5883_16BIT_OFFSET      32768
#define QMC5883_16BIT_SENSITIVITY 4096

#define QMC5883_18BIT_OFFSET      131072
#define QMC5883_18BIT_SENSITIVITY 16384

#define QMC5883_T_ZERO        (-75)
#define QMC5883_T_SENSITIVITY (0.8)

static rt_device_t i2c_dev;

RT_WEAK void qmc5883l_user_calibrate(float data[3]);

/* Re-implement this function to define customized rotation */
RT_WEAK void qmc5883l_rotate_to_frd(float* data)
{
    /* do nothing */
}

static rt_err_t mag_raw_measure(float* raw)
{
    uint8_t data_reg[7] = { 0 };
    uint32_t data18bit[3] = { 0 };

    /* Read register data */
    RT_TRY(i2c_read_regs(i2c_dev, QMC5883_REG_DATA, data_reg, sizeof(data_reg)));

    /* Get 18bits data, raw data unit is "count or LSB" */
    data18bit[0] = (uint32_t)(data_reg[0] << 10 | data_reg[1] << 2 | (data_reg[6] & 0xC0) >> 6);
    data18bit[1] = (uint32_t)(data_reg[2] << 10 | data_reg[3] << 2 | (data_reg[6] & 0x30) >> 4);
    data18bit[2] = (uint32_t)(data_reg[4] << 10 | data_reg[5] << 2 | (data_reg[6] & 0x0C) >> 2);

    /* Magnetic field output, unit is Gauss */
    raw[0] = ((float)data18bit[0] - QMC5883_18BIT_OFFSET) / QMC5883_18BIT_SENSITIVITY;
    raw[1] = ((float)data18bit[1] - QMC5883_18BIT_OFFSET) / QMC5883_18BIT_SENSITIVITY;
    raw[2] = ((float)data18bit[2] - QMC5883_18BIT_OFFSET) / QMC5883_18BIT_SENSITIVITY;

    return RT_EOK;
}

static rt_err_t mag_measure(float* mag)
{
    float mag_raw[3];

    /* get the QMC5883 data, unit is gauss */
    RT_TRY(mag_raw_measure(mag_raw));

    // correct coordinate according to datasheet
    mag_raw[2] = -mag_raw[2];

    mag[0] = mag_raw[0];
    mag[1] = mag_raw[1];
    mag[2] = mag_raw[2];

    qmc5883l_rotate_to_frd(mag);

    return RT_EOK;
}

static rt_err_t probe(void)
{
    uint8_t tries = 5;
    uint8_t value;

    sys_msleep(200);
    while (tries) {


        RT_TRY(i2c_read_reg(i2c_dev, QMC5883_REG_CHIPID1, &value));

        // read WHO_AM_I value
        if (value == QMC5883_PRODUCT_ID) {
            console_printf("get QMC5883!!!!ID =  %02x \r\n",value);
            return RT_EOK;
        }
        else{
            console_printf("-----------MAG ID = %02x !!!\r\n",value);
        }

        sys_msleep(10);
        tries--;
    }

    return RT_ERROR;
}

static rt_err_t qmc5883l_init(void)
{
    /* check if device connected */
    RT_TRY(probe());

    /*Work mode setting*/
    /* Write reg 0x0A */
    /* Set BW<1:0> = bandwith

		BW1	BW0	Measurement Time	Bandwidth
		0	0		8ms				100Hz
		0	1		4ms				200Hz
		1	0		2ms				400Hz
		1	1		0.5ms			800Hz
	*/
    RT_CHECK(i2c_write_reg(i2c_dev, QMC5883_REG_CTRL1, QMC5883_CMD_BW00));
    /* Write reg 0x09 */
    /* Set Auto_SR_en bit '1', Enable the function of automatic set/reset */
    RT_CHECK(i2c_write_reg(i2c_dev, QMC5883_REG_CTRL0, QMC5883_CMD_AUTO_SR_EN));

    /* Write reg 0x0B */
    /* Set Cmmm_en bit '1', Enable the continuous mode */
    /* Set CM_Freq<2:0> = sampling_rate


				001				1 Hz
				010				10 Hz
				011				20 Hz
				100				50 Hz
				101				100 Hz
				110				200 Hz
				111				1000 Hz
	*/
    RT_CHECK(i2c_write_reg(i2c_dev, QMC5883_REG_CTRL2, QMC5883_CMD_CMM_EN | QMC5883_CMD_CM_FREQ_100HZ));
    sys_msleep(20);

    return RT_EOK;
}

static rt_size_t qmc5883l_read(mag_dev_t mag, rt_off_t pos, void* data, rt_size_t size)
{
    if (data == RT_NULL) {
        return 0;
    }

    if (mag_measure(((float*)data)) != RT_EOK) {
        return 0;
    }

    return size;
}

const static struct mag_ops __mag_ops = {
    NULL,
    NULL,
    qmc5883l_read,
};

rt_err_t drv_qmc5883l_init(const char* i2c_device_name, const char* mag_device_name)
{
    static struct mag_device mag_dev = {
        .ops = &__mag_ops,
        .config = NULL,
        .bus_type = MAG_I2C_BUS_TYPE
    };

    i2c_dev = rt_device_find(i2c_device_name);

    RT_ASSERT(i2c_dev != NULL);

    RT_CHECK(rt_device_open(i2c_dev, RT_DEVICE_OFLAG_RDWR));

    RT_TRY(qmc5883l_init());

    RT_CHECK(hal_mag_register(&mag_dev, mag_device_name, RT_DEVICE_FLAG_RDWR, RT_NULL));

    return RT_EOK;
}
