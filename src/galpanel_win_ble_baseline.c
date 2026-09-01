/*
 * Windows BLE pairing baseline name override.
 *
 * The Bluetooth device name is stored in nRF settings when it has previously
 * been changed at runtime. Reflashing a UF2 does not erase that settings
 * partition, so this diagnostic explicitly replaces any persisted GALPANEL N
 * name after BLE startup.
 */

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/ble.h>

LOG_MODULE_REGISTER(galpanel_win_ble_baseline, CONFIG_ZMK_LOG_LEVEL);

static void set_baseline_name(struct k_work *work) {
    ARG_UNUSED(work);

    char name[] = "GP WIN BLE";
    const int err = zmk_ble_set_device_name(name);
    if (err < 0) {
        LOG_ERR("Failed to set Windows BLE baseline name: %d", err);
    } else {
        LOG_INF("Windows BLE baseline name active: %s", name);
    }
}

static K_WORK_DELAYABLE_DEFINE(baseline_name_work, set_baseline_name);

static int baseline_name_init(void) {
    /* Let ZMK restore BLE/settings first, then override any persisted name. */
    k_work_schedule(&baseline_name_work, K_MSEC(1000));
    return 0;
}

SYS_INIT(baseline_name_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
