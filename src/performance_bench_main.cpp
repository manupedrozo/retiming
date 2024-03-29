#include <benchmark/benchmark.h>
#include <math.h>

#include "types.h"
#include "graph_printer.cpp" 
#include "circuit_generator.cpp" 
#include "wd.cpp" 
#include "opt.cpp" 
#include "cp.cpp"
#include "feas.cpp"
#include "retiming_checker.cpp"

/*
const int graph_count = 7;
const int graph_max_index = graph_count-1;

Graph graphs[] = {
    generate_circuit(100),
    generate_circuit(200),
    generate_circuit(300),
    generate_circuit(400),
    generate_circuit(500),
    generate_circuit(600),
    generate_circuit(700),
};
*/

const int graph_count = 12;
const int graph_max_index = graph_count-1;

//Shared array of random graphs
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
    generate_circuit(1<<13),
    generate_circuit(1<<14),
};

int retimings[graph_count];

const int opt2_wc_graph_count = 7;
const int opt2_wc_graph_max_index = opt2_wc_graph_count-1;

//Generate a graph for the OPT2 worst case
Graph generate_opt2_wc_circuit(int vertex_count) {
    int edge_count = vertex_count;
    Vertex *vertices = (Vertex *) malloc(sizeof(Vertex) * vertex_count);
    Edge   *edges    = (Edge *)   malloc(sizeof(Edge) * edge_count);

    vertices[0] = Vertex(vertex_count-1);
    edges[0]    = Edge(edge_count-1, 0, 1);
    for (int i = 1; i < vertex_count; ++i) {
        vertices[i] = Vertex(vertex_count-i-1);
        edges[i] = Edge(i-1, i, 0);
    }

    return Graph(vertices, edges, vertex_count, edge_count);
}

//Shared array of graphs for opt2 worst case benchmarks
Graph opt2_wc_graphs[] = {
    generate_opt2_wc_circuit(1<<6),
    generate_opt2_wc_circuit(1<<7),
    generate_opt2_wc_circuit(1<<8),
    generate_opt2_wc_circuit(1<<9),
    generate_opt2_wc_circuit(1<<10),
    generate_opt2_wc_circuit(1<<11),
    generate_opt2_wc_circuit(1<<12),
};


/**
 * Benchmark our blg topology algorithm usage
 * - O(V + E)
 * - Using O(E) here to compare with cp
 */
void BM_topology(benchmark::State& state) {
    using namespace boost;
    typedef adjacency_list<vecS, vecS, directedS> BGLGraph;
    typedef boost::graph_traits<BGLGraph>::vertex_descriptor BGLVertex;

    int index = state.range(0);
    Graph graph = graphs[index];

    Edge *edges = graph.edges;
    Vertex *vertices = graph.vertices;
    int vertex_count = graph.vertex_count;

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
    }

    for(auto _ : state) {
        state.PauseTiming();
        std::vector<BGLVertex> sorted_vertices;
        state.ResumeTiming();

        topological_sort(g, std::back_inserter(sorted_vertices));
    }

    state.SetComplexityN(graph.edge_count);
}

/**
 * Benchmark CP algorithm
 *  - O(E) 
 */
void BM_cp(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = graphs[index];
    int *deltas = (int *) malloc(sizeof(int) * graph.vertex_count);
    for(auto _ : state) {
        cp(graph, deltas);
    }
    free(deltas);
    state.SetComplexityN(graph.edge_count);
}

/**
 * Benchmark WD algorithm
 * - Paper: O(log(V) * V^2 + V * E)
 * - Implementation: O(V * E * log(V))
 */
void BM_wd(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = graphs[index];
    for(auto _ : state) {
        WDEntry *WD = wd(graph);

        state.PauseTiming();
        free(WD);
        state.ResumeTiming();
    }
    state.SetComplexityN(graph.vertex_count * graph.edge_count * log(graph.vertex_count));
}

