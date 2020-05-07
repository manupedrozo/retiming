#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <fstream>
using namespace boost;

struct Vertex {
    int weight;

    Vertex() {}

    Vertex(int weight) {
        this->weight = weight;
    }
};

struct Edge {
    int from;
    int to;
    int weight;

    Edge(int from, int to, int weight) {
        this->from = from;
        this->to = to;
        this->weight = weight;
    }
};

typedef adjacency_list<vecS, vecS, directedS, Vertex, property<edge_weight_t, int>> Graph;

int main() {
    const int vertex_count = 8;
    const int edge_count = 11;

    Vertex vertices[] = {
        Vertex(0),
        Vertex(3),
        Vertex(3),
        Vertex(3),
        Vertex(3),
        Vertex(7),
        Vertex(7),
        Vertex(7),
    };

    Edge edges[] = { 
        Edge(0, 1, 1),
        Edge(1, 2, 1),
        Edge(1, 7, 0),
        Edge(2, 3, 1),
        Edge(2, 6, 0),
        Edge(3, 4, 1),
        Edge(3, 5, 0),
        Edge(4, 5, 0),
        Edge(5, 6, 0),
        Edge(6, 7, 0),
        Edge(7, 0, 0),
    };

    Graph g(vertex_count);

    for(int i = 0; i < vertex_count; ++i) {
        g[i].weight = vertices[i].weight;
    }

    for(int i = 0; i < edge_count; ++i) {
        add_edge(edges[i].from, edges[i].to, edges[i].weight, g);
    }

    std::ofstream outputFile("graph.dot");
    
    write_graphviz(outputFile, g, make_label_writer(get(&Vertex::weight, g)), make_label_writer(get(edge_weight, g)));
    outputFile.close();
}
