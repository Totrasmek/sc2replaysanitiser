#pragma once

#include <vector>

#include "archive.hpp"

class Message {
public:
    Message(Archive& archive);
    void debug_print(void);
private:
    enum MessageType : uint8_t {
        CLIENT_CHAT_MESSAGE = 0,
        CLIENT_PING_MESSAGE = 1,
        LOADING_PROGRESS_MESSAGE = 2,
        SERVER_PING_MESSAGE = 3,
    };

    struct __attribute__((__packed__)) HeaderStart {
        uint8_t additional_frame_byte_count : 2;
        uint8_t time : 6;
    };
    struct __attribute__((__packed__)) HeaderEnd {
        uint8_t pid : 5;
        enum MessageType flag : 4;
        uint8_t data_overrun : 7;
    };
    struct __attribute__((__packed__)) ClientChatMessage {
        uint16_t header_overrun : 1;
        uint16_t recipient : 3;
        uint16_t chat_message_length : 11;
    };
    struct __attribute__((__packed__)) ClientPingMessage {
        uint32_t header_overrun : 1;
        uint32_t recipient : 3;
        uint32_t ping_x_coordinate_raw : 32;
        uint32_t ping_y_coordinate_raw : 32;
    };
    struct __attribute__((__packed__)) LoadingProgressMessage {
        uint32_t header_overrun : 1;
        uint32_t loading_progress : 32;
    };

    void read_header(void);
    void read_frame(void);
    void read_body(void);
    void read_client_chat_message(void);
    void read_client_ping_message(void);
    void read_loading_progress_message(void);

    Archive& archive_;
    std::vector<uint8_t> additional_frame_bytes_;
    uint32_t frame_;
    HeaderStart header_start_;
    HeaderEnd header_end_;
    ClientChatMessage client_chat_message_;
    ClientPingMessage client_ping_message_;
    LoadingProgressMessage loading_progress_message_;
    std::string chat_message_;
};
