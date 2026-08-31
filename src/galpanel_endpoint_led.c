/*
 * GALPANEL active endpoint indicator
 *
 * Copyright (c) 2026 GALPANEL contributors
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/endpoint_changed.h>

LOG_MODULE_REGISTER(galpanel_endpoint_led, CONFIG_ZMK_LOG_LEVEL);

static const struct gpio_dt_spec aux_led = GPIO_DT_SPEC_GET(DT_NODELABEL(led_aux), gpios);

static void update_aux_led(struct k_work *work) {
    const struct zmk_endpoint_instance endpoint = zmk_endpoint_get_selected();
    const int err = gpio_pin_set_dt(&aux_led, endpoint.transport == ZMK_TRANSPORT_USB);

    if (err < 0) {
        LOG_ERR("Failed to update AUX endpoint LED: %d", err);
    }
}

static K_WORK_DELAYABLE_DEFINE(aux_led_update_work, update_aux_led);

static int endpoint_led_listener(const zmk_event_t *eh) {
    ARG_UNUSED(eh);
    k_work_reschedule(&aux_led_update_work, K_NO_WAIT);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(galpanel_endpoint_led, endpoint_led_listener);
ZMK_SUBSCRIPTION(galpanel_endpoint_led, zmk_endpoint_changed);

static int endpoint_led_init(void) {
    if (!gpio_is_ready_dt(&aux_led)) {
        LOG_ERR("AUX LED GPIO is not ready");
        return -ENODEV;
    }

    const int err = gpio_pin_configure_dt(&aux_led, GPIO_OUTPUT_INACTIVE);
    if (err < 0) {
        LOG_ERR("Failed to configure AUX endpoint LED: %d", err);
        return err;
    }

    /* Endpoint initialization uses the same application init phase. Delay the
     * first read so it observes ZMK's completed USB/BLE selection. */
    k_work_schedule(&aux_led_update_work, K_MSEC(100));
    return 0;
}

SYS_INIT(endpoint_led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
