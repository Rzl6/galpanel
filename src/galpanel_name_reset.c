/*
 * One-shot BLE name reset for recovery and historical regression testing.
 * This image writes the original static name "GALPANEL" to settings.
 */

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/ble.h>

LOG_MODULE_REGISTER(galpanel_name_reset, CONFIG_ZMK_LOG_LEVEL);

static void set_historical_name(struct k_work *work) {
    ARG_UNUSED(work);

    char name[] = "GALPANEL";
    const int err = zmk_ble_set_device_name(name);
    if (err < 0) {
        LOG_ERR("Failed to restore historical BLE name: %d", err);
    } else {
        LOG_INF("Historical BLE name active: %s", name);
    }
}

static K_WORK_DELAYABLE_DEFINE(name_reset_work, set_historical_name);

static int name_reset_init(void) {
    /* Allow BLE/settings initialization to complete first. */
    k_work_schedule(&name_reset_work, K_MSEC(1000));
    return 0;
}

SYS_INIT(name_reset_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
