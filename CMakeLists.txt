# Copyright (c) 2026 GALPANEL contributors
# SPDX-License-Identifier: MIT

if(CONFIG_GALPANEL_ENDPOINT_LED)
  target_sources(app PRIVATE src/galpanel_endpoint_led.c)
endif()

if(CONFIG_GALPANEL_STATUS_INDICATORS)
  target_sources(app PRIVATE src/galpanel_status.c)
endif()
