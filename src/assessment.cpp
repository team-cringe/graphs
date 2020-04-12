#include <iostream>

#include <osmium/io/pbf_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

struct MyHandler: public osmium::handler::Handler {
    static void way(const osmium::Way& way) {
        std::cout << "Way " << way.id() << '\n';
        for (const auto& t: way.tags()) {
            std::cout << t.key() << '=' << t.value() << '\n';
        }
    }

    static void node(const osmium::Node& node) {
        std::cout << "Node "
                  << node.id() << '\t'
                  << node.location() << "\n\n";
    }
};

namespace assessment {
void nearest(int nodes, int objects) {
    (void)nodes, (void)objects;

    osmium::io::File input { "NNMap.pbf" };
    osmium::io::Reader reader { input, osmium::osm_entity_bits::all };

    MyHandler handler;
    osmium::apply(reader, handler);
    reader.close();
}
} // namespace assessment
