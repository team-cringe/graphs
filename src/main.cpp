#include <iostream>

#include "argparse.hpp"
#include "assessment.hpp"

int main(int argc, const char** argv) {
    argparse::ArgumentParser program { "graphs" };

    /*
     * Main tasks.
     */
    program.add_argument("nodes")
           .help("buildings nodes")
           .action([](const std::string& value) { return std::stoi(value); });

    program.add_argument("objects")
           .help("infrastructure objects")
           .action([](const std::string& value) { return std::stoi(value); });

    /*
     * Optional arguments.
     */
    program.add_argument("-l", "--log")
           .help("enable logging")
           .default_value(false)
           .implicit_value(true);

    try {
        program.parse_args(argc, argv);
    }
    catch (std::exception& err) {
        std::cout << program;
        exit(0);
    }

    assessment::nearest(program.get<int>("nodes"), program.get<int>("objects"));
};