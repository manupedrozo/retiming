#ifndef RETCHECKER
#define RETCHECKER

#include "types.h"

//#define DEBUGRETCHECKER

#ifdef DEBUGRETCHECKER
#include <iostream>
#endif

bool check_legal(Graph &graph, Graph &retimed, int c, WDEntry *WD) {
    Vertex *retimed_vertices = retimed.vertices;
    Edge *retimed_edges = retimed.edges;
    Edge *edges = graph.edges;
    int edge_count = graph.edge_count;
    int vertex_count = graph.vertex_count;

    bool ok = true;
    for (int i = 0; i < edge_count; ++i) {
        Edge edge = edges[i];
        int u = edge.from;
        int v = edge.to;

        // W1
        if(retimed_edges[i].weight < 0) {
#ifdef DEBUGRETCHECKER
            printf("Edge: (%d, %d, [%d]) has negative weight\n", retimed_edges[i].from, retimed_edges[i].to, retimed_edges[i].weight);
#endif
            ok = false;
        }

        // 7.1
        if(retimed_vertices[u].weight - retimed_vertices[v].weight > edge.weight) {
#ifdef DEBUGRETCHECKER
            printf("Edge: (%d, %d, [%d]) Fails condition 7.1\n", retimed_edges[i].from, retimed_edges[i].to, retimed_edges[i].weight);
#endif
            ok = false;
        }

        //7.2
        WDEntry entry = WD[u * vertex_count + v];
        if(entry.D > c && retimed_vertices[u].weight - retimed_vertices[v].weight > entry.W - 1) {
#ifdef DEBUGRETCHECKER
            printf("Edge: (%d, %d, [%d]) Fails condition 7.2\n", retimed_edges[i].from, retimed_edges[i].to, retimed_edges[i].weight);
#endif
            ok = false;
        }

        if(!ok) return false;
    }
    return true;
}

#endif
