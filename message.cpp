#include <iostream>
#include <StormLib.h> // Include StormLib header

#include "archive_reader.cpp"

struct MessageHeader {
    struct __attribute__((__packed__)) MessageHeaderStart {
        uint8_t additional_byte_count : 2;
        uint8_t time : 6;
    };
    struct __attribute__((__packed__)) MessageHeaderEnd {
        uint8_t pid : 5;
        uint8_t flag : 4;
    };
    MessageHeaderStart header_start;
    uint32_t additional_bytes;
    MessageHeaderEnd header_end;

    bool deserialise(ArchiveReader& archive_reader) {
        if (!archive_reader.read_from_chat_file(&header_start, sizeof(header_start))) {
            return false;
        }
        if (!archive_reader.read_from_chat_file(&additional_bytes, header_start.additional_byte_count)) {
            return false;
        }
        if (!archive_reader.read_from_chat_file(&header_end, sizeof(header_end)) {
            return false;
        }
        if (!archive_reader.move_file_pointer(-1)) {
            return false;
        }
        // TODO: Possible LSB vs MSB issues here
    };

    std::vector<uint8_t> serialise(void) {
    };
};

// How to loop this to check for EOF error return?
// Could call the read function here and have the read function return bytes read?

int main(void) {
ArchiveReader archive_reader(); // think about initialisation and how classes should interact
std::vector<std::unique_ptr<Message>> messages;
while (???) {
    MessageHeader message_header;
    if (!message_header.deserialise(archive_reader)) {
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
    messages.back().deserialise(archive_reader);
}
}

// duplicate chat messages so user can restore any actions
// display chat messages to user
// user selects any chat message
// user can pop message from vector
// user can edit message data
// user can add a message??

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
    virtual int deserialise(ArchiveReader archive_reader);
    virtual serialise // but has a parent method which serialises the header?
        //frame = (first_byte.time << (8 * first_byte.additional_bytes)) | additional_bytes;
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

/*
Build list of each event
Edit list of events
Serialise list of events with a new array that gets realloced with each adjustment
    - better if we can constantly calculate the updated size to check for issues
    - each event should have a 'get size'
*/
