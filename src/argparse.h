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
