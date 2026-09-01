/*
 * GALPANEL status indicators and guarded SYS controls.
 *
 * The status test intentionally uses a small polling loop. ZMK exposes the
 * selected endpoint and BLE/profile state as stable APIs, while polling also
 * catches disconnects that do not generate an endpoint event by themselves.
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

#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/keymap.h>

LOG_MODULE_REGISTER(galpanel_status, CONFIG_ZMK_LOG_LEVEL);

#define GALPANEL_FN_LAYER 1
#define GALPANEL_SYS_POSITION 5
#define GALPANEL_STATUS_PERIOD K_MSEC(150)
#define GALPANEL_SAFETY_HOLD_MS 6000
#define GALPANEL_SYS_TAP_TERM_MS 300

static const struct gpio_dt_spec led_link = GPIO_DT_SPEC_GET(DT_NODELABEL(led_link), gpios);
static const struct gpio_dt_spec led_info = GPIO_DT_SPEC_GET(DT_NODELABEL(led_info), gpios);
static const struct gpio_dt_spec led_fn = GPIO_DT_SPEC_GET(DT_NODELABEL(led_fn), gpios);
static const struct gpio_dt_spec led_warn = GPIO_DT_SPEC_GET(DT_NODELABEL(led_warn), gpios);
static const struct gpio_dt_spec led_aux = GPIO_DT_SPEC_GET(DT_NODELABEL(led_aux), gpios);

static uint8_t info_toggle_remaining;
static bool info_flash_on;
static bool safety_held;
static bool safety_cleared;
static int64_t safety_hold_started;
static int64_t safety_ack_until;
static uint8_t safety_tap_count;
static bool last_ble_connected;
static int64_t link_led_until;
static int64_t info_on_until;

static void status_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(status_work, status_work_handler);

#if IS_ENABLED(CONFIG_GALPANEL_SAFETY_TEST)
static void safety_warn_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(safety_warn_work, safety_warn_work_handler);
static void safety_tap_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(safety_tap_work, safety_tap_work_handler);
#endif

static bool set_led(const struct gpio_dt_spec *led, bool on) {
    return gpio_pin_set_dt(led, on) >= 0;
}

static void write_status_leds(void) {
    const bool ble_connected = zmk_ble_active_profile_is_connected();
    const bool fn_active = zmk_keymap_layer_active(GALPANEL_FN_LAYER);
    const bool usb_selected = zmk_endpoint_get_selected().transport == ZMK_TRANSPORT_USB;

    /* LINK is a BLE indicator. USB use is deliberately quiet; when BLE is
     * selected but not connected it slow-blinks so pairing/reconnect remains
     * visible without confusing it with the board's power LED. */
    const int64_t now = k_uptime_get();
    if (ble_connected && !last_ble_connected) {
        link_led_until = now + CONFIG_GALPANEL_LINK_ON_MS;
    }
    last_ble_connected = ble_connected;

    bool link_on;
    if (usb_selected) {
        link_on = false;
    } else if (!ble_connected) {
        link_on = (now / 500) % 2;
    } else if (CONFIG_GALPANEL_LINK_ON_MS == 0) {
        link_on = true;
    } else {
        link_on = now < link_led_until;
    }
    set_led(&led_link, link_on);
    set_led(&led_fn, fn_active);
    set_led(&led_aux, usb_selected);

    /* INFO announces a profile with flashes. A successful bond clear has a
     * longer, unambiguous ten-second acknowledgement. */
    set_led(&led_info, info_flash_on || now < info_on_until);

    /* WARN is reserved for the safety test and remains off otherwise. */
    const bool warn_on = safety_held ? (safety_cleared || (k_uptime_get() / 250) % 2)
                                     : (k_uptime_get() < safety_ack_until);
    set_led(&led_warn, warn_on);
}

static void status_work_handler(struct k_work *work) {
    ARG_UNUSED(work);

    if (info_toggle_remaining > 0) {
        info_flash_on = !info_flash_on;
        info_toggle_remaining--;
    } else {
        info_flash_on = false;
    }

    write_status_leds();
    k_work_schedule(&status_work, GALPANEL_STATUS_PERIOD);
}

