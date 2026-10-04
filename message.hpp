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

    void extract_body(void);
    void extract_header(void);
    void extract_frame(void);

    Archive& archive_;
    std::vector<uint8_t> additional_frame_bytes_;
    uint32_t frame_;
    HeaderStart header_start_;
    HeaderEnd header_end_;
};
