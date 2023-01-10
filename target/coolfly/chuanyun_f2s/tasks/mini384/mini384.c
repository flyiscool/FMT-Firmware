#include "debuglog.h"
#include <stdlib.h>

#include "ar1002_chip.h"
#include "ar1002_hal.h"

#include "mini384.h"

#include "module/workqueue/workqueue_manager.h"

#define MINI384_DVP_CHANNEL 1
#define MINI384_RST         HAL_GPIO_NUM103
#define MINI384_I2C         HAL_I2C_COMPONENT_0
#define MINI384_I2C_ADDR    0x3C

#define reg1_640x512 0x009D
uint8_t config1_640x512_30fps[8] = { 0x0F, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08 };
#define reg2_640x512 0x081D
uint8_t config2_640x512_30fps[8] = { 0x00, 0x00, 0x02, 0x80, 0x02, 0x00, 0x1E, 0x00 };

#define reg1_384x288 0x009D
uint8_t config1_384x288_30fps[8] = { 0x0F, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08 };
#define reg2_384x288 0x081D
uint8_t config2_384x288_30fps[8] = { 0x00, 0x00, 0x01, 0x80, 0x01, 0x20, 0x1E, 0x00 };

static volatile int is_Inited = 0;

_EXT_DTCM1
static void mini384_init(void)
{
    DLOG_Critical("mini384_init");

    // try to start device
    uint8_t tries = 5;

    HAL_I2C_MasterInit(MINI384_I2C, MINI384_I2C_ADDR, HAL_I2C_FAST_SPEED);
    HAL_Delay(1000);

    while (tries) {

        uint8_t value1 = 0xFF;
        uint8_t value2 = 0xFF;

        uint8_t reg1[2] = { 0x00, 0x00 };

        HAL_RET_T ret = HAL_I2C_MasterReadData(MINI384_I2C, MINI384_I2C_ADDR, reg1, 2, &value1, 1, 1000);

        if (ret != HAL_OK) {
            DLOG_Critical("ret =%08x !", ret);
        }

        uint8_t reg2[2] = { 0x00, 0x01 };
        ret = HAL_I2C_MasterReadData(MINI384_I2C, MINI384_I2C_ADDR, reg2, 2, &value2, 1, 1000);

        if (ret != HAL_OK) {
            DLOG_Critical("ret =%08x !", ret);
        }

        DLOG_Critical("value1 =%02x value2 = %02x !", value1, value2);

        // read WHO_AM_I value
        if ((value1 == 0x53) && (value2 == 0x52)) {
            break;
        }

        HAL_Delay(10);
        tries--;
    }

    if (!tries) {
        DLOG_Critical("can't find mini384 !");
        return;
    }

    tries = 5;

    DLOG_Critical("mini384 init success~!");

    HAL_Delay(20);

    uint8_t config1_readid[10] = { 0x1d, 0x00, 0x05, 0x84, 0x04, 0x00, 0x00, 0x04, 0x00, 0x04 };
    uint8_t reg_readid[2] = { 0x1d, 0x08 };
    uint8_t config2_readid[4];

    HAL_I2C_MasterWriteData(MINI384_I2C, MINI384_I2C_ADDR, config1_readid, 10, 1000);
    HAL_RET_T ret = HAL_I2C_MasterReadData(MINI384_I2C, MINI384_I2C_ADDR, reg_readid, 2, config2_readid, 4, 1000);

    if (ret != HAL_OK) {
        DLOG_Critical("ret =%08x !", ret);
    }

    DLOG_Critical("version = %02x %02x %02x %02x", config2_readid[0], config2_readid[1], config2_readid[2], config2_readid[3]);

    HAL_Delay(20);

    is_Inited = 1;
}

_EXT_DTCM1
void mini384_reset(void)
{
    HAL_GPIO_OutPut(MINI384_RST);
    HAL_GPIO_SetPin(MINI384_RST, HAL_GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_SetPin(MINI384_RST, HAL_GPIO_PIN_SET);
    HAL_Delay(3000);
}

/////////////////////////////////////////////////////

_EXT_DTCM1
static void run_mini384(void* parameter)
{
    if (!is_Inited) {

        mini384_reset();
        mini384_init();
        is_Inited = 1; // just set once.
    } else {
    }
}

_EXT_DTCM1_BSS
static struct WorkItem mini384_item = {
    .name = "mini384",
    .period = 300,
    .schedule_time = 2000,
    .run = run_mini384
};

_EXT_DTCM1
void mini384_start(void)
{

    WorkQueue_t lp_wq = workqueue_find("wq:lp_work");

    RT_ASSERT(lp_wq != NULL);

    FMT_CHECK(workqueue_schedule_work(lp_wq, &mini384_item));
}
