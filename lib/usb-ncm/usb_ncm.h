/**
 * @file usb_ncm.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-09-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

typedef bool (*IsPacketReady)();
typedef uint8_t* (*GetPacket)(uint32_t *);
typedef void (*FinishedProcessing)();
typedef bool (*TransmitPacket)(uint8_t *, uint32_t);
typedef void (*OnPacket)(uint8_t *, uint32_t, bool);

#define STATIC_IP_ADDR "10.0.0.2"
#define DEFAULT_GATEWAY "10.0.0.1"
#define DEFAULT_NETMASK "255.255.255.0"

/**
 * @brief Initialize USB NCM and start interface
 *
 * @return esp_err_t
 */
esp_err_t usb_ncm_init(IsPacketReady pktReady, GetPacket getPacket, FinishedProcessing finished, TransmitPacket transmit);
esp_err_t poll_for_packets();
void set_packet_handler(OnPacket handler);

#ifdef __cplusplus
}
#endif
