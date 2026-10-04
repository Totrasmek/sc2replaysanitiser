#include "message.hpp"

#include <iostream>

#include "archive.hpp"

Message::Message(Archive& archive) : archive_(archive) {
    extract_header();
    extract_body();
    // then based on type read in as much as necessary
    // store raw data for non message types
}

void Message::extract_header(void) {
    if (!archive_.read_from_chat_file(reinterpret_cast<uint8_t*>(&header_start_), sizeof(header_start_))) {
        throw std::runtime_error("Message deserialisation failed reading header start.");
    }
    if (header_start_.additional_frame_byte_count > 0) {
        additional_frame_bytes_.resize(header_start_.additional_frame_byte_count);
        if (!archive_.read_from_chat_file(additional_frame_bytes_.data(), header_start_.additional_frame_byte_count)) {
            throw std::runtime_error("Message deserialisation failed reading additional frame bytes.");
        }
    }
    extract_frame();
    if (!archive_.read_from_chat_file(reinterpret_cast<uint8_t*>(&header_end_), sizeof(header_end_))) {
        throw std::runtime_error("Message deserialisation failed reading header end.");
    }
}

void Message::extract_frame(void) {
    for (uint8_t frame_byte_index = 0; frame_byte_index < header_start_.additional_frame_byte_count; ++frame_byte_index) {
        frame_ |= additional_frame_bytes_[frame_byte_index] << (header_start_.additional_frame_byte_count - frame_byte_index - 1) * 8;
    }
    frame_ |= header_start_.time << (header_start_.additional_frame_byte_count * 8);
}

void Message::extract_body(void) {
    switch (header_end_.flag) {
    case CLIENT_CHAT_MESSAGE: {
        /*3bits recipient or 2 based on 'base build'... shit need to read another file
        what is base build?
        11 bits for string size
        read(string_size)
        align byte*/
        break;
    }
    case CLIENT_PING_MESSAGE: {
        /*3bits recipient or 2 based on 'base build'... shit need to read another file
        uint32_t
        uint32_t
        so 9B, don't need base build
        ... or do we? 3bits reads inside the previous byte which was already overrun
        do the uin32_t reads happen byte aligned?*/
        break;
    }
    case LOADING_PROGRESS_MESSAGE: {
        /*4B*/
        break;
    }
    case SERVER_PING_MESSAGE: {
        /*pass*/
        break;
    }
    default: {
        throw std::runtime_error(
            std::string("Message deserialisation failed unsupported message type: ")
            + std::to_string(static_cast<int>(header_end_.flag))
            + std::string(".")
        );
        break;
    }
    }
}

//    std::vector<uint8_t> serialise(void) {}

void Message::debug_print() {
    printf( "Message Header: "
            "additional_frame_byte_count = 0x%02x, "
            "time = 0x%02x, ",
            header_start_.additional_frame_byte_count,
            header_start_.time);
    printf("additional_frame_bytes_ = ");
    for (const uint8_t& byte : additional_frame_bytes_) {
        printf("0x%02x ", byte);
    }
    printf( "frame = 0x%02x, "
            "pid = 0x%02x, "
            "flag = 0x%02x, "
            "data_overrun = 0x%02x.",
            frame_,
            header_end_.pid,
            header_end_.flag,
            header_end_.data_overrun);
    printf("\n");
}

// How to loop this to check for EOF error return?
// Could call the read function here and have the read function return bytes read?

/*std::vector<std::unique_ptr<Message>> messages;
while (???) {
    MessageHeader message_header;
    if (!message_header.deserialise(archive)) {
        std::cout << "Failed deserialising archive reader" << std::endl;
        return 1;
    }
    switch (message_header.header_end_.flag) {
    case CLIENT_CHAT_MESSAGE: {
        messages.push_back(std::make_unique<ClientChatMessage>(message_header));
        break;
    }
    case CLIENT_PING_MESSAGE: {
        messages.push_back(std::make_unique<ClientPingMessage>(message_header));
        break;
    }
    case LOADING_PROGRESS_MESSAGE: {
        messages.push_back(std::make_unique<LoadingProgressMessage>(message_header));
        break;
    }
    case SERVER_PING_MESSAGE: {
        messages.push_back(std::make_unique<ServerPingMessage>(message_header));
        break;
    }
    default: {
        throw std::runtime_error(std::string("Unsupported message event flag" + message_header.header_end_.flag));
    }
    }
    messages.back().deserialise(archive);
}*/

// duplicate chat messages so user can restore any actions
// display chat messages to user
// user selects any chat message
// user can pop message from vector
// user can edit message data
// user can add a message??
/*
class Message {
public:
    enum MessageType {
        CLIENT_CHAT_MESSAGE = 0;
        CLIENT_PING_MESSAGE = 1;
        LOADING_PROGRESS_MESSAGE = 2;
        SERVER_PING_MESSAGE = 3;
    };
    static read_header // calls frame pid flag read
    get_message_type // use this to identify and edit chat messages?
    virtual int deserialise(Archive archive);
    virtual serialise // but has a parent method which serialises the header?
        //frame = (first_byte.time << (8 * first_byte.additional_frame_bytes)) | additional_frame_bytes;
    virtual std::vector<uint8_t> serialise(void);
    virtual destructor
    virtual edit_data (implement stubs for non chat messages)
private:
    read_frame
    read_pid
    read_flag
    serialise_header
    virtual serialise_data
    MessageType message_type_;
    MessageHeader message_header_;
};

class ClientChatMessage : public Message {
    read_recipient
    read_text_length
    read_text
};

class ClientPingMessage : public Message {
    read_recipient
    read_x
    read_y
};

class LoadingProgressMessage : public Message {
    read_progress
};

class ServerPingMessage : public Message {};
*/
/*
Build list of each event
Edit list of events
Serialise list of events with a new array that gets realloced with each adjustment
    - better if we can constantly calculate the updated size to check for issues
    - each event should have a 'get size'
*/
