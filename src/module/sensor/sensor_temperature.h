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
#ifndef SENSOR_TEMPERATURE_H__
#define SENSOR_TEMPERATURE_H__

#include <firmament.h>

#include "module/sensor/sensor_hub.h"

#ifdef __cplusplus
extern "C" {
#endif

sensor_temperature_t sensor_temperature_init(const char* temp_dev_name);
fmt_err_t sensor_temperature_measure(sensor_temperature_t temp_dev, temperature_data_t* temp_report);

#ifdef __cplusplus
}
#endif

#endif