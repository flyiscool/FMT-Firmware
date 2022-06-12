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

#include "drv_gpio.h"
#include "hal/pin/pin.h"

static struct pin_device pin_device;

static void ar1002_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
    // GPIO_TypeDef* gpio_port;
    // uint16_t gpio_pin;

    // if (PIN_PORT(pin) < PIN_STPORT_MAX) {
    //     gpio_port = PIN_STPORT(pin);
    //     gpio_pin = PIN_STPIN(pin);

    //     if (value == GPIO_PIN_RESET) {
    //         LL_GPIO_ResetOutputPin(gpio_port, gpio_pin);
    //     } else {
    //         LL_GPIO_SetOutputPin(gpio_port, gpio_pin);
    //     }
    // }
}

static int ar1002_pin_read(rt_device_t dev, rt_base_t pin)
{
    // GPIO_TypeDef* gpio_port;
    // uint16_t gpio_pin;
    int value = PIN_LOW;

    // if (PIN_PORT(pin) < PIN_STPORT_MAX) {
    //     gpio_port = PIN_STPORT(pin);
    //     gpio_pin = PIN_STPIN(pin);

    //     value = LL_GPIO_IsInputPinSet(gpio_port, gpio_pin);
    // }

    return value;
}

static void ar1002_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode, rt_base_t otype)
{
    // LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    // if (PIN_PORT(pin) >= PIN_STPORT_MAX) {
    //     return;
    // }

    // GPIO_InitStruct.Pin = PIN_STPIN(pin);
    // GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    // GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    // GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    // GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;

    // if (otype == PIN_OUT_TYPE_PP) {
    //     GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    // } else if (otype == PIN_OUT_TYPE_OD) {
    //     GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    // }

    // if (mode == PIN_MODE_OUTPUT) {
    //     /* output setting */
    //     GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    //     GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    // } else if (mode == PIN_MODE_INPUT) {
    //     /* input setting: not pull. */
    //     GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    //     GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    // } else if (mode == PIN_MODE_INPUT_PULLUP) {
    //     /* input setting: pull up. */
    //     GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    //     GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    // } else if (mode == PIN_MODE_INPUT_PULLDOWN) {
    //     /* input setting: pull down. */
    //     GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    //     GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
    // } else {
    //     /* unsurpoted mode */
    //     return;
    // }

    // LL_GPIO_Init(PIN_STPORT(pin), &GPIO_InitStruct);
}

const static struct pin_ops _ar1002_pin_ops = {
    ar1002_pin_mode,
    ar1002_pin_write,
    ar1002_pin_read,
};

rt_err_t drv_gpio_init(void)
{
    pin_device.ops = &_ar1002_pin_ops;

    return hal_pin_register(&pin_device, "pin", RT_DEVICE_FLAG_RDWR, RT_NULL);
}
