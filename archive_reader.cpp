#pragma once

#include <iostream>
#include <filesystem>
#include <StormLib.h> // Include StormLib header

class ArchiveReader {
public:
    ArchiveReader(std::string archive_path, std::string output_path) {
        std::error_code error_code;
        std::filesystem::copy_file(archive_path, output_path, std::filesystem::copy_options::overwrite_existing, error_code);
        if (error_code) {
            throw std::runtime_error(std::string("Failed copying archive: " + error_code.message()));
        }
        if (!SFileOpenArchive(output_path.c_str(), 0, STREAM_FLAG_WRITE_SHARE, &mpq_handle_)) {
            throw std::runtime_error(std::string("Failed to open archive. Error: " + GetLastError()));
        }
        if (!open_chat_file()) {
            throw std::runtime_error(std::string("Failed to open chat file. Error: " + GetLastError()));
        }
    };

    ~ArchiveReader() {
        if (mpq_handle_ != NULL) {
            SFileCloseArchive(mpq_handle_);
        }
        if (chat_file_handle_ != NULL) {
            SFileCloseFile(chat_file_handle_);
        }
    };

    bool open_chat_file() {
        if (!SFileOpenFileEx(mpq_handle_, CHAT_FILE_NAME, 0, &chat_file_handle_)) {
            std::cout << "Failed opening chat file." << std::endl;
            return false;
        }
        return true;
    }

    bool read_from_chat_file(uint8_t* read_buffer, size_t read_size) {
        if (read_buffer == NULL || chat_file_handle_ == NULL) {
            return false;
        }
        DWORD bytes_read = 1;
        uint8_t* read_buffer_index = read_buffer;
        while(bytes_read > 0) {
            if (!SFileReadFile(chat_file_handle_, read_buffer_index, read_size, &bytes_read, NULL) && GetLastError() != ERROR_HANDLE_EOF) {
                std::cout << "Failed reading chat file: " << GetLastError() << std::endl;
                return false;
            }
            read_buffer_index += bytes_read;
        }
        return true;
    }

bool load_chat_file() {
    HANDLE file_handle_ = NULL;
    if (!SFileOpenFileEx(mpq_handle_, CHAT_FILE_NAME, 0, &file_handle_)) {
        std::cout << "Failed opening chat file." << std::endl;
        return false;
    }
    DWORD bytes_read = 1;
    uint8_t* chat_file_buffer_copy_index = chat_file_buffer_;
    while(bytes_read > 0) {
        if (!SFileReadFile(file_handle_, chat_file_buffer_copy_index, sizeof(chat_file_buffer_), &bytes_read, NULL) && GetLastError() != ERROR_HANDLE_EOF) {
            std::cout << "Failed reading chat file." << std::endl;
            return false;
        }
        chat_file_buffer_copy_index += bytes_read;
    }
    SFileCloseFile(file_handle_);
    file_handle_ = NULL;
    return true;
}


private:
    const char* CHAT_FILE_NAME = "replay.message.events";
    static constexpr uint32_t MAX_CHAT_FILE_BYTES = 0x100000;
    uint8_t chat_file_buffer_[MAX_CHAT_FILE_BYTES] = {0};
    HANDLE mpq_handle_ = NULL;
    HANDLE chat_file_handle_ = NULL;
};
