#ifndef GRAPHPRINTER
#define GRAPHPRINTER

#include <fstream>
#include <string>
#include "types.h"

void print_graph(Graph &graph, std::string name) {
    printf("------ Graph %s ------ \n", name.c_str());
    printf("--- VERTEX --- \n");
    for (int i = 0; i < graph.vertex_count; ++i) {
        printf("r(V%d) = %d\n", i, graph.vertices[i].weight);
    }
    printf("--- EDGES --- \n");
    for (int i = 0; i < graph.edge_count; ++i) {
        printf("(%d, %d, [%d]) \n", graph.edges[i].from, graph.edges[i].to, graph.edges[i].weight);
    }
}

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

#endif
