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
#include "bb_led.h"
#include "sys_event.h"
#include "bb_match_id.h"
#include "bb_led.h"
#include "ar1002_hal.h"


extern uint32_t flag_searchIdTimerStart;
extern uint8_t vt_id_timer_start_flag;


// void BB_skyRcIdEventHandler(void *p)
// {
//     STRU_SysEvent_DEV_BB_STATUS *pstru_status = (STRU_SysEvent_DEV_BB_STATUS *)p;
//     uint8_t id[7];

//     if(pstru_status->pid == BB_GET_RCID)
//     {
//         DLOG_Critical("Get rcid: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x,rssi_a %d, rssi_b %d", pstru_status->rcid[0], pstru_status->rcid[1], 
//                                                                        pstru_status->rcid[2], pstru_status->rcid[3], pstru_status->rcid[4],
//                                                                        pstru_status->agc1,pstru_status->agc2);
//     }

//     if (pstru_status->pid == BB_SKY_SEARCHING_STATES_CHAGE)
//     {
//         if (SKY_WAIT_VT_ID_MSG == pstru_status->e_sky_searchState)
//         {
//             DLOG_Warning("search id: SKY_WAIT_VT_ID_MSG");
//         }
//         else if (SKY_WAIT_RC_ID_MATCH == pstru_status->e_sky_searchState)
//         {
//             memcpy(id,pstru_status->vtid,2);
//             memcpy(id+2,pstru_status->rcid,5);
//             add_dev_info(id,pstru_status->agc1,pstru_status->agc2);
            
//             if(!vt_id_timer_start_flag)
//             {
//                 vt_id_timer_start_flag = 1;
//                 HAL_TIMER_Start(GET_VT_ID_TIMER);
//                 DLOG_Warning("got vtid: %dus timer start",GET_VT_ID_TIMEOUT);
//             }
//             //DLOG_Warning("search id: SKY_WAIT_RC_ID_MATCH");
//         }
//         else if (SKY_RC_SEARCH_END == pstru_status->e_sky_searchState)
//         {
//             DLOG_Warning("search id: SKY_RC_SEARCH_END rc id: %x %x %x %x %x; vt id: %x %x", 
//                           pstru_status->rcid[0], pstru_status->rcid[1], pstru_status->rcid[2], pstru_status->rcid[3], pstru_status->rcid[4],
//                           pstru_status->vtid[0], pstru_status->vtid[1]);

//             if (flag_searchIdTimerStart)
//             {
//                 HAL_TIMER_Stop(SEARCH_ID_TIMER);
//                 flag_searchIdTimerStart = 0;
//                 vt_id_timer_start_flag = 0;
//             }
//         }
//     }
// }

fmt_err_t task_local_init(void)
{

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_EVENT, BB_skyRcIdEventHandler);
    return FMT_EOK;
}

void task_local_entry(void* parameter)
{
    printf("Hello FMT! This is a local demo task.\n");
    printf("Hello ChuanYun! This is a local demo task.\n");

    while (1) {
        // sky_led_video_process();
        led_link_process();
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
