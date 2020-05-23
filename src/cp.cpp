#ifndef CPALG
#define CPALG

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <stdlib.h>
#include <list>
#include <algorithm>
#include <vector>
#include <set>
#include <queue>
#include "types.h"

//#define CPDEBUG

#ifdef CPDEBUG
#include <iostream>
#endif


/*
 * CP ALGORITHM
 * Computes clock period deltas for the given graph into the deltas array.
 * deltas array has to be of length = vertex_count
 * Returns an array of deltas in vertex order.
 */
int cp(Graph &graph, int *deltas) {
    using namespace boost;
    typedef adjacency_list<vecS, vecS, directedS> BGLGraph;
    typedef boost::graph_traits<BGLGraph>::vertex_descriptor BGLVertex;

#ifdef CPDEBUG
        printf("CP:\n");
#endif

    Edge *edges = graph.edges;
    Vertex *vertices = graph.vertices;
    int vertex_count = graph.vertex_count;

    //Get edges with weight 0 and their vertices
    std::list<Edge *> g0_edges;
    for (int i = graph.edge_count-1; i >= 0; --i) {
        if(edges[i].weight == 0) {
            g0_edges.push_front(&edges[i]);
        }
    }

    int g0_edge_count = g0_edges.size();
    BGLGraph g(vertex_count);

    for(Edge *edge: g0_edges) {
        add_edge(edge->from, edge->to, g);
        //printf("Added edge %d -> %d", edge->from, edge->to);
    }

    std::vector<BGLVertex> sorted_vertices;
    topological_sort(g, std::back_inserter(sorted_vertices));

    int c = 0; //clock period (max delta)

    for (std::vector<BGLVertex>::reverse_iterator i=sorted_vertices.rbegin(); i!=sorted_vertices.rend(); ++i) {
        int vertex = *i;

        int delta = 0;
        for (Edge *edge : g0_edges) {
            if(edge->to == vertex) {
                int d_delta = deltas[edge->from];
                if(d_delta > delta)
                    delta = d_delta;
            }
        }
        delta += vertices[vertex].weight;
        deltas[vertex] = delta;
        if(delta > c) c = delta; //set clock period to current delta if greater

#ifdef CPDEBUG
        printf("%d: delta = %d\n", vertex, delta);
#endif
    }

    return c;     
}

#ifdef CPDEBUG
int main_cp() {
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

    Graph graph(vertices, edges, vertex_count, edge_count);

    int deltas[vertex_count];
    int c = cp(graph, deltas);

    printf("C = %d\n", c);

    for (int i = 0; i < vertex_count; ++i) {
        printf("delta %d: %d\n", i, deltas[i]);
    }

    c = cp_(graph, deltas);
    printf("C = %d\n", c);

    for (int i = 0; i < vertex_count; ++i) {
        printf("delta %d: %d\n", i, deltas[i]);
    }

    return 0;
}

int main_cp2() {
    const int vertex_count = 8;
    const int edge_count = 10;

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
        Edge(0, 1, 0),
        Edge(1, 7, 1),
        Edge(7, 2, 0),
        Edge(2, 5, 0),
        Edge(2, 3, 0),
        Edge(5, 6, 0),
        Edge(5, 4, 1),
        Edge(6, 4, 0),
        Edge(4, 3, 0),
        Edge(3, 0, 1),
    };

    Graph graph(vertices, edges, vertex_count, edge_count);

    int deltas[vertex_count];
    int c = cp(graph, deltas);

    return 0;
}
#endif

#endif
