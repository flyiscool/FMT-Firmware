#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "memory_config.h"
#include "ar1002_hal.h"
#include "bb_led.h"

#include "board_device.h"

#include "debuglog.h"

// #include "debuglog.h"
// #include "hal.h"
// #include "usr_usb_task.h"
// #include "usr_wir_core.h"
// #include "hal_sram_ground.h"
// #include "test_hdmi.h"
// #include "data_type.h"
// #include "hal_bb.h"

static LINK_LED_STATUS link_led_status;
static int encode_cnt=0;
static LINK_LED_STATUS link_status;
static LINK_LED_STATUS link_status;

LINK_LED_STATUS get_link_status(void)
{
    return link_status;
}

void set_link_led_status(LINK_LED_STATUS e_link_led_status)
{
    //DLOG_Warning("last %d cur %d",link_led_status,e_link_led_status);
    link_led_status = e_link_led_status;
}

void c201s_lna_bypass(void)
{
    HAL_GPIO_OutPut(LNA_BYPASS_GPIO);
    HAL_GPIO_SetPin(LNA_BYPASS_GPIO,HAL_GPIO_PIN_RESET);
    DLOG_Warning("lna off !");
}

void lna_open(void)
{
	HAL_GPIO_InPut(LNA_BYPASS_GPIO);
    DLOG_Warning("lna on !");
}

void BB_ledLock(void)
{
    HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_RESET);
}

void BB_ledUnlock(void)
{
    HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_SET);
}

void led_videoIsEmpty(void)
{
    HAL_GPIO_SetPin(VIDEO_RED_LED_GPIO, HAL_GPIO_PIN_SET);
}

void led_videoIsFull(void)
{
    static int toggle_flag = 0;

    if(toggle_flag)
    {
        toggle_flag = 0;
        HAL_GPIO_SetPin(VIDEO_RED_LED_GPIO,  HAL_GPIO_PIN_SET);
    }
    else
    {
        toggle_flag = 1;
        HAL_GPIO_SetPin(VIDEO_RED_LED_GPIO,  HAL_GPIO_PIN_RESET);
    }
}

void led_videoIsNormal(void)
{
    HAL_GPIO_SetPin(VIDEO_RED_LED_GPIO,  HAL_GPIO_PIN_RESET);
}
void BB_ledSearchId(void)
{
    static int toggle_flag = 0;

    if(toggle_flag)
    {
        toggle_flag = 0;
        HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_SET );
    }
    else
    {
        toggle_flag = 1;
        HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_RESET );
    }
}
void BB_ledIdNoMatch(void)
{
    static int toggle_flag = 0;
    static uint8_t cnt = 0;

    cnt++;

    if(cnt%4 == 0)
    {
        if(toggle_flag)
        {
            toggle_flag = 0;
            HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_SET );
        }
        else
        {
            toggle_flag = 1;
            HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_RESET );
        }
    }
}

void led_enter_sleep(void)
{
    HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_SET);
    HAL_GPIO_SetPin(VIDEO_RED_LED_GPIO,  HAL_GPIO_PIN_SET);
    link_led_status = LINK_UNLOCK;

}
void BB_ledGpioInit(void)
{
    HAL_GPIO_SetMode(LINK_LED_GPIO, HAL_GPIO_PIN_MODE2);
    HAL_GPIO_OutPut(LINK_LED_GPIO);

    HAL_GPIO_SetPin(LINK_LED_GPIO, HAL_GPIO_PIN_SET);
    link_led_status = LINK_UNLOCK;
    BB_ledUnlock();

    HAL_GPIO_SetMode(VIDEO_RED_LED_GPIO, HAL_GPIO_PIN_MODE2);
    HAL_GPIO_OutPut(VIDEO_RED_LED_GPIO);


    HAL_GPIO_SetPin(VIDEO_RED_LED_GPIO,  HAL_GPIO_PIN_SET);
    led_videoIsEmpty();
}

void BB_EventHandler(void *p)
{
    DLOG_Critical("BB_EventHandler~~");
    STRU_SysEvent_DEV_BB_STATUS *pstru_status = (STRU_SysEvent_DEV_BB_STATUS *)p;

    if (pstru_status->pid == BB_LOCK_STATUS)
    {
        if (pstru_status->lockstatus == 3)
        {
            link_led_status = LINK_LOCK;
            DLOG_Info("------> lock");
        }
        else
        {
            if(link_led_status != LINK_SEARCH_ID)
            {
                link_led_status = LINK_UNLOCK;
            }
            DLOG_Info("------> unlock");
        }
    }
    else if(pstru_status->pid == BB_GOT_ERR_CONNNECT)
       {        
        if (pstru_status->lockstatus)        
        {           
            DLOG_Info("Got mismatch signal");
            if(link_led_status != LINK_SEARCH_ID)
            {
                link_led_status = LINK_ID_NO_MATCH;
            }
        }       
        else        
        {            
            DLOG_Info("not got signal");     
            //link_led_status = LINK_UNLOCK;
        }    
       }

}

