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

#include "sys_event.h"
// #include "bb_led.c"

fmt_err_t task_sys_event_process_init(void)
{
    return FMT_EOK;
}

void task_sys_event_process_entry(void* parameter)
{
    printf("Hello ChuanYun! This is a sys_event_process demo task.\n");

    // SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_EVENT, BB_EventHandler);
    // SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_EVENT, BB_skyRcIdEventHandler);
    // SYS_EVENT_RegisterHandler(SYS_EVENT_ID_USER_CFG_CHANGE, BB_ReplyPcHandler);
    // SYS_EVENT_RegisterHandler(SYS_EVENT_ID_TO_APP, Sky_AppSysEventHandler);
    // SYS_EVENT_RegisterHandler(SYS_EVENT_ID_USER_DEFINE, USER_Define_EventHandler);

    while (1) {
        SYS_EVENT_Process();
        sys_msleep(50);
    }
}

TASK_EXPORT __fmt_task_desc = {
    .name = "sys_event_process",
    .init = task_sys_event_process_init,
    .entry = task_sys_event_process_entry,
    .priority = COOLFLY_InterCore,
    .stack_size = 2048,
    .param = NULL,
    .dependency = NULL
};
