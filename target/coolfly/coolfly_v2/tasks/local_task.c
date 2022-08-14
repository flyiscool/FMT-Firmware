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
#include "debuglog.h"
#include "sys_event.h"
#include "bb_match_id.h"
#include "ar1002_hal.h"
#include "board_device.h"

#include "local_task.h"
#include "bb_led.h"
#include "sky_sbus.h"

//--------------------------------------
void link_led_process(void);


fmt_err_t task_local_init(void)
{
    return FMT_EOK;
}


void task_local_entry(void* parameter)
{

    sbus_start();
    bb_led_start();
    bb_match_id_start();

    while (1) {
        DLOG_Process(NULL);
        sys_msleep(10); 
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

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////




