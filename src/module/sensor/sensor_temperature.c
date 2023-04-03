/******************************************************************************
 * Copyright 2022 The Firmament Authors. All Rights Reserved.
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
#include "module/sensor/sensor_temperature.h"
#include "hal/temperature/temperature.h"

/**
 * @brief Measure temperature data
 * 
 * @param airspeed_dev Airspeed sensor device
 * @param airspeed_report Airspeed data buffer
 * @return fmt_err_t FMT_EOK for success
 */
fmt_err_t sensor_temperature_measure(sensor_temperature_t temperature_dev, temperature_data_t* temp_report)
{
    rt_size_t r_size;
    temperature_report_t report;

    RT_ASSERT(temperature_dev != NULL);

    r_size = rt_device_read(temperature_dev->dev, 9, &report, sizeof(temperature_data_t));

    temp_report->timestamp_ms = report.timestamp_ms;
    temp_report->temperature_deg = report.temperature_deg;

    return r_size == sizeof(temperature_data_t) ? FMT_EOK : FMT_ERROR;
}

/**
 * @brief Initialize temperature sensor device
 * 
 * @param temp_dev_name temperature device name
 * @return sensor_temperature_t temperature sensor device
 */
sensor_temperature_t sensor_temperature_init(const char* temperature_dev_name)
{
    sensor_temperature_t temperature_dev = (sensor_temperature_t)rt_malloc(sizeof(struct sensor_temperature));
    RT_ASSERT(temperature_dev != NULL);

    /* find temperature device */
    temperature_dev->dev = rt_device_find(temperature_dev_name);
    RT_ASSERT(temperature_dev->dev != NULL);

    /* open device */
    RT_CHECK(rt_device_open(temperature_dev->dev, RT_DEVICE_OFLAG_RDWR));


    return temperature_dev;
}