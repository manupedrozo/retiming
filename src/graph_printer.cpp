#include <fstream>
#include <string>
#include "types.h"

void to_dot(Graph &graph, std::string path) {

    std::ofstream dot_file(path);
    dot_file << "digraph D {\n"
        << "  rankdir=LR\n"
        << "  size=\"5,3\"\n"
        << "  ratio=\"fill\"\n"
        << "  edge[style=\"bold\"]\n" << "  node[shape=\"circle\"]\n";

    Edge *edges = graph.edges;
    Vertex *vertices = graph.vertices;

    for(int i = 0; i < graph.vertex_count; ++i) {
        Vertex vertex = vertices[i];
        dot_file << i << "[label=\"(" << i << ", " << vertex.weight << ")\" , color = \"black\"]\n";
    }
    for(int i = 0; i < graph.edge_count; ++i) {
        Edge edge = edges[i];
        dot_file << edge.from << " -> " << edge.to << "[label=\"" << edge.weight << "\" , color = \"black\"]\n";
    }
    dot_file << "}";

    dot_file.close();
}
