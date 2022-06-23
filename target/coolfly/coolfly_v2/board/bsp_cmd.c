/******************************************************************************
 * Copyright 2020 The Firmament Authors. All Rights Reserved.
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

#include "module/syscmd/syscmd.h"
#include "ar1002_chip.h"
#include "ar1002_hal.h"

static int handle_reboot_cmd(int argc, char** argv, int optc, optv_t* optv)
{
	printf("rebooting...\n");
	printf(" coolfly will reboot, goodbye !~ \n");

	sys_msleep(10);

    HAL_GPIO_SetPin(HAL_GPIO_NUM74, HAL_GPIO_PIN_RESET);
	
	return 0;
}

int cmd_reboot(int argc, char** argv)
{
	return syscmd_process(argc, argv, handle_reboot_cmd);
}
FINSH_FUNCTION_EXPORT_ALIAS(cmd_reboot, __cmd_reboot, reboot the system);




