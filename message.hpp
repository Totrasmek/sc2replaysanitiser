#pragma once

#include <vector>

#include "archive.hpp"

class Message {
public:
    Message(Archive& archive);
    void debug_print(void);
private:
    struct __attribute__((__packed__)) HeaderStart {
        uint8_t additional_frame_byte_count : 2;
        uint8_t time : 6;
    };
    struct __attribute__((__packed__)) HeaderEnd {
        uint8_t pid : 5;
        uint8_t flag : 4;
        uint8_t data_overrun : 7;
    };

    void extract_frame(void);

    std::vector<uint8_t> additional_frame_bytes;
    uint32_t frame;
    HeaderStart header_start;
    HeaderEnd header_end;
};
