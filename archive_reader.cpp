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
    };

    ~ArchiveReader() {
        if (mpq_handle_ != NULL) {
            SFileCloseArchive(mpq_handle_);
        }
        if (chat_file_handle_ != NULL) {
            SFileCloseFile(chat_file_handle_);
        }
    };

    void list_files() {
        SFILE_FIND_DATA search_data;
        HANDLE search_handle = SFileFindFirstFile(mpq_handle_, "*", &search_data, NULL);
        if (search_handle != NULL) {
            std::cout << "Files found in archive:\n";
            do {
                std::cout << " - " << search_data.cFileName
                          << " (Size: " << search_data.dwFileSize << " bytes)\n";
            } while (SFileFindNextFile(search_handle, &search_data));
            SFileFindClose(search_handle);
        } else {
            std::cout << "No files found or internal listfile is missing/empty.\n";
        }
    }

    bool has_chat_file() {
        SFILE_FIND_DATA chat_file_data;
        HANDLE search_handle = SFileFindFirstFile(mpq_handle_, CHAT_FILE_NAME, &chat_file_data, NULL);
        if (search_handle == NULL) {
            std::cout << "Chat file not found." << std::endl;
            return false;
        }
        if (chat_file_data.dwFileSize > MAX_CHAT_FILE_BYTES) {
            std::cout << "Chat file size " << chat_file_data.dwFileSize << "B is larger than max " << MAX_CHAT_FILE_BYTES << "B." << std::endl;
            return false;
        }
        return true;
    }

    bool open_chat_file() {
        HANDLE file_handle_ = NULL;
        if (!SFileOpenFileEx(mpq_handle_, CHAT_FILE_NAME, 0, &file_handle_)) {
            std::cout << "Failed opening chat file." << std::endl;
            return false;
        }
        return true;
    }


    bool read_from_chat_file(uint8_t* read_buffer, size_t read_size) {
        if (read_buffer == NULL) {
            return false;
        }
        DWORD bytes_read = 1;
        uint8_t* read_buffer_index = read_buffer;
        while(bytes_read > 0) {
            if (!SFileReadFile(chat_file_handle_, read_buffer_index, read_size, &bytes_read, NULL) && GetLastError() != ERROR_HANDLE_EOF) {
                std::cout << "Failed reading chat file." << std::endl;
                return false;
            }
            read_buffer_index += bytes_read;
        }
        return true;
    }


    bool move_file_pointer(int32_t offset) {
        if (!SFileSetFilePointer(chat_file_handle_, (LONG)offset, NULL, FILE_CURRENT)) {
            std::cout << "Failed setting file pointer." << std::endl;
            return false;
        }
        return true;
    }

    bool load_chat_file() {
        if (!has_chat_file()) {
            std::cout << "No chat file." << std::endl;
            return false;
        }
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

    bool delete_chat_file_and_compact() {
        if (!SFileRemoveFile(mpq_handle_, CHAT_FILE_NAME, 0)) {
            std::cout << "Could not remove chat file." << std::endl;
            return false;
        }
        if (!SFileCompactArchive(mpq_handle_, NULL, 0)) {
            std::cout << "Could not compact archive." << std::endl;
            return false;
        }
        return true;
    }

private:
    const char* CHAT_FILE_NAME = "replay.message.events";
    static constexpr uint32_t MAX_CHAT_FILE_BYTES = 0x100000;

    HANDLE mpq_handle_ = NULL;
    HANDLE chat_file_handle_ = NULL;
    uint8_t chat_file_buffer_[MAX_CHAT_FILE_BYTES] = {0};
};

int main() {
    std::string input_name = "verylargerepeatedmsgs.SC2Replay";
    std::string output_name = "output.SC2Replay";
    ArchiveReader archive_reader(input_name, output_name);
    archive_reader.list_files();
    if (!archive_reader.load_chat_file()) {
        std::cout << "Chat file load failed" << std::endl;
        return 1;
    }
    if (!archive_reader.delete_chat_file_and_compact()) {
        return 1;
    }
}