/**
 * Benchmark bellman algorithm when solving a system of linear inequalities, just like in the OPT1 algorithm, using the original clock period as the target c.
 * - O(V^3): bellman is O(V*E) with the max E being V^2 (max inequalities)
 * - Around 0.2 O(V^3) when adding all the inequalities
*/
void BM_bellman_full(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = graphs[index];
    Edge *edges = graph.edges;
    Vertex *vertices = graph.vertices;
    int vertex_count = graph.vertex_count;
    int edge_count = graph.edge_count;

    int *deltas = (int *) malloc(sizeof(int) * vertex_count);
    int c = cp(graph, deltas);
    free(deltas);

    WDEntry *WD = wd(graph);

    //Get edges for 7.1
    Edge *opt_edges1 = (Edge *) malloc(sizeof(Edge) * edge_count);
    for (int i = 0; i < edge_count; ++i) {
        opt_edges1[i] = Edge(edges[i].to, edges[i].from, edges[i].weight);
    }
    //Get edges for 7.2
    std::list<Edge> opt_edges2;
    for (int u = 0; u < vertex_count; ++u) {
        for (int v = 0; v < vertex_count; ++v) {
            WDEntry entry = WD[u * vertex_count + v];
            if(entry.D > c){// && (entry.D - vertices[u].weight <= c) && (entry.D - vertices[v].weight <= c)) {
                opt_edges2.push_front(Edge(v, u, entry.W - 1));
            }
        }
    }
    free(WD);
    //Merge edges into a single array
    int opt_edge_count = edge_count + opt_edges2.size();
    Edge *opt_edges = (Edge *) malloc(sizeof(Edge) * opt_edge_count);
    int k;
    for (k = 0; k < edge_count; ++k) {
        opt_edges[k] = opt_edges1[k];
    }
    for (Edge edge : opt_edges2) {
        opt_edges[k] = edge;
        ++k;
    }

    Graph bell_graph(vertices, opt_edges, vertex_count, opt_edge_count);
    int *distance = (int *) malloc(sizeof(int) * (vertex_count + 1));

    for(auto _ : state) {
        bellman(bell_graph, distance);
    }

    free(opt_edges);
    free(distance);

    state.SetComplexityN(pow(graph.vertex_count, 3));
} 
void BM_bellman(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = graphs[index];
    Edge *edges = graph.edges;
    Vertex *vertices = graph.vertices;
    int vertex_count = graph.vertex_count;
    int edge_count = graph.edge_count;

    int c = retimings[index];

    WDEntry *WD = wd(graph);

    //Get edges for 7.1
    Edge *opt_edges1 = (Edge *) malloc(sizeof(Edge) * edge_count);
    for (int i = 0; i < edge_count; ++i) {
        opt_edges1[i] = Edge(edges[i].to, edges[i].from, edges[i].weight);
    }
    //Get edges for 7.2
    std::list<Edge> opt_edges2;
    for (int u = 0; u < vertex_count; ++u) {
        for (int v = 0; v < vertex_count; ++v) {
            WDEntry entry = WD[u * vertex_count + v];
            if(entry.D > c && (entry.D - vertices[u].weight <= c) && (entry.D - vertices[v].weight <= c)) {
                opt_edges2.push_front(Edge(v, u, entry.W - 1));
            }
        }
    }
    free(WD);
    //Merge edges into a single array
    int opt_edge_count = edge_count + opt_edges2.size();
    Edge *opt_edges = (Edge *) malloc(sizeof(Edge) * opt_edge_count);
    int k;
    for (k = 0; k < edge_count; ++k) {
        opt_edges[k] = opt_edges1[k];
    }
    for (Edge edge : opt_edges2) {
        opt_edges[k] = edge;
        ++k;
    }

    Graph bell_graph(vertices, opt_edges, vertex_count, opt_edge_count);
    int *distance = (int *) malloc(sizeof(int) * (vertex_count + 1));

    for(auto _ : state) {
        bellman(bell_graph, distance);
    }

    free(opt_edges);
    free(distance);

    state.SetComplexityN(pow(graph.vertex_count, 3));
}

/**
 * Benchmark opt1 algorithm
 * - O(V^3 * log(V))
 * - Usually less, see bellman
 */
void BM_opt1(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = graphs[index];
    //printf("vertices: %d\tedges: %d\n", graph.vertex_count, graph.edge_count);
    for(auto _ : state) {

        WDEntry *WD = wd(graph);
        OptResult result = opt1(graph, WD);

        state.PauseTiming();
        if(result.r) {
            retimings[index] = result.c;
            free(result.graph.vertices);
            free(result.graph.edges);
        }
        free(WD);
        state.ResumeTiming();
    }
    state.SetComplexityN(pow(graph.vertex_count, 3) * log(graph.vertex_count));
}