static void info_flash_start(uint8_t profile) {
    info_on_until = 0;
    info_toggle_remaining = (profile + 1) * 2;
    info_flash_on = false;
    k_work_reschedule(&status_work, K_NO_WAIT);
}

static int status_event_listener(const zmk_event_t *eh) {
    const struct zmk_ble_active_profile_changed *profile_event =
        as_zmk_ble_active_profile_changed(eh);

    if (profile_event != NULL) {
        info_flash_start(profile_event->index);
    }

    k_work_reschedule(&status_work, K_NO_WAIT);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(galpanel_status, status_event_listener);
ZMK_SUBSCRIPTION(galpanel_status, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(galpanel_status, zmk_layer_state_changed);

#if IS_ENABLED(CONFIG_GALPANEL_SAFETY_TEST)

static void safety_warn_work_handler(struct k_work *work) {
    ARG_UNUSED(work);

    if (!safety_held) {
        return;
    }

    const int64_t held_ms = k_uptime_get() - safety_hold_started;
    if (held_ms >= GALPANEL_SAFETY_HOLD_MS && !safety_cleared) {
        zmk_ble_clear_bonds();
        safety_cleared = true;
        info_on_until = k_uptime_get() + 10000;
        LOG_WRN("Safety test cleared the current BLE profile bond");
    }

    k_work_reschedule(&status_work, K_NO_WAIT);
    k_work_schedule(&safety_warn_work, K_MSEC(100));
}

static void safety_tap_work_handler(struct k_work *work) {
    ARG_UNUSED(work);

    if (safety_tap_count == 1) {
        zmk_ble_prof_next();
    }
    safety_tap_count = 0;
}

static int safety_event_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *event = as_zmk_position_state_changed(eh);
    if (event == NULL || event->source != ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL ||
        event->position != GALPANEL_SYS_POSITION) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (event->state) {
        k_work_cancel_delayable(&safety_tap_work);
        safety_held = true;
        safety_cleared = false;
        safety_hold_started = k_uptime_get();
        k_work_reschedule(&safety_warn_work, K_NO_WAIT);
    } else {
        const int64_t held_ms = k_uptime_get() - safety_hold_started;
        safety_held = false;
        k_work_cancel_delayable(&safety_warn_work);
        if (safety_cleared) {
            /* Keep a short post-action acknowledgement visible. */
            safety_ack_until = k_uptime_get() + 500;
            safety_cleared = false;
            safety_tap_count = 0;
        } else if (held_ms <= GALPANEL_SYS_TAP_TERM_MS) {
            safety_tap_count++;
            if (safety_tap_count >= 2) {
                safety_tap_count = 0;
                zmk_endpoint_toggle_preferred_transport();
            } else {
                k_work_reschedule(&safety_tap_work, K_MSEC(GALPANEL_SYS_TAP_TERM_MS));
            }
        } else {
            /* An aborted long hold is not a tap and must not change Profile. */
            safety_tap_count = 0;
        }
        k_work_reschedule(&status_work, K_NO_WAIT);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(galpanel_safety, safety_event_listener);
ZMK_SUBSCRIPTION(galpanel_safety, zmk_position_state_changed);

#endif

static int status_init(void) {
    const struct gpio_dt_spec *leds[] = {&led_link, &led_info, &led_fn, &led_warn, &led_aux};
    for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
        if (!gpio_is_ready_dt(leds[i])) {
            LOG_ERR("GALPANEL status LED %u is not ready", i);
            return -ENODEV;
        }
        const int err = gpio_pin_configure_dt(leds[i], GPIO_OUTPUT_INACTIVE);
        if (err < 0) {
            LOG_ERR("Failed to configure GALPANEL status LED %u: %d", i, err);
            return err;
        }
    }

    info_flash_start(zmk_ble_active_profile_index());
    k_work_schedule(&status_work, GALPANEL_STATUS_PERIOD);
    return 0;
}

SYS_INIT(status_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
