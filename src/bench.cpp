#include "types.h"
#include "graph_printer.cpp" 
#include "circuit_generator.cpp" 
#include "wd.cpp" 
#include "opt.cpp" 
#include "cp.cpp"
#include "feas.cpp"
#include "retiming_checker.cpp"

#include <benchmark/benchmark.h>

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
};

/**
 * Benchmark cp algorithm
 *  - Almost E^2, although the paper says its O(E) -> BGL topological sort is O(E+V)
 */
void BM_cp(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = graphs[index];
    int *deltas = (int *) malloc(sizeof(int) * graph.vertex_count);
    for(auto _ : state) {
        cp(graph, deltas);
    }
    free(deltas);
    state.SetComplexityN(graph.vertex_count + 2*graph.edge_count);
}

/**
 * Benchmark feas algorithm
 */
void BM_feas(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = graphs[index];
    WDEntry *WD = wd_algorithm(graph);
    OptResult result = opt1(graph, WD);
    int *deltas = (int *) malloc(sizeof(int) * graph.vertex_count);
    int target_c = result.c;
    if(result.r) {
        free(result.graph.vertices);
        free(result.graph.edges);
    }
    free(WD);
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
 * Benchmark bellman algorithm when solving a system of linear inequalities, just like in the OPT1 algorithm, using the original clock period as the target c.
*/
void BM_bellman(benchmark::State& state) {
    int vertex_count = state.range(0);
    for(auto _ : state) {
        state.PauseTiming();

        Graph graph = generate_circuit(vertex_count);
        Edge *edges = graph.edges;
        Vertex *vertices = graph.vertices;
        int edge_count = graph.edge_count;

        int *deltas = (int *) malloc(sizeof(int) * vertex_count);
        int c = cp(graph, deltas);
        free(deltas);

        WDEntry *WD = wd_algorithm(graph);

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
        state.ResumeTiming();

        bellman(bell_graph, distance);

        state.PauseTiming();
        free(opt_edges);
        free(distance);
        free(graph.vertices);
        free(graph.edges);
        state.ResumeTiming();

    }

    state.SetComplexityN(state.range(0));
} 

/**
 * Benchmark opt1 algorithm
 */
void BM_opt1(benchmark::State& state) {
    int vertex_count = state.range(0);
    int total_vertices = 0;
    int total_edges = 0;
    int i = 0;
    for(auto _ : state) {
        state.PauseTiming();
        Graph graph = generate_circuit(vertex_count);
        total_vertices += vertex_count;
        total_edges += graph.edge_count;
        i++;
        state.ResumeTiming();

        WDEntry *WD = wd_algorithm(graph);
        OptResult result = opt1(graph, WD);

        state.PauseTiming();
        if(result.r) {
            free(result.graph.vertices);
            free(result.graph.edges);
        }
        free(graph.vertices);
        free(graph.edges);
        free(WD);
        state.ResumeTiming();
    }
    //printf("i: %d\tvertices: %d\tedges: %d\n", i, total_vertices, total_edges);
    state.SetComplexityN(state.range(0));
}

/**
 * Benchmark opt2 algorithm
 */
void BM_opt2(benchmark::State& state) {
    int vertex_count = state.range(0);
    int total_vertices = 0;
    int total_edges = 0;
    int i = 0;
    for(auto _ : state) {
        state.PauseTiming();
        Graph graph = generate_circuit(vertex_count);
        total_vertices += vertex_count;
        total_edges += graph.edge_count;
        i++;
        state.ResumeTiming();

        WDEntry *WD = wd_algorithm(graph);
        OptResult result = opt2(graph, WD);

        state.PauseTiming();
        if(result.r) {
            free(result.graph.vertices);
            free(result.graph.edges);
        }
        free(graph.vertices);
        free(graph.edges);
        free(WD);
        state.ResumeTiming();
    }
    //printf("i: %d\tvertices: %d\tedges: %d\n", i, total_vertices, total_edges);
    state.SetComplexityN(state.range(0));
}

/**
 * Benchmark opt2 algorithm
 */
void BM_opt22(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = graphs[index];
    //printf("vertices: %d\tedges: %d\n", graph.vertex_count, graph.edge_count);
    for(auto _ : state) {

        WDEntry *WD = wd_algorithm(graph);
        OptResult result = opt2(graph, WD);

        state.PauseTiming();
        if(result.r) {
            free(result.graph.vertices);
            free(result.graph.edges);
        }
        free(WD);
        state.ResumeTiming();
    }
    state.SetComplexityN(graph.vertex_count);
}

/**
 * Benchmark opt1 algorithm
 */
void BM_opt12(benchmark::State& state) {
    int index = state.range(0);
    Graph graph = graphs[index];
    //printf("vertices: %d\tedges: %d\n", graph.vertex_count, graph.edge_count);
    for(auto _ : state) {

        WDEntry *WD = wd_algorithm(graph);
        OptResult result = opt1(graph, WD);

        state.PauseTiming();
        if(result.r) {
            free(result.graph.vertices);
            free(result.graph.edges);
        }
        free(WD);
        state.ResumeTiming();
    }
    state.SetComplexityN(graph.vertex_count);
}

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

    state.SetComplexityN(graph.vertex_count + graph.edge_count);
}

//BENCHMARK(BM_InsertSort)->RangeMultiplier(2)->Range(1<<10, 1<<18)->Complexity();

//BENCHMARK(BM_bellman)->RangeMultiplier(2)->Range(1<<3, 1<<11)->Complexity();
//BENCHMARK(BM_opt1)->RangeMultiplier(2)->Range(1<<3, 1<<11)->Complexity();
//BENCHMARK(BM_opt2)->RangeMultiplier(2)->Range(1<<3, 1<<11)->Complexity();
//BENCHMARK(BM_opt22)->DenseRange(0, 6)->Complexity();
//BENCHMARK(BM_opt12)->DenseRange(0, 6)->Complexity();
BENCHMARK(BM_topology)->DenseRange(0, 8)->Complexity(benchmark::oN);
BENCHMARK(BM_cp)->DenseRange(0, 8)->Complexity(benchmark::oN);
//BENCHMARK(BM_feas)->DenseRange(0, 8)->Complexity(benchmark::oN);



BENCHMARK_MAIN();

