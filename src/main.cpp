#include <iostream>
#include <thread>
#include <filesystem>
#include <optional>

#include "p-ranav/argparse.hpp"

#include "map.hpp"
#include "assessment.hpp"
#include "planning.hpp"

namespace fs = std::filesystem;

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
    program.add_argument("-f", "--file")
           .help("path to file with map")
           .default_value(std::string { "NNMap.pbf" })
           .nargs(1);

    program.add_argument("-l", "--log")
           .help("enable logging")
           .default_value(false)
           .implicit_value(true);

    program.add_argument("--recache")
           .help("recache map")
           .default_value(false)
           .implicit_value(true);

    try {
        program.parse_args(argc, argv);
    }
    catch (std::exception& err) {
        std::cout << program;
        exit(0);
    }

    if (!fs::exists(".cache")) {
        fs::create_directory(".cache");
    }
    fs::path filename = program.get<std::string>("--file");
    auto cname = filename.stem();
    auto recache = program["--recache"] == true
                   || !fs::exists((fs::path { ".cache" } / cname).concat("-map.dmp"))
                   || !fs::exists((fs::path { ".cache" } / cname).concat("-gph.dmp"));
    auto houses = program.get<int>("houses"),
        facilities = program.get<int>("facilities");
    graphs::Map map;

    if (auto result = graphs::import_map_from_pbf(filename, recache)) {
        map = result.value();
    } else {
        std::cerr << "Map not found" << std::endl;
        return 1;
    }

    /*
     * Start tasks in separate threads.
     */
    auto first = std::thread { assessment, std::ref(map), houses, facilities };
    // auto second = std::thread { planning, std::ref(map), houses, facilities };

    first.join();
    // second.join();
};