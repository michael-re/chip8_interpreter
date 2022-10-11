#include "interpreter.h"
#include <argparse.h>

#include <string>
#include <vector>

using interpreter_8::Interpreter;

int main(int argc, char* argv[]) {
    std::vector<std::string> args = argparse(argc, argv);

    const char* title = args[0].c_str();
    const char* rom   = args[1].c_str();

    unsigned scale =  std::stoi(args[2]);
    unsigned delay = 3;

    // Create interpreter and RUN RUN RUN RUN RUN!!!!!!!!!!!
    Interpreter chip8(title, rom, delay, scale);
    chip8.Run();

    return EXIT_SUCCESS;
}
