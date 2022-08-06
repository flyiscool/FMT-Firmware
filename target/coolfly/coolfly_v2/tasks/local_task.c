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

#include "board_device.h"

//--------------------------------------
void link_led_process(void);
void link_status_EventHandler(void *p);
void key_bb_match_process(void);

fmt_err_t task_local_init(void)
{

    HAL_GPIO_InPut(EXTERN_SEARCH_ID_PIN);

    return FMT_EOK;
}

void task_local_entry(void* parameter)
{

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_EVENT, link_status_EventHandler);
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_EVENT, BB_skyRcIdEventHandler);

    while (1) {
        key_bb_match_process();
        link_led_process();
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


static void link_led_fasttoggle(void)
{
    static uint8_t toggle_flag = 0;

    if(toggle_flag)
    {
        toggle_flag = 0;
        HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_SET);
    }
    else
    {
        toggle_flag = 1;
        HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_RESET);
    }
}

static void link_led_slowtoggle(void)
{
    static uint8_t cnt = 0;
    cnt++;
    if(cnt%4 == 0)
    {
        link_led_fasttoggle();
    }
}


static void link_led_on(void)
{
    HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_RESET);
}

static void link_led_off(void)
{
    HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_SET);
}



static LINK_LED_STATUS _link_led_status;

void link_status_EventHandler(void *p)
{
    STRU_SysEvent_DEV_BB_STATUS *pstru_status = (STRU_SysEvent_DEV_BB_STATUS *)p;

    if (pstru_status->pid == BB_LOCK_STATUS)
    {
        if (pstru_status->lockstatus == 3)
        {
            _link_led_status = LINK_LOCK;
            DLOG_Info("------> lock");
        }
        else
        {
            if(_link_led_status != LINK_SEARCH_ID)
            {
                _link_led_status = LINK_UNLOCK;
            }
            DLOG_Info("------> unlock");
        }
    }
    else if(pstru_status->pid == BB_GOT_ERR_CONNNECT)
    {        
        if (pstru_status->lockstatus)        
        {           
            DLOG_Info("Got mismatch signal");
            if(_link_led_status != LINK_SEARCH_ID)
            {
                _link_led_status = LINK_ID_NO_MATCH;
            }
        }       
        else        
        {            
            DLOG_Info("not got signal");     
        }    
    }
}


void link_led_process(void)
{
    static uint64_t t_last;
    uint64_t t = HAL_GetSysUsTick();


    // this task is 20hz = 50ms;
    if(t - t_last < 50000)
    {
        return;
    }

    t_last = t;


    ////////////////////////////////
    STRU_DEVICE_INFO p;
    STRU_DEVICE_INFO       *pstDeviceInfo =(STRU_DEVICE_INFO*) &p;
    
    if(HAL_BB_GetDeviceInfo(&pstDeviceInfo) != HAL_OK)
    {
        DLOG_Critical("failed");
        return;
    }

    if(pstDeviceInfo->inSearching) //0,un search, 1 , searching
    {
        link_led_fasttoggle();
        return;
    }

    // link_status = _link_led_status;
    if(_link_led_status == LINK_UNLOCK)
    {
        link_led_off();
    }
    else if(_link_led_status == LINK_LOCK)
    {
        link_led_on();
    }
    else if(_link_led_status == LINK_SEARCH_ID)
    {
        link_led_fasttoggle();
    }
    else if(_link_led_status == LINK_ID_NO_MATCH)
    {
        link_led_slowtoggle();
    }
}



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void key_bb_match_process(void)
{
    static uint64_t t_last;
    uint64_t t = HAL_GetSysUsTick();


    // this task is 5hz = 200ms;
    if(t - t_last < 200000)
    {
        return;
    }

    t_last = t;



    //
    static uint8_t cnt = 0;
    static uint32_t pin_value;

    HAL_GPIO_GetPin(EXTERN_SEARCH_ID_PIN, &pin_value);
    if(pin_value == 0)
    {
        cnt++;
        if(cnt >= 10)
        {
            cnt = 0;
            DLOG_Warning("pin search id start");
            BB_Sky_SearchIdHandler(NULL);
            set_link_led_status(LINK_SEARCH_ID);
        }
    }
    else
    {
        cnt = 0;
    }
        
}
