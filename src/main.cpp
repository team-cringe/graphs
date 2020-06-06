#include <thread>
#include <filesystem>
#include <optional>

#include <fmt/format.h>
#include <benchmark/benchmark.h>

#include "p-ranav/argparse.hpp"

#include "map.hpp"
#include "assessment.hpp"
#include "planning.hpp"

#ifdef GRAPHS_RUN_BENCHMARK
static void BM_ImportMap(benchmark::State& state) {
    for (auto _: state) {
        auto map = graphs::import_map_from_pbf("NNMap.pbf", false);
    }
}

auto map = graphs::import_map_from_pbf("NNMap.pbf", false).value();

static void BM_SelectBuildings(benchmark::State& state) {
    for (auto _: state) {
        map.select_random_houses(100);
        map.select_random_facilities(100);
    }
}

auto houses_1 = map.select_random_houses(1);
auto facilities_1 = map.select_random_facilities(5);

static void BM_ShortestPathsWithTraces(benchmark::State& state) {
    for (auto _: state) {
        auto house = *houses_1.begin();
        map.shortest_paths_with_trace(house, facilities_1);
    }
}

auto houses_2 = map.select_random_houses(1);
auto facilities_2 = map.select_random_facilities(100);

static void BM_ShortestPaths(benchmark::State& state) {
    for (auto _: state) {
        auto house = *houses_2.begin();
        map.shortest_paths(house, facilities_2);
    }
}

BENCHMARK(BM_ImportMap)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_SelectBuildings)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_ShortestPaths)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_ShortestPathsWithTraces)->Unit(benchmark::kMillisecond);
BENCHMARK_MAIN();
#endif

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
    program.add_argument("-i", "--import")
           .help("import map from .csv or .pbf")
           .default_value(std::string { "NNMap.pbf" })
           .nargs(1);

    program.add_argument("-e", "--export")
           .help("export graph as .csv")
           .default_value(false)
           .implicit_value(true);

    program.add_argument("-r", "--recache")
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

    if (!fs::exists(".cache")) { fs::create_directory(".cache"); }
    fs::path filename = program.get<std::string>("--import");
    auto extension = filename.extension();

    graphs::Map map;

    if (extension == ".csv") {
        map = import_map_from_csv(filename);
        auto paths = map.dijkstra(Node { 0 });
        fmt::print("From: {}\n", 0);
        for (const auto& path: paths) {
            auto[to, d] = path;
            fmt::print("\tto: {} ({} m)\n", to.id(), d);
        }
    } else if (extension == ".pbf") {
        auto cache_name = filename.stem();
        auto recache = program["--recache"] == true
                       || !fs::exists((fs::path { ".cache" } / cache_name) += "-map.dmp")
                       || !fs::exists((fs::path { ".cache" } / cache_name) += "-gph.dmp");
        auto houses = program.get<int>("houses"), facilities = program.get<int>("facilities");
        if (auto result = graphs::import_map_from_pbf(filename, recache)) {
            map = result.value();
        } else {
            fmt::print(stderr, "Map not found");
            return 1;
        }

        /*
         * Start tasks in separate threads.
         */
        auto first = std::thread { assessment, std::ref(map), houses, facilities };
        auto second = std::thread { planning, std::ref(map), houses, facilities };

        first.join();
        second.join();
    } else {
        fmt::print(stderr, "Map format not recognised");
        return 1;
    }

    if (program["--export"] == true) {
        graphs::export_map_to_csv(map, "Graph.csv");
    }
};
