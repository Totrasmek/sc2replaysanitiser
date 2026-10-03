#include "message.cpp"
#include "archive_reader.cpp"

int main(void) {
    std::string input_name = "verylargerepeatedmsgs.SC2Replay";
    std::string output_name = "output.SC2Replay";
    ArchiveReader archive_reader(input_name, output_name);
        uint8_t byte;
        archive_reader.read_from_chat_file(&byte, 1);
    // something broken with passing the archive reader into deserialiser, i think the pass by reference or something
    MessageHeader message_header;
    message_header.deserialise(archive_reader);
    message_header.debug_print();
    return 0;
}
