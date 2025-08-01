#pragma once
#ifndef UART_FRAME_HPP
#define UART_FRAME_HPP

#define BYTE_MAX 255
#define KILO_BYTE_MAX 1024
#define AUTH_TAG_SIZE 16
#define PUBLIC_KEY_SIZE 32
#define ENCRYPTED_PAYLOAD_SIZE 256
#define NONCE_SIZE 16
#define NEVER_KEY_EXCHANGE 1000
#define TOTAL_UART_DATA_SIZE_TRANSMIT_TO_STM32 59
#define MAX_SEQUENCE_NUMBER 11
#define NUM_PACKETS_WAIT_FOR_STABLE 10

#define IGNORE_PADDING __attribute__((packed))

enum UartFrameConstants
{
    UART_FRAME_HEADER_1 = 0xAB,
    UART_FRAME_HEADER_2 = 0xCD,
    UART_FRAME_TRAILER_1 = 0xE1,
    UART_FRAME_TRAILER_2 = 0xE2,
    UART_FRAME_ERROR_ENCRYPTED = 0x01,
    UART_FRAME_ERROR_UNKNOWN = 0xFF,
    UART_FRAME_ERROR_MISMATCH = 0xFA,
    UART_FRAME_ERROR_IDENTIFIER = 0xFB,
    UART_FRAME_ERROR_HEADER_MISMATCH = 0xDA,
    UART_FRAME_ERROR_TRAILER_MISMATCH = 0xDB,
    UART_FRAME_ERROR_NONCE_MISMATCH = 0xDC,
    UART_FRAME_INVALID_AAD_LEN = 0xDD,
    UART_FRAME_INVALID_SECRET_LEN = 0xDE,
    UART_FRAME_DECRYPTION_FAILED = 0xDF,
    UART_FRAME_WRONG_PACKET_TYPE = 0xEA,
    UART_FRAME_TRIGGER_SIGNAL = 0x01
};

enum Dummy
{
    DUMMY = 0x00
};

enum UartTimer
{
    UART_FRAME_TIMEOUT_MS = 100
};

enum UARTCommand
{
    STM_RECEIVE_KEY = 0x02,
    SIGNAL = 0x01,
    INITIAL = 0x03,
    FAILED = 0x04
};

enum UartParserState
{
    VERIFY_HEADER_1_BYTE,
    VERIFY_HEADER_2_BYTE,
    RECEIVE_DEVICE_ID,
    RECEIVE_NONCE,
    WAIT_FOR_DATA_LENGTH_1_BYTE,
    WAIT_FOR_DATA_LENGTH_2_BYTE,
    RECEIVE_DATA,
    RECEIVE_AUTH_TAG,
    VERIFY_TRAILER_1_BYTE,
    VERIFY_TRAILER_2_BYTE,
    WAIT_FOR_CRC_1_BYTE,
    WAIT_FOR_CRC_2_BYTE,
    VERIFY_CRC,
    FRAME_COMPLETE,
    FRAME_ERROR
};

enum ServerFrameConstants
{
    SERVER_FRAME_PREAMBLE = 0xAA55,
    SERVER_FRAME_IDENTIFIER_ID = 0x08110910,
    SERVER_FRAME_PACKET_HANDSHAKE_TYPE = 0x03,
    SERVER_FRAME_PACKET_DATA_TYPE = 0x01,
    SERVER_FRAME_PACKET_ACK_TYPE = 0x02,
    SERVER_FRAME_PACKET_METRICS_TYPE = 0x04,
    SERVER_FRAME_PACKET_INIT_SESSION_TYPE = 0x05,
    SERVER_FRAME_UPDATE_SEQUENCE_NUMBER = 0x11,
    SERVER_FRAME_SEQUENCE_NUMBER = 10,
    SERVER_FRAME_END_MAKER = 0xAABB,
    RESET_SEQUENCE = 0,
    INITIAL_SEQUENCE = -1,
    SERVER_RECEIVE_ACK,
    SERVER_RECEIVE_SEQUENCE_NUMBER,
    SERVER_MQTT_FAILED,
    SERVER_FRAME_UNIQ = 9,
    SERVER_GET_UNIQ = 0
};

const uint8_t SERVER_FRAME_AUTH_TAG[AUTH_TAG_SIZE] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x18, 0x29, 0x3A, 0x4B, 0x5C, 0x6D, 0x7E, 0x8F, 0x90};

enum HandshakeState
{
    GENERATE_PUBLIC_KEY,
    CONSTRUCT_PUBLIC_KEY_FRAME,
    SEND_PUBLIC_KEY_FRAME,
    WAIT_FOR_PUBLIC_FROM_SERVER,
    COMPUTE_SHARED_SECRET,
    HANDSHAKE_COMPLETE
};

#endif // UART_FRAME_HPP