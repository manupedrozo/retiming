#ifndef TYPES
#define TYPES

#include "stdlib.h"

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

struct Graph {
    Vertex* vertices;
    Edge* edges;
    int vertex_count;
    int edge_count;

    Graph(Vertex* vertices, Edge* edges, int vertex_count, int edge_count) {
        this->vertices = vertices;
        this->edges = edges;
        this->vertex_count = vertex_count;
        this->edge_count = edge_count;
    }

    Graph() {}
};

//packs a WD pair
struct WDEntry {
    int W;
    int D;
};

int *make_array(int length, int value) {
    int *array = (int *) malloc(sizeof(int)*length);
    for(int i = 0; i < length; ++i) {
        array[i] = value;
    }
    return array;
}

#endif
