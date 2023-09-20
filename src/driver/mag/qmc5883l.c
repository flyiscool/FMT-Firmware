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

//#define DRV_DBG(...)              console_printf(__VA_ARGS__)
#define DRV_DBG(...)

// #define QMC5883_7BITI2C_ADDRESS		0x30

#define QMC5883_PRODUCT_ID        0xFF

// 18-bit mode, null field output (32768)
#define QMC5883_16BIT_OFFSET      32768
#define QMC5883_16BIT_SENSITIVITY 12000

static rt_device_t i2c_dev;

RT_WEAK void qmc5883l_user_calibrate(float data[3]);

/* Re-implement this function to define customized rotation */
RT_WEAK void qmc5883l_rotate_to_frd(float* data)
{
    /* do nothing */
}

static rt_err_t mag_raw_measure(float* raw)
{
    uint8_t  data_reg[6]  = { 0 };
    uint32_t data18bit[3] = { 0 };

    /* Read register data */
    RT_TRY(i2c_read_regs(i2c_dev, 0x00, data_reg, sizeof(data_reg)));

    /* Get 18bits data, raw data unit is "count or LSB" */
    data18bit[0] = (uint32_t)(data_reg[1] << 8 | data_reg[0]);
    data18bit[1] = (uint32_t)(data_reg[3] << 8 | data_reg[2]);
    data18bit[2] = (uint32_t)(data_reg[5] << 8 | data_reg[4]);

    /* Magnetic field output, unit is Gauss */
    raw[0] = ((float)data18bit[0] - QMC5883_16BIT_OFFSET) / QMC5883_16BIT_SENSITIVITY;
    raw[1] = ((float)data18bit[1] - QMC5883_16BIT_OFFSET) / QMC5883_16BIT_SENSITIVITY;
    raw[2] = ((float)data18bit[2] - QMC5883_16BIT_OFFSET) / QMC5883_16BIT_SENSITIVITY;

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

        RT_TRY(i2c_read_reg(i2c_dev, 0x0D, &value));

        // read WHO_AM_I value
        if (value == QMC5883_PRODUCT_ID) {
            console_printf("get QMC5883!!!!ID =  %02x \r\n", value);
            return RT_EOK;
        } else {
            console_printf("-----------MAG ID = %02x !!!\r\n", value);
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

    RT_CHECK(i2c_write_reg(i2c_dev, 0x0B, 0x01));
    RT_CHECK(i2c_write_reg(i2c_dev, 0x20, 0x40));
    RT_CHECK(i2c_write_reg(i2c_dev, 0x21, 0x01));
    RT_CHECK(i2c_write_reg(i2c_dev, 0x09, 0x0D));

    uint8_t value = 0;
    do {
        sys_msleep(20);
        i2c_read_reg(i2c_dev, 0x09, &value);
        console_printf("QMC5883 Init ing...\r\n");
    } while (value != 0x0D);

    console_printf("QMC5883 Init success !!!\r\n");
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
        .ops      = &__mag_ops,
        .config   = NULL,
        .bus_type = MAG_I2C_BUS_TYPE
    };

    i2c_dev = rt_device_find(i2c_device_name);

    RT_ASSERT(i2c_dev != NULL);

    RT_CHECK(rt_device_open(i2c_dev, RT_DEVICE_OFLAG_RDWR));

    RT_TRY(qmc5883l_init());

    RT_CHECK(hal_mag_register(&mag_dev, mag_device_name, RT_DEVICE_FLAG_RDWR, RT_NULL));

    return RT_EOK;
}
