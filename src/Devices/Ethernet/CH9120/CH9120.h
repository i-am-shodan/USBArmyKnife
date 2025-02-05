#ifndef _CH9120_H_
#define _CH9120_H_

#include <Arduino.h>

#include <stdlib.h> // malloc() free()
#include <stdio.h>


/// \tag::uart_advanced[]
#define UART_ID1 Serial2 
#define Inti_BAUD_RATE 9600
#define Transport_BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.

#define UART_TX_PIN1 20
#define UART_RX_PIN1 21

#define CFG_PIN 18
#define RES_PIN 19
#define GPIO_OUT 1
#define GPIO_IN 0

#define UCHAR unsigned char
#define UBYTE uint8_t
#define UWORD uint16_t
#define UDOUBLE uint32_t

#define TCP_SERVER 0
#define TCP_CLIENT 1
#define UDP_SERVER 2
#define UDP_CLIENT 3

#define Mode1 0x10               //Port 1: Setup Mode   0x00:TCP Server 0x01:TCP Client 0x02:UDP Server 0x03:UDP Client
#define LOCAL_IP 0x11            //Local IP
#define SUBNET_MASK 0x12         //Subnet Mask
#define GATEWAY 0x13             //Gateway
#define LOCAL_PORT1 0X14         //Port 1:Local Port
#define TARGET_IP1 0x15          //Port 1:Target IP
#define TARGET_PORT1 0x16        //Port 1:Target Port
#define PORT_RANDOM_ENABLE1 0x17 //Port 1:Port Random Enable
#define UART1_BAUD1 0x21         //Port 1:Baud rate of serial port 1

void CH9120_init(void);
void RX_TX();
void DEV_Delay_ms(UDOUBLE xms);
void DEV_Delay_us(UDOUBLE xus);

#endif
