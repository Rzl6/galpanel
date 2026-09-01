/*
 * GALPANEL BLE profile names.
 *
 * The active profile is advertised as GALPANEL 1 through GALPANEL 5 so a host
 * can distinguish the five independent pairing slots.
 *
 * Copyright (c) 2026 GALPANEL contributors
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

#include <zmk/ble.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>

LOG_MODULE_REGISTER(galpanel_profile_name, CONFIG_ZMK_LOG_LEVEL);

static void set_active_profile_name(struct k_work *work) {
    ARG_UNUSED(work);

    char name[16];
    const uint8_t profile = zmk_ble_active_profile_index();
    const int len = snprintk(name, sizeof(name), "GALPANEL %u", profile + 1);
    if (len < 0 || len >= sizeof(name)) {
        LOG_ERR("Failed to format BLE profile name for profile %u", profile + 1);
        return;
    }

    const int err = zmk_ble_set_device_name(name);
    if (err < 0) {
        LOG_ERR("Failed to set BLE profile name to %s: %d", name, err);
    } else {
        LOG_INF("BLE advertising name: %s", name);
    }
}

static K_WORK_DELAYABLE_DEFINE(profile_name_work, set_active_profile_name);

static int profile_name_listener(const zmk_event_t *eh) {
    ARG_UNUSED(eh);
    k_work_reschedule(&profile_name_work, K_NO_WAIT);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(galpanel_profile_name, profile_name_listener);
ZMK_SUBSCRIPTION(galpanel_profile_name, zmk_ble_active_profile_changed);

static int profile_name_init(void) {
    /* BLE/settings initialization completes before this deferred work runs. */
    k_work_schedule(&profile_name_work, K_MSEC(500));
    return 0;
}

SYS_INIT(profile_name_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
