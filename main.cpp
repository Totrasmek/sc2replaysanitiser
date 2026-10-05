#include "message.hpp"
#include "archive.hpp"

int main(void) {
    std::string input_name = "verylargerepeatedmsgs.SC2Replay";
    std::string output_name = "output.SC2Replay";
    Archive archive(input_name, output_name);
    for(int i = 0; i < 4; ++i) {
        Message message(archive);
        message.debug_print();
    }
    return 0;
}