void ENCODE_EventHandler(void *p)
{
    encode_cnt++;
}
static LED_VIDEO_STATUS led_video_state = V_INVALID;

LED_VIDEO_STATUS get_video_state(void)
{
    return led_video_state;
}

uint32_t get_encoding_status(void)
{

      return *((uint32_t *)(0xa00100e0));
}

#define VIDEO_BUF_MAX_LEN ((1<<20))
#define VIDEO_FULL_VALUE_THRESHOLD ((1<<18))//buf leave 1/4


int check_encode_status(void)
{
	static int last_cnt = 0,cnt=0;

	STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);
    encode_cnt = osdptr->enc_running_cnt;

	if(last_cnt == encode_cnt)
	{
        cnt++;
	}
	else
	{
		last_cnt  = encode_cnt;
		cnt = 0;
	}

	if(cnt > 4)
	{
		return 1;//encode stop
	}
    else if(cnt == 0)
	{
		return 2;//encode ok
	}

	return 0;


}


// void sky_led_video_process(void)
// {
//     static int empty_cnt = 0,full_cnt = 0,normal_cnt = 0;
//     static uint32_t last_len=0,cur_len=0;
//     int len;
//     uint8_t hdmi_ok;
//     HAL_RET_T ret;
//     //static uint32_t last_en,cur_en;

//     // hdmi_ok = get_hdmi_status();
//     hdmi_ok = 0;
//     // cur_len = get_usb_recv_size0();
//     cur_len = 0;
//     if(get_vedio_deep(&len)< 0)
//     {
//         return;
//     }

//     if(hdmi_ok)
//     {
//         if(check_encode_status() != 2)
//         {
//             empty_cnt++;
//             full_cnt = 0;
//             normal_cnt = 0;
//         }
// 		else if(len < VIDEO_FULL_VALUE_THRESHOLD)
//         {
//             full_cnt++;
//             empty_cnt = 0;
//             normal_cnt = 0;
//         }
//         else
//         {
//             normal_cnt++;
//             empty_cnt = 0;
//             full_cnt = 0;
//         }
//     }
//     else
//     {
//         if(cur_len == last_len)
//         {
//             empty_cnt++;
//             full_cnt = 0;
//             normal_cnt = 0;
//         }
//         else if(len < VIDEO_FULL_VALUE_THRESHOLD)
//         {
//             full_cnt++;
//             empty_cnt = 0;
//             normal_cnt = 0;
//         }
//         else
//         {
//             normal_cnt++;
//             empty_cnt = 0;
//             full_cnt = 0;
//         }
//     }

//     last_len = cur_len;
    
//     if(link_led_status != LINK_LOCK)
//     {
//         empty_cnt = 6;//force to video display error
//     }

//     if(empty_cnt > 3)
//     {
//         empty_cnt = 0;
//         led_video_state = V_EMPTY;
//         led_videoIsEmpty();
//     }
//     else if(full_cnt > 3)
//     {
//         full_cnt = 0;
//         led_video_state = V_FULL;
//         led_videoIsFull();
//     }
//     else if(normal_cnt > 0)
//     {
//         normal_cnt = 0;
//         led_video_state = V_NORMAL;
//         led_videoIsNormal();
//     }
// }


void led_link_process(void)
{
    STRU_DEVICE_INFO p;
    STRU_DEVICE_INFO       *pstDeviceInfo =(STRU_DEVICE_INFO*) &p;
    //if bb not locked , to notice spi user data bypass not output data

    if(HAL_BB_GetDeviceInfo(&pstDeviceInfo) != HAL_OK)
    {
        DLOG_Error("failed");
        return;
    }
        
    if(pstDeviceInfo->inSearching) //0,un search, 1 , searching
    {
        BB_ledSearchId();
        link_status = LINK_SEARCH_ID;
        return;
    }
    link_status = link_led_status;
    if(link_led_status == LINK_UNLOCK)
    {
        BB_ledUnlock();
    }
    else if(link_led_status == LINK_LOCK)
    {
        BB_ledLock();
    }
    else if(link_led_status == LINK_SEARCH_ID)
    {
        BB_ledSearchId();
    }
    else if(link_led_status == LINK_ID_NO_MATCH)
    {
        BB_ledIdNoMatch();
    }
    DLOG_Critical("link_led_status = %d ", link_led_status);
}
