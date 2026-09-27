#include "archive.hpp"

#include <iostream>
#include <filesystem>

Archive::Archive(std::string archive_path, std::string output_path) {
    std::error_code error_code;
    std::filesystem::copy_file(archive_path, output_path, std::filesystem::copy_options::overwrite_existing, error_code);
    if (error_code) {
        throw std::runtime_error(std::string("Failed copying archive: " + error_code.message()));
    }
    if (!SFileOpenArchive(output_path.c_str(), 0, STREAM_FLAG_WRITE_SHARE, &mpq_handle_)) {
        throw std::runtime_error(std::string("Failed to open archive. Error: " + GetLastError()));
    }
    if (!SFileOpenFileEx(mpq_handle_, CHAT_FILE_NAME, 0, &chat_file_handle_)) {
        throw std::runtime_error(std::string("Failed to open chat file. Error: " + GetLastError()));
    }
};

Archive::~Archive(void) {
    if (chat_file_handle_ != NULL) {
        SFileCloseFile(chat_file_handle_);
    }
    if (mpq_handle_ != NULL) {
        SFileCloseArchive(mpq_handle_);
    }
};

bool Archive::read_from_chat_file(uint8_t* buffer, size_t size) {
    if (buffer == NULL || chat_file_handle_ == NULL) {
        printf("null arg\n");
        return false;
    }
    DWORD bytes_to_read = size;
    DWORD bytes_read = 1;
    uint8_t* buffer_index = buffer;
    while(bytes_read > 0 && bytes_read <= size) {
        if (!SFileReadFile(chat_file_handle_, buffer_index, bytes_to_read, &bytes_read, NULL) && GetLastError() != ERROR_HANDLE_EOF) {
            std::cout << "Failed reading chat file: " << GetLastError() << std::endl;
            return false;
        }
        buffer_index += bytes_read;
        bytes_to_read -= bytes_read;
    }
    return true;
}
