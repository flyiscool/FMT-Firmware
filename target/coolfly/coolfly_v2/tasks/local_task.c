/******************************************************************************
 * Copyright 2021 The Firmament Authors. All Rights Reserved.
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
#include <firmament.h>

#include "module/task_manager/task_manager.h"
#include "led.h"

fmt_err_t task_local_init(void)
{
    return FMT_EOK;
}

char** str = (char*[]) { "Hello", "C++", "World", NULL };

void task_local_entry(void* parameter)
{
    printf("Hello FMT! This is a local demo task.\n");
    printf("Hello ChuanYun! This is a local demo task.\n");

    struct device_pin_mode r_pin_mode = { HAL_GPIO_NUM61, PIN_MODE_OUTPUT, PIN_OUT_TYPE_PP };
    
    static rt_device_t pin_dev;
    /* configure led pin */
    pin_dev = rt_device_find("pin");
    RT_ASSERT(pin_dev != NULL);

    RT_CHECK(rt_device_open(pin_dev, RT_DEVICE_OFLAG_RDWR));
    led_init(r_pin_mode);
    LED_ON(HAL_GPIO_NUM61);

    // sys_msleep(5000);
    // // test_printf_sector0();

    // rt_device_t dev_id = rt_device_find("mtdblk0");
    // if(dev_id == NULL)
    // {
    //     console_println("can't find mtdblk0");
    // }
    // else
    // {
    //     console_println("find mtd success~ ");

    //     if(rt_device_open(dev_id, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    //     {
    //         console_println("mtd open failed ");
    //     }

    //     console_println("open mtd success~ ");
    
    // }


    while (1) {
        LED_TOGGLE(HAL_GPIO_NUM61);
        sys_msleep(1000);

        // rt_device_read(dev_id, 20, buf, 1);

        // for(int m = 0; m<512; m++)
        // {
        //     buf[m] = m;
        //     console_println(" buf[%d] = %d ", m, buf[m]);
        // }   

        // rt_device_write(dev_id, 20, buf, 1);

    }
}

TASK_EXPORT __fmt_task_desc = {
    .name = "local",
    .init = task_local_init,
    .entry = task_local_entry,
    .priority = 25,
    .stack_size = 1024,
    .param = NULL,
    .dependency = NULL
};
