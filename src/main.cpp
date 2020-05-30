#include <iostream>
#include <thread>

#include "graph.hpp"
#include "argparse.hpp"
#include "assessment.hpp"
#include "planning.hpp"

int main(int argc, const char** argv) {
    argparse::ArgumentParser program { "graphs" };

    /*
     * Main tasks.
     */
    program.add_argument("houses")
           .help("number of houses")
           .action([](const std::string& value) { return std::stoi(value); });

    program.add_argument("facilities")
           .help("number of facilities")
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

    auto houses = program.get<int>("houses"),
        facilities = program.get<int>("facilities");
    auto map = graph::import_map("NNMap.pbf");

    /*
     * Start tasks in separate threads.
     */
    auto first = std::thread { assessment, std::ref(map), houses, facilities };
    auto second = std::thread { planning, std::ref(map), houses, facilities };

    first.join();
    second.join();
};