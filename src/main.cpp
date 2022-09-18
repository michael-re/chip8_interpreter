#include "interpreter.h"

#include <string>
#include <vector>
#include <iostream>

std::vector<std::string> argparse(int argc, char* argv[]) {
    std::vector<std::string> args;
    if (argc == 5 && std::string(argv[1]) == "-rom" || std::string(argv[3]) == "-scale") {
        args.push_back(argv[2]); // ROM
        args.push_back(argv[2]); // title
        args.push_back(argv[4]); // scale

        // Remove file extention from title (title.ch8 => title)
        if (args[0].back() == '8')
            for (int i = 0; i < 4; i++)
                args[0].pop_back();
    }
    else {
        std::cerr << "Error: invalid usage.\n\t" << "usage: " << argv[0] << " -rom <FILE> -scale <SCALE_FACTOR>" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return args;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args = argparse(argc, argv);
    unsigned delay = 1;

    // Create interpreter and RUN RUN RUN RUN RUN!!!!!!!!!!!
    interpreter_8::Interpreter chip8_interpreter(args[0].c_str(), args[1].c_str(), delay, std::stoi(args[2]));
    chip8_interpreter.Run();

    return EXIT_SUCCESS;
}
