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
#include "drv_spi.h"
#include "hal/spi/spi.h"
#include "ar1002_hal.h"



struct ar1002_spi_bus {
    struct rt_spi_bus parent;
    ENUM_SPI_COMPONENT SPI;
};


/**
 * @brief Configure spi device
 * 
 * @param device SPI device
 * @param configuration SPI device configuration
 * @return rt_err_t RT_EOK for success
 */
static rt_err_t configure(struct rt_spi_device* device,
    struct rt_spi_configuration* configuration)
{
    // TODO
    return RT_EOK;
}

/**
 * @brief SPI transfer function
 * 
 * @param device SPI device instance
 * @param message SPI message to be transfered
 * @return rt_uint32_t bytes have been transfered
 */
static rt_uint32_t transfer(struct rt_spi_device* device, struct rt_spi_message* message)
{
    struct ar1002_spi_bus* ar1002_spi_bus = (struct ar1002_spi_bus*)device->bus;
    ENUM_SPI_COMPONENT SPI = ar1002_spi_bus->SPI;

    struct rt_spi_message* message_1;
    struct rt_spi_message* message_2;


    // rt_uint32_t size = message->length;
    // console_println("SPI = %d  size = %d ",SPI, size);

    if(message->next == NULL)   //
    {
        message_1 = message;
        // case  juset send one package
        if((message->cs_take == 1) && (message->cs_release == 1) )
        {
            HAL_RET_T ret = HAL_SPI_MasterWriteRead(SPI, (uint8_t *)message_1->send_buf, message_1->length, (uint8_t *)message_1->recv_buf, 0, 100);
            if(ret != HAL_OK)
            {
                console_println("!!! case  juset send one package error");
                return 0;
            }
            else
            {
                return message->length;
            }
        }
    }
    else
    {
        message_1 = message;
        message_2 = message->next;
    }

    // case  send and recv 
    if((message_1->cs_take == 1) && (message_1->cs_release == 0) 
    && (message_2->cs_take == 0) && (message_2->cs_release == 1) )
    {
        HAL_RET_T ret = HAL_SPI_MasterWriteRead(SPI, (uint8_t *)message_1->send_buf, message_1->length, (uint8_t *)message_2->recv_buf, message_2->length, 100);
        if(ret != HAL_OK)
        {
            console_println("!!! // case  send and recv   error");
            return 0;
        }
        else
        {
            return message_2->length;
        }
    }else if((message_1->cs_take == 1) && (message_1->cs_release == 0) 
        && (message_2->cs_take == 1) && (message_2->cs_release == 1) )
    {
        console_println("LIUWEI TODO // case  send and send   error");
    }
    else
    {
        console_println("2---case spi no defined");
    }

    return message->length;
}

static struct rt_spi_ops ar1002_spi_ops = {
    configure,
    transfer
};

/** \brief init and register ar1002 spi bus.
 *
 * \param SPI: AR1002 SPI, e.g: SPI_1,SPI_2,SPI_3.
 * \param ar1002_spi: ar1002 spi bus struct.
 * \param spi_bus_name: spi bus name, e.g: "spi1"
 * \return rt_err_t RT_EOK for success
 */

static rt_err_t ar1002_spi_register(ENUM_SPI_COMPONENT SPI,
    struct ar1002_spi_bus* ar1002_spi,
    const char* spi_bus_name)
{
    if (SPI == SPI_2) {
        ar1002_spi->SPI = SPI_2;

        /* SPI2 configure */
    	STRU_HAL_SPI_INIT st_spiInitInfo = {
    		.u16_halSpiBaudr = 9,
    		.e_halSpiPolarity = HAL_SPI_POLARITY_HIGH,
    		.e_halSpiPhase = HAL_SPI_PHASE_2EDGE,
    	};

    	HAL_SPI_MasterInit(SPI_2, &st_spiInitInfo);

    } else if (SPI == SPI_4) {

        ar1002_spi->SPI = SPI_4;

        /* SPI2 configure */
    	STRU_HAL_SPI_INIT st_spiInitInfo = {
    		.u16_halSpiBaudr = 9,
    		.e_halSpiPolarity = HAL_SPI_POLARITY_HIGH,
    		.e_halSpiPhase = HAL_SPI_PHASE_2EDGE,
    	};

    	HAL_SPI_MasterInit(SPI_4, &st_spiInitInfo);

    } else {
        return RT_ENOSYS;
    }
    return rt_spi_bus_register(&ar1002_spi->parent, spi_bus_name, &ar1002_spi_ops);
}

/**
 * @brief Initialize spi bus and device
 * 
 * @return rt_err_t RT_EOK for success
 */
rt_err_t drv_spi_init(void)
{
    rt_err_t ret;

    /* register SPI bus */
    static struct ar1002_spi_bus ar1002_spi2;


    /* register SPI2 bus */
    ret = ar1002_spi_register(SPI_2, &ar1002_spi2, "spi2");
    if (ret != RT_EOK) {
        return ret;
    }

    /* attach spi_device_1 to spi1 */
    {
        static struct rt_spi_device rt_spi_device_1;

        ret = rt_spi_bus_attach_device(&rt_spi_device_1, "spi2_dev1", "spi2", RT_NULL);
        if (ret != RT_EOK) {
            return ret;
        }
    }

    /* register SPI bus */
    static struct ar1002_spi_bus ar1002_spi4;


    /* register SPI2 bus */
    ret = ar1002_spi_register(SPI_4, &ar1002_spi4, "spi4");
    if (ret != RT_EOK) {
        return ret;
    }

    /* attach spi_device_1 to spi1 */
    {
        static struct rt_spi_device rt_spi_device_1;

        ret = rt_spi_bus_attach_device(&rt_spi_device_1, "spi4_dev1", "spi4", RT_NULL);
        if (ret != RT_EOK) {
            return ret;
        }
    }








    return RT_EOK;
}
