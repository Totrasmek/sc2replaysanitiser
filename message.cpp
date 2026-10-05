#include "message.hpp"

#include <iostream>

#include "archive.hpp"

Message::Message(Archive& archive) : archive_(archive) {
    read_header();
    read_body();
    // then based on type read in as much as necessary
    // store raw data for non message types
}

void Message::read_header(void) {
    if (!archive_.read_from_chat_file(reinterpret_cast<uint8_t*>(&header_start_), sizeof(header_start_))) {
        throw std::runtime_error("Message deserialisation failed reading header start.");
    }
    if (header_start_.additional_frame_byte_count > 0) {
        additional_frame_bytes_.resize(header_start_.additional_frame_byte_count);
        if (!archive_.read_from_chat_file(additional_frame_bytes_.data(), header_start_.additional_frame_byte_count)) {
            throw std::runtime_error("Message deserialisation failed reading additional frame bytes.");
        }
    }
    read_frame();
    if (!archive_.read_from_chat_file(reinterpret_cast<uint8_t*>(&header_end_), sizeof(header_end_))) {
        throw std::runtime_error("Message deserialisation failed reading header end.");
    }
}

void Message::read_frame(void) {
    for (uint8_t frame_byte_index = 0; frame_byte_index < header_start_.additional_frame_byte_count; ++frame_byte_index) {
        frame_ |= additional_frame_bytes_[frame_byte_index] << (header_start_.additional_frame_byte_count - frame_byte_index - 1) * 8;
    }
    frame_ |= header_start_.time << (header_start_.additional_frame_byte_count * 8);
}

void Message::read_body(void) {
    // sc2reader claims big endian, but we assume little endian
    switch (header_end_.flag) {
    case CLIENT_CHAT_MESSAGE: {
        read_client_chat_message();
        break;
    }
    case CLIENT_PING_MESSAGE: {
        read_client_ping_message();
        break;
    }
    case LOADING_PROGRESS_MESSAGE: {
        read_loading_progress_message();
        break;
    }
    case SERVER_PING_MESSAGE: {
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

void Message::read_client_chat_message(void) {
    /*
    // alternative with more bit bashing but less pointers
    recipient_.value = header_end_.body_overrun & 0x07;
    uint8_t chat_message_length_upper_bits;
    if (!archive_.read_from_chat_file(&chat_message_length_upper_bits, 1)) {
        throw std::runtime_error("Message deserialisation failed reading client chat message length.");
    }
    chat_message_length.value = (header_end_.body_overrun >> 4) | ((chat_message_length_upper_bits & 0x7F)) << 3);
    chat_message_.resize(chat_message_length.value);
    if (!archive_.read_from_chat_file(&chat_message_.data(), chat_message_length.value)) {
        throw std::runtime_error("Message deserialisation failed reading client chat message.");
    }
    */
    memcpy(&client_chat_message_, ((uint8_t*)&header_end_)+1, 1);
    if (!archive_.read_from_chat_file(((uint8_t*)&client_chat_message_)+1, 1)) {
        throw std::runtime_error("Message deserialisation failed reading client chat message length.");
    }
    chat_message_.resize(client_chat_message_.chat_message_length);
    if (!archive_.read_from_chat_file((uint8_t*)chat_message_.data(), client_chat_message_.chat_message_length)) {
        throw std::runtime_error("Message deserialisation failed reading client chat message.");
    }
}

void Message::read_client_ping_message(void) {
    memcpy(&client_ping_message_, ((uint8_t*)&header_end_)+1, 1);
    if (!archive_.read_from_chat_file(((uint8_t*)&client_ping_message_)+1, 8)) {
        throw std::runtime_error("Message deserialisation failed reading client ping message length.");
    }
}

void Message::read_loading_progress_message(void) {
    memcpy(&loading_progress_message_, ((uint8_t*)&header_end_)+1, 1);
    if (!archive_.read_from_chat_file(((uint8_t*)&loading_progress_message_)+1, 4)) {
        throw std::runtime_error("Message deserialisation failed reading loading progress message length.");
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
            "body_overrun = 0x%02x, ",
            frame_,
            header_end_.pid,
            header_end_.flag,
            header_end_.data_overrun);
    if (header_end_.flag == CLIENT_CHAT_MESSAGE) {
        printf(
                "recipient = 0x%02x, "
                "chat_message_length = 0x%04x, "
                "chat_message = %s.",
                client_chat_message_.recipient,
                client_chat_message_.chat_message_length,
                chat_message_.c_str()
        );
    }
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
