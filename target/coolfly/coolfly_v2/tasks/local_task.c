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


cf_rc_s 	sky_rc;
cf_sbus_s  	sky_sbus;

//--------------------------------------
void link_led_process(void);
void link_status_EventHandler(void *p);
void key_bb_match_process(void);

void debug_sbus(void)
{
    static uint64_t t_last;
    uint64_t t = HAL_GetSysUsTick();


    // this task is 20hz = 50ms;
    if(t - t_last < 1000000)
    {
        return;
    }

    t_last = t;

    DLOG_Critical("---------sky_rc----------");
	DLOG_Critical("%d %d %d %d %d %d %d %d",
		      sky_rc.ch[0], sky_rc.ch[1], sky_rc.ch[2], sky_rc.ch[3],
		      sky_rc.ch[4], sky_rc.ch[5], sky_rc.ch[6], sky_rc.ch[7]);

	DLOG_Critical("%d %d %d %d %d %d %d %d",
		      sky_rc.ch[8], sky_rc.ch[9], sky_rc.ch[10], sky_rc.ch[11],
		      sky_rc.ch[12], sky_rc.ch[13], sky_rc.ch[14], sky_rc.ch[15]);
}


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


LINK_LED_STATUS _link_led_status;

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
//      bb_match
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
            _link_led_status = LINK_SEARCH_ID;
        }
    }
    else
    {
        cnt = 0;
    }
        
}



////////////////////////////////////////////////
//   sbus
////////////////////////////////////////////////
// sky used


static void cf_sky_bb_spi_irq_handler(void *p);
static void cf_sbus_parse(void);


void cf_sky_sbus_init(void)
{
	HAL_RET_T ret;

	//init BaseBand rt-uart function
	ret = HAL_BB_SpiDataTransInit(25);

	if (ret != HAL_OK) {
		DLOG_Error("BB SPI init failed");
	}

	//register BaseBand rt-uart session, will malloc a special session to rt-uart function
	ret = HAL_BB_ComRegisterSession(BB_COM_SESSION_SPI,
					BB_COM_SESSION_PRIORITY_HIGH,
					BB_COM_SESSION_DATA_NORMAL,
					cf_sky_bb_spi_irq_handler);

	if (ret != HAL_OK) {
		DLOG_Error("BB ComRegister failed");
	}

}



/**
* @brief  read BaseBand data,then Output from Hal uart.
* @param  data and data len
* @retval date len
* @note   This function only called by sky
*/

static void cf_sky_bb_spi_irq_handler(void *p)
{

#define BB_READ_MAX_LEN 32
	uint32_t cnt;
	uint8_t buffer[BB_READ_MAX_LEN];
	HAL_RET_T ret;
	uint8_t i;

	cnt = 0;
	ret = HAL_BB_ComReceiveMsg(BB_COM_SESSION_SPI, buffer, BB_READ_MAX_LEN, &cnt);

	if (ret != HAL_OK && cnt != 0) {
		DLOG_Error("failed read bbcom %02lx, %ld", ret, cnt);
		return;
	}

	if (cnt > 0 && cnt <= BB_READ_MAX_LEN) {

		if (cnt != 25) {
			DLOG_Error("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], \
				   buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
		}

		for (i = 0; i < 25; i++) {
			sky_sbus.sbus_buff[i] = buffer[i];
		}

		sky_sbus.timestamp = HAL_GetSysUsTick();
		cf_sbus_parse();

	} else {
		DLOG_Info("bb read %ld", cnt);
	}

}

static void cf_sbus_parse(void) // just for test
{
	// check the head & end
	if ((sky_sbus.sbus_buff[0] != 0x0F) || (sky_sbus.sbus_buff[24] != 0x00)) {
		return;
	}

	// parse the sbus
	sky_rc.ch[ 0] = ((uint32_t)sky_sbus.sbus_buff[ 1] >> 0 | ((uint32_t)sky_sbus.sbus_buff[ 2] << 8)) & 0x07FF;
	sky_rc.ch[ 1] = ((uint32_t)sky_sbus.sbus_buff[ 2] >> 3 | ((uint32_t)sky_sbus.sbus_buff[ 3] << 5)) & 0x07FF;
	sky_rc.ch[ 2] = ((uint32_t)sky_sbus.sbus_buff[ 3] >> 6 | ((uint32_t)sky_sbus.sbus_buff[ 4] << 2)  |
			 (uint32_t)sky_sbus.sbus_buff[ 5] << 10) & 0x07FF;
	sky_rc.ch[ 3] = ((uint32_t)sky_sbus.sbus_buff[ 5] >> 1 | ((uint32_t)sky_sbus.sbus_buff[ 6] << 7)) & 0x07FF;
	sky_rc.ch[ 4] = ((uint32_t)sky_sbus.sbus_buff[ 6] >> 4 | ((uint32_t)sky_sbus.sbus_buff[ 7] << 4)) & 0x07FF;
	sky_rc.ch[ 5] = ((uint32_t)sky_sbus.sbus_buff[ 7] >> 7 | ((uint32_t)sky_sbus.sbus_buff[ 8] << 1)  |
			 (uint32_t)sky_sbus.sbus_buff[9] <<  9) & 0x07FF;
	sky_rc.ch[ 6] = ((uint32_t)sky_sbus.sbus_buff[ 9] >> 2 | ((uint32_t)sky_sbus.sbus_buff[10] << 6)) & 0x07FF;
	sky_rc.ch[ 7] = ((uint32_t)sky_sbus.sbus_buff[10] >> 5 | ((uint32_t)sky_sbus.sbus_buff[11] << 3)) & 0x07FF;
	sky_rc.ch[ 8] = ((uint32_t)sky_sbus.sbus_buff[12] << 0 | ((uint32_t)sky_sbus.sbus_buff[13] << 8)) & 0x07FF;
	sky_rc.ch[ 9] = ((uint32_t)sky_sbus.sbus_buff[13] >> 3 | ((uint32_t)sky_sbus.sbus_buff[14] << 5)) & 0x07FF;
	sky_rc.ch[10] = ((uint32_t)sky_sbus.sbus_buff[14] >> 6 | ((uint32_t)sky_sbus.sbus_buff[15] << 2)  |
			 (uint32_t)sky_sbus.sbus_buff[16] << 10) & 0x07FF;
	sky_rc.ch[11] = ((uint32_t)sky_sbus.sbus_buff[16] >> 1 | ((uint32_t)sky_sbus.sbus_buff[17] << 7)) & 0x07FF;
	sky_rc.ch[12] = ((uint32_t)sky_sbus.sbus_buff[17] >> 4 | ((uint32_t)sky_sbus.sbus_buff[18] << 4)) & 0x07FF;
	sky_rc.ch[13] = ((uint32_t)sky_sbus.sbus_buff[18] >> 7 | ((uint32_t)sky_sbus.sbus_buff[19] << 1)  |
			 (uint32_t)sky_sbus.sbus_buff[20] <<  9) & 0x07FF;
	sky_rc.ch[14] = ((uint32_t)sky_sbus.sbus_buff[20] >> 2 | ((uint32_t)sky_sbus.sbus_buff[21] << 6)) & 0x07FF;
	sky_rc.ch[15] = ((uint32_t)sky_sbus.sbus_buff[21] >> 5 | ((uint32_t)sky_sbus.sbus_buff[22] << 3)) & 0x07FF;


	sky_rc.timestamp = sky_sbus.timestamp;

}

