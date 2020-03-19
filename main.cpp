#include <osmium/io/pbf_input.hpp>

int main() {
    osmium::io::File input { "NNMap.pbf" };
    osmium::io::Reader reader { input, osmium::osm_entity_bits::all };

    return 0;
};