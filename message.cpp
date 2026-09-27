#include "message.hpp"

#include <iostream>

#include "archive.hpp"

Message::Message(Archive& archive) {
    if (!archive.read_from_chat_file(reinterpret_cast<uint8_t*>(&header_start), sizeof(header_start))) {
        throw std::runtime_error("Message deserialisation failed reading header start.");
    }
    if (header_start.additional_frame_byte_count > 0) {
        additional_frame_bytes.resize(header_start.additional_frame_byte_count);
        if (!archive.read_from_chat_file(additional_frame_bytes.data(), header_start.additional_frame_byte_count)) {
            throw std::runtime_error("Message deserialisation failed reading additional frame bytes.");
        }
    }
    extract_frame();
    if (!archive.read_from_chat_file(reinterpret_cast<uint8_t*>(&header_end), sizeof(header_end))) {
        throw std::runtime_error("Message deserialisation failed reading header end.");
    }
    // then based on type read in as much as necessary
    // store raw data for non message types
}

void Message::extract_frame(void) {
    for (uint8_t frame_byte_index = 0; frame_byte_index < header_start.additional_frame_byte_count; ++frame_byte_index) {
        frame |= additional_frame_bytes[frame_byte_index] << (header_start.additional_frame_byte_count - frame_byte_index - 1) * 8;
    }
    frame |= header_start.time << (header_start.additional_frame_byte_count * 8);
}

//    std::vector<uint8_t> serialise(void) {}

void Message::debug_print() {
    printf( "Message Header: "
            "additional_frame_byte_count = 0x%02x, "
            "time = 0x%02x, ",
            header_start.additional_frame_byte_count,
            header_start.time);
    printf("additional_frame_bytes = ");
    for (const uint8_t& byte : additional_frame_bytes) {
        printf("0x%02x ", byte);
    }
    printf( "frame = 0x%02x, "
            "pid = 0x%02x, "
            "flag = 0x%02x, "
            "data_overrun = 0x%02x.",
            frame,
            header_end.pid,
            header_end.flag,
            header_end.data_overrun);
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
    switch (message_header.header_end.flag) {
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
        throw std::runtime_error(std::string("Unsupported message event flag" + message_header.header_end.flag));
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