/**
 * Benchmark feas algorithm
 * - O(V * E)
 * - Checks out!
 */
void BM_feas(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = graphs[index];
    int target_c = retimings[index];
    int *deltas = (int *) malloc(sizeof(int) * graph.vertex_count);
    for(auto _ : state) {

        FeasResult feas_result = feas(graph, target_c, deltas);
        
        state.PauseTiming();
        if(feas_result.r) { 
                free(feas_result.graph.vertices);
                free(feas_result.graph.edges);
        }
        state.ResumeTiming();
    }
    free(deltas);
    state.SetComplexityN(graph.vertex_count * graph.edge_count);
}

/**
 * Benchmark opt2 algorithm
 * - O(V * E * log(V))
 */
void BM_opt2(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = graphs[index];
    for(auto _ : state) {

        WDEntry *WD = wd(graph);
        OptResult result = opt2(graph, WD);

        state.PauseTiming();
        if(result.r) {
            free(result.graph.vertices);
            free(result.graph.edges);
        }
        free(WD);
        state.ResumeTiming();
    }
    state.SetComplexityN(graph.vertex_count * graph.edge_count * log(graph.vertex_count));
}

/**
 * Benchmark opt2 worst case
 */
void BM_opt2_opt2_wc(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = opt2_wc_graphs[index];
    for(auto _ : state) {

        WDEntry *WD = wd(graph);
        OptResult result = opt2(graph, WD);

        state.PauseTiming();
        if(result.r) {
            free(result.graph.vertices);
            free(result.graph.edges);
        }
        free(WD);
        state.ResumeTiming();
    }
    state.SetComplexityN(graph.vertex_count * graph.edge_count * log(graph.vertex_count));
}

/**
 * Benchmark opt1 with opt2 worst case
 */
void BM_opt1_opt2_wc(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = opt2_wc_graphs[index];
    for(auto _ : state) {

        WDEntry *WD = wd(graph);
        OptResult result = opt1(graph, WD);

        state.PauseTiming();
        if(result.r) {
            free(result.graph.vertices);
            free(result.graph.edges);
        }
        free(WD);
        state.ResumeTiming();
    }
    state.SetComplexityN(pow(graph.vertex_count, 3) * log(graph.vertex_count));
}

//BENCHMARK(BM_bellman_full)->DenseRange(0, graph_max_index)->Complexity(benchmark::oN);

BENCHMARK(BM_topology)->DenseRange(0, graph_max_index)->Complexity(benchmark::oN);
BENCHMARK(BM_cp)      ->DenseRange(0, graph_max_index)->Complexity(benchmark::oN);

BENCHMARK(BM_wd)      ->DenseRange(0, graph_max_index)->Complexity(benchmark::oN);
BENCHMARK(BM_opt1)    ->DenseRange(0, graph_max_index)->Complexity(benchmark::oN);
BENCHMARK(BM_bellman) ->DenseRange(0, graph_max_index)->Complexity(benchmark::oN);

BENCHMARK(BM_feas)    ->DenseRange(0, graph_max_index)->Complexity(benchmark::oN);
BENCHMARK(BM_opt2)    ->DenseRange(0, graph_max_index)->Complexity(benchmark::oN);

BENCHMARK(BM_opt2_opt2_wc)    ->DenseRange(0, opt2_wc_graph_max_index)->Complexity(benchmark::oN);
BENCHMARK(BM_opt1_opt2_wc)    ->DenseRange(0, opt2_wc_graph_max_index)->Complexity(benchmark::oN);

//BENCHMARK_MAIN();
int main(int argc, char** argv)
{
    for(int i = 0; i < graph_count; ++i) {
        int zero_edges = 0;
        for(int j = 0; j < graphs[i].edge_count; ++j) {
            if(graphs[i].edges[j].weight == 0)
                ++zero_edges;
        }
        printf("Graph %d: vertices: %d, edges: %d, zero edges: %d\n", i, graphs[i].vertex_count, graphs[i].edge_count, zero_edges);
    }

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}

