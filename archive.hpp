#pragma once

#include <string>

#include <StormLib.h>

class Archive {
public:
    Archive(std::string archive_path, std::string output_path);
    ~Archive(void);

    bool read_from_chat_file(uint8_t* buffer, size_t size);
    bool load_chat_file(void);
private:
    const char* CHAT_FILE_NAME = "replay.message.events";
    static constexpr uint32_t MAX_CHAT_FILE_BYTES = 0x100000;
    uint8_t chat_file_buffer_[MAX_CHAT_FILE_BYTES] = {0};
    HANDLE mpq_handle_ = NULL;
    HANDLE chat_file_handle_ = NULL;
};
