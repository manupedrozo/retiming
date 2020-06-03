#ifndef FEASALG
#define FEASALG

#include "types.h"
#include "cp.cpp"

//#define FEASDEBUG

#ifdef FEASDEBUG
#include <iostream>
#endif

struct FeasResult {
    bool r; //retiming found
    int c; //minimized clock period
    Graph graph; //retimed graph
};

/**
 * FEAS ALGORITHM
 * Calculates a retiming of the circuit with clock period <= target_c
 * Uses CP algorithm
 * deltas: int array of vertex_count size to calculate CP algorithm
 * Returns a FeasResult
 */
FeasResult feas(Graph &graph, int target_c, int *deltas) {
#ifdef SPACEBENCH
    space_bench->push_stack();
#endif
    int vertex_count = graph.vertex_count;
    int edge_count = graph.edge_count;

    //Allocate and initialize r(v) = 0 for each vertex v 
    Vertex *retimed_vertices = (Vertex *) malloc(sizeof(Vertex) * vertex_count);
    for (int i = 0; i < vertex_count; ++i) {
        retimed_vertices[i] = Vertex(0);
    }
    
    Edge *retimed_edges = (Edge *) malloc(sizeof(Edge) * edge_count);

    //Build retimed graph (edges to be updated).
    Graph gr(graph.vertices, retimed_edges, vertex_count, edge_count);

    //compute and initialize retimed edges
    for (int j = 0; j < edge_count; ++j) {
        Edge edge = graph.edges[j];
        retimed_edges[j] = { edge.from, edge.to, edge.weight };
    }

#ifdef SPACEBENCH
    space_bench->allocated(sizeof(Vertex) * vertex_count, true, VERTEX, "retimed vertices");
    space_bench->allocated(sizeof(Edge) * edge_count, true, EDGE, "retimed edges");
#endif

    bool changed = true;

    //repeat |V|-1 times
    int i;
    for (i = 1; i < vertex_count && changed; ++i) {
        changed = false;

        //Run CP to calculate deltas
        cp(gr, deltas);

        //Increment r(v) with values > target_c
        for (int v = 0; v < vertex_count; ++v) {
            if(deltas[v] > target_c) {
                changed = true;
                ++retimed_vertices[v].weight; 
            }
        }

        //Update retimed edges
        for (int j = 0; j < edge_count; ++j) {
            Edge edge = graph.edges[j];
            retimed_edges[j].weight = edge.weight + retimed_vertices[edge.to].weight - retimed_vertices[edge.from].weight;
        }
    }

    //Run CP one last time
    int c = cp(gr, deltas);
 
    //Build retimed graph
    Graph retimed(retimed_vertices, retimed_edges, vertex_count, edge_count);

#ifdef SPACEBENCH
    space_bench->pop_stack();
#endif

    return { c <= target_c, c, retimed };
}

#ifdef FEASDEBUG
int main_feas() {
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

    int c = 13;//best retiming

    int deltas[vertex_count];
    FeasResult result = feas(graph, c, deltas);

    Graph retimed = result.graph;

    if(result.r) {
        printf("--- RETIMED EDGES --- \n");
        for (int i = 0; i < edge_count; ++i) {
            printf("(%d, %d, [%d]) \n", retimed.edges[i].from, retimed.edges[i].to, retimed.edges[i].weight);
        }
        printf("--- r(Vi) --- \n");
        for (int i = 0; i < vertex_count; ++i) {
            printf("r(V%d) = %d\n", i, retimed.vertices[i].weight);
        }

    } else {
        printf("--- No retiming found --- \n");
    }

    free(retimed.vertices);
    free(retimed.edges);

    return 0;
}
#endif

#endif
