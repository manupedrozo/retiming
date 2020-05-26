
//#define DEBUGSPACEBENCH

#include "space_bench.cpp"
#define SPACEBENCH

#include "cp.cpp"
#include "wd.cpp" 
#include "opt.cpp" 
#include "circuit_generator.cpp" 
#include "types.h"

const int graph_count = 10;

Graph graphs[] = {
    generate_circuit(1<<3),
    generate_circuit(1<<4),
    generate_circuit(1<<5),
    generate_circuit(1<<6),
    generate_circuit(1<<7),
    generate_circuit(1<<8),
    generate_circuit(1<<9),
    generate_circuit(1<<10),
    generate_circuit(1<<11),
    generate_circuit(1<<12),
};

/**
 * Benchmark CP algorithm
 * - O(V+E) ~ 1.60N
 * - Depends on the amount of 0 weight edges
 */
void SBM_cp() {
    double N = 0;
    SpaceBench benchmarks[graph_count];

    printf("CP Benchmark:\n");
    for(int i = 0; i < graph_count; ++i) {
        space_bench = &benchmarks[i];
        Graph graph = graphs[i];
        printf("cp/%d\tvertices: %d, edges: %d\n", i, graph.vertex_count, graph.edge_count);
        space_bench->push_stack();
        space_bench->allocated(sizeof(Vertex) * graph.vertex_count, true, VERTEX);
        space_bench->allocated(sizeof(Edge) * graph.edge_count, true, EDGE);

        int *deltas = (int *) malloc(sizeof(int) * graph.vertex_count);
        space_bench->allocated(sizeof(int) * graph.vertex_count, true, INT, "deltas");

        cp(graph, deltas);

        free(deltas);

        space_bench->deallocated(sizeof(int) * graph.vertex_count, INT);

        space_bench->pop_stack();
        space_bench->print_state();
        N += space_bench->ON(sizeof(Vertex) * graph.vertex_count + sizeof(Edge) * graph.edge_count);// O(V+E)
        printf("\n");
    }

    N /= graph_count;
    printf("cp: %.2f N\n", N);
    printf("\n ---------- \n");
}

/**
 * Benchmark WD algorithm
 * - O(V^2 + E) ~ 1.0N
 * - The V^2 being of type WDEntry (int, int)
 */
void SBM_wd() {
    double N = 0;
    SpaceBench benchmarks[graph_count];

    printf("WD Benchmark:\n");
    for(int i = 0; i < graph_count; ++i) {
        space_bench = &benchmarks[i];
        Graph graph = graphs[i];
        printf("wd/%d\tvertices: %d, edges: %d\n", i, graph.vertex_count, graph.edge_count);
        space_bench->push_stack();
        space_bench->allocated(sizeof(Vertex) * graph.vertex_count, true, VERTEX);
        space_bench->allocated(sizeof(Edge) * graph.edge_count, true, EDGE);


        WDEntry *WD = wd_algorithm(graph);

        free(WD);

        space_bench->deallocated(sizeof(WDEntry) * graph.vertex_count * graph.vertex_count, INT);

        space_bench->pop_stack();
        space_bench->print_state();
        N += space_bench->ON(sizeof(WDEntry) * graph.vertex_count * graph.vertex_count + sizeof(Edge) * graph.edge_count);
        printf("\n");
    }

    N /= graph_count;
    printf("wd: %.2f N\n", N);
    printf("\n ---------- \n");
}

/**
 * Benchmark OPT1  algorithm
 * - O(V^2 + E) ~ 1.1N (around 2.0N for small graphs)
 */
void SBM_opt1() {
    double N = 0;
    SpaceBench benchmarks[graph_count];

    printf("OPT1 Benchmark:\n");
    for(int i = 0; i < graph_count; ++i) {
        space_bench = &benchmarks[i];
        Graph graph = graphs[i];
        printf("opt1/%d\tvertices: %d, edges: %d\n", i, graph.vertex_count, graph.edge_count);
        space_bench->push_stack();
        space_bench->allocated(sizeof(Vertex) * graph.vertex_count, true, VERTEX);
        space_bench->allocated(sizeof(Edge) * graph.edge_count, true, EDGE);

        WDEntry *WD = wd_algorithm(graph);
        OptResult result = opt1(graph, WD);

        free(WD);
        space_bench->deallocated(sizeof(WDEntry) * graph.vertex_count * graph.vertex_count, INT);
        if(result.r) {
            free(result.graph.vertices);
            free(result.graph.edges);
            space_bench->deallocated(sizeof(Vertex) * graph.vertex_count, VERTEX);
            space_bench->deallocated(sizeof(Edge) * graph.edge_count, EDGE);
        }

        space_bench->pop_stack();
        printf("stack_size: %zu\n", space_bench->stack_size);
        space_bench->print_state();
        N += space_bench->ON(sizeof(WDEntry) * graph.vertex_count * graph.vertex_count + sizeof(Edge) * graph.edge_count);
        printf("\n");
    }

        

    N /= graph_count;
    printf("opt1: %.2f N\n", N);
    printf("\n ---------- \n");
}

int main() {
    SBM_cp();
    SBM_wd();
    SBM_opt1();
}
