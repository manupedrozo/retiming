#ifndef OPTALG
#define OPTALG

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <iomanip>
#include <iostream>
#include <set>

#include "types.h"
#include "wd.cpp" 
#include "feas.cpp" 
#include "graph_printer.cpp" 

//#define OPT1DEBUG

using namespace boost;

//TODO@remove
struct BellmanResult {
    bool r; //no negative cycle
    int *distance; //array of distances from root_vertex to each vertex
};

/**
 * Bellman ALGORITHM
 * Internally builds a bgl graph from the provided vertices and edges.
 * Vertices weights are ignored.
 * Result is stored into the distance array (distance from root_vertex to every other vertex).
 * Returns true if no negative cycle was found.
 */
bool bellman(Graph &graph, int *distance, const int root_vertex) {
    typedef adjacency_list <vecS, vecS, directedS, no_property, property<edge_weight_t, int>> BGLGraph;

    Edge *edges = graph.edges;
    int vertex_count = graph.vertex_count;

    BGLGraph g(vertex_count);

    for(int i = 0; i < graph.edge_count; ++i) {
        add_edge(edges[i].from, edges[i].to, edges[i].weight, g);
    }

    //int *distance = make_array(vertex_count, 0);//, std::numeric_limits<int>::max());
    distance[root_vertex] = 0;

    bool r = bellman_ford_shortest_paths(g, distance_map(distance).root_vertex(root_vertex));

    return r;
}

struct OptResult {
    bool r; //retiming found
    int c; //minimized clock period
    Graph graph; //retimed graph
};

/**
 * OPT1 ALGORITHM
 * Uses Bellman.
 * Returns an OptResult.
 */
OptResult opt1(Graph &graph, WDEntry *WD) {

    int vertex_count = graph.vertex_count;
    int edge_count = graph.edge_count;
    Edge *edges = graph.edges;
    Vertex *vertices = graph.vertices;

    WDEntry entry;

    //Get different c values from D(u,v)
    int* c_candidates;
    int c_count;
    {
        std::set<int> c_candidates_set;
        for (int u = 0; u < vertex_count; ++u) {
            for (int v = 0; v < vertex_count; ++v) {
                entry = WD[u * vertex_count + v];
                c_candidates_set.insert(entry.D);
            }
        }
        c_count = c_candidates_set.size();
        c_candidates = (int *) malloc(sizeof(int) * c_count);
        int k = 0;
        for (int c: c_candidates_set) {
            c_candidates[k] = c;
            ++k;
        }
    }

    //Get edges for 7.1 (the same for every c)
    Edge *opt_edges1 = (Edge *) malloc(sizeof(Edge) * edge_count);
    for (int i = 0; i < edge_count; ++i) {
        opt_edges1[i] = Edge(edges[i].to, edges[i].from, edges[i].weight);
    }

    int c = -1; //best c
    int *distance = (int *) malloc(sizeof(int) * vertex_count);;//distance array of best c
    int *tmp_distance = (int *) malloc(sizeof(int) * vertex_count);//distance array of current c
    int *aux_distance;//aux for swapping between distance and temp_distance

    //Binary search ordered c values
    int b, current_c;
    int bot = 0;
    int top = c_count;
    while(bot < top) {
        b = (top + bot)/2;
        current_c = c_candidates[b];

#ifdef OPT1DEBUG
        printf("[Binary search] b: %d\tbot: %d\ttop: %d\tc = %d\n", b, current_c, bot, top);
#endif

        //Get edges for 7.2
        std::list<Edge> opt_edges2;
        for (int u = 0; u < vertex_count; ++u) {
            for (int v = 0; v < vertex_count; ++v) {
                entry = WD[u * vertex_count + v];
                //check the requirements on D(u,v)
                if(entry.D > current_c && (entry.D - vertices[u].weight <= current_c) && (entry.D - vertices[v].weight <= current_c)) {
                    //add the edge v -> u with weight W(u, v) - 1
                    opt_edges2.push_front(Edge(v, u, entry.W - 1));
                }
            }
        }

        //Merge edges into a single array
        int opt_edge_count = edge_count + opt_edges2.size();
        //probably wanna resize the same opt_edges instead of fully allocating each pass
        Edge *opt_edges = (Edge *) malloc(sizeof(Edge) * opt_edge_count);
        int k;
        for (k = 0; k < edge_count; ++k) {
            opt_edges[k] = opt_edges1[k];
        }

        for (Edge edge : opt_edges2) {
            opt_edges[k] = edge;
            ++k;
        } 

#ifdef OPT1DEBUG
        printf("--- OPT EDGES --- \n");
        for (int i = 0; i < opt_edge_count; ++i) {
        printf("(%d, %d, [%d]) \n", opt_edges[i].from, opt_edges[i].to, opt_edges[i].weight);
        }
#endif

        Graph opt_graph(vertices, opt_edges, vertex_count, opt_edge_count);

        //Run bellman
        bool r = bellman(opt_graph, tmp_distance, 0);//root_vertex); 

        free(opt_edges);

        if(r) { 
            top = b - 1;

            //save c
            c = current_c;

            //switch distance array
            aux_distance = distance;
            distance = tmp_distance;
            tmp_distance = aux_distance;
#ifdef OPT1DEBUG
            std::cout << "Retiming found" << std::endl; 
#endif
        } else {
            bot = b + 1;
#ifdef OPT1DEBUG
            std::cout << "Negative cycle" << std::endl;
#endif
        }
    }

    //If no retiming was found, return base graph as best retiming.
    OptResult result = {false, c, graph};

    if(c >= 0) {
        //a retiming c was found, make the retimed graph and return.

        //Calculate edge weights of the retimed graph: wr(e) = w(e) + r(v) - r(u)
        //Reusing opt_edges1
        Edge *retimed_edges = opt_edges1;
        for (int i = 0; i < edge_count; ++i) {
            int from = edges[i].from;
            int to = edges[i].to;
            retimed_edges[i] = Edge(from, to, edges[i].weight + distance[to] - distance[from]);
        }

        //Calculate r(Vi) for each vertex: distance to that vertex.
        Vertex *retimed_vertices = (Vertex *) malloc(sizeof(Vertex) * vertex_count);
        for (int i = 0; i < vertex_count; ++i) {
            retimed_vertices[i] = Vertex(distance[i]);
        }

        Graph retimed(retimed_vertices, retimed_edges, vertex_count, edge_count);
        result = {true, c, retimed};
    } 

    free(c_candidates);
    free(tmp_distance);
    free(distance);

    return result;
}

/**
 * OPT2 ALGORITHM
 * Uses feas.
 * Returns an OptResult.
 */
OptResult opt2(Graph &graph, WDEntry *WD) {

    int vertex_count = graph.vertex_count;
    int edge_count = graph.edge_count;
    Edge *edges = graph.edges;
    Vertex *vertices = graph.vertices;

    int *deltas = (int *) malloc(sizeof(int) * vertex_count);

    WDEntry entry;

    //Get different c values from D(u,v)
    int* c_candidates;
    int c_count;
    {
        std::set<int> c_candidates_set;
        for (int u = 0; u < vertex_count; ++u) {
            for (int v = 0; v < vertex_count; ++v) {
                entry = WD[u * vertex_count + v];
                c_candidates_set.insert(entry.D);
            }
        }
        c_count = c_candidates_set.size();
        c_candidates = (int *) malloc(sizeof(int) * c_count);
        int k = 0;
        for (int c: c_candidates_set) {
            c_candidates[k] = c;
            ++k;
        }
    }

    Graph retimed_graph;
    int c = -1; //best c

    //Binary search ordered c values
    int b, current_c;
    int bot = 0;
    int top = c_count;
    while(bot < top) {
        b = (top + bot)/2;
        current_c = c_candidates[b];

#ifdef OPT2DEBUG
        printf("[Binary search] b: %d\tbot: %d\ttop: %d\tc = %d\n", b, current_c, bot, top);
#endif

        //Run feas
        FeasResult feas_result = feas(graph, current_c, deltas);

        if(feas_result.r) { 
            top = b - 1;

            if(c >= 0) {
                free(retimed_graph.vertices);
                free(retimed_graph.edges);
            }

            //save c
            c = current_c;

            retimed_graph = feas_result.graph;

#ifdef OPT2DEBUG
            std::cout << "Retiming found for c: " << current_c << std::endl; 
            print_graph(retimed_graph, "retimed");
#endif
        } else {
            bot = b + 1;

            free(feas_result.graph.vertices);
            free(feas_result.graph.edges);

#ifdef OPT2DEBUG
            std::cout << "No retiming found for c: " << current_c << std::endl;
#endif
        }
    }

    free(deltas);
    free(c_candidates);

    if(c >= 0) return {true, c, retimed_graph};
    else return {false, c, graph};;
}

int main_opt1() {
    //Correlator1
    const int vertex_count = 8;
    const int edge_count = 11;
    const int root_vertex = 0;

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

    WDEntry* WD = wd_algorithm(graph);

    OptResult result = opt1(graph, WD);

    if(result.r) {
        Graph retimed = result.graph;
        printf("--- RETIMED EDGES --- \n");
        for (int i = 0; i < edge_count; ++i) {
            printf("(%d, %d, [%d]) \n", retimed.edges[i].from, retimed.edges[i].to, retimed.edges[i].weight);
        }
        printf("--- r(Vi) --- \n");
        for (int i = 0; i < vertex_count; ++i) {
            printf("r(V%d) = %d\n", i, retimed.vertices[i].weight);
        }

        free(retimed.vertices);
        free(retimed.edges);
    } else {
        printf("--- No retiming found --- \n");
    }

    free(WD);

    return 0;
}

int test_correlator1() {
    //correlator1
    const int vertex_count = 8;
    const int edge_count = 16;
    const int root_vertex = 0;

    //Vertex weights don't matter for bellman
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

    //Edges for correlator1 already calculated for bellman (Theorem 7)
    Edge edges[] = { 
        //inverted existing edges (7.1)
        Edge(1, 0, 1),
        Edge(2, 1, 1),
        Edge(7, 1, 0),
        Edge(3, 2, 1),
        Edge(6, 2, 0),
        Edge(4, 3, 1),
        Edge(5, 3, 0),
        Edge(5, 4, 0),
        Edge(6, 5, 0),
        Edge(7, 6, 0),
        Edge(0, 7, 0),

        //5 constraints (7.2)
        Edge(5, 1, 1),
        Edge(6, 5, -1),
        Edge(7, 6, -1),
        Edge(3, 7, 2),
        Edge(6, 7, 1),
    };
    

    Graph graph(vertices, edges, vertex_count, edge_count);

    int distance[vertex_count];
    bool r = bellman(graph, distance, root_vertex);

    if (r)
        for (int i = 0; i < vertex_count; ++i)
            std::cout << i <<  ": " << std::setw(3) << distance[i] << " " << std::endl;
    else
        std::cout << "Negative cycle" << std::endl;

    return 0;
}

int test_bgl_example() {
    //bgl example
    const int vertex_count = 5;
    const int edge_count = 10;
    const int root_vertex = 4;

    //Vertex weights don't matter for bellman
    Vertex vertices[] = {
        Vertex(0),
        Vertex(0),
        Vertex(0),
        Vertex(0),
        Vertex(0),
    };

    Edge edges[] = { 
        Edge(0, 3, -4),
        Edge(0, 2,  8),
        Edge(0, 1,  5),
        Edge(1, 0, -2),
        Edge(2, 3,  9),
        Edge(2, 1, -3),
        Edge(3, 1,  7),
        Edge(3, 4,  2),
        Edge(4, 0,  6),
        Edge(4, 2,  7),
    };

    Graph graph(vertices, edges, vertex_count, edge_count);

    int distance[vertex_count];
    bool r = bellman(graph, distance, root_vertex);

    if (r)
        for (int i = 0; i < vertex_count; ++i)
            std::cout << i <<  ": " << std::setw(3) << distance[i] << " " << std::endl;
    else
        std::cout << "Negative cycle" << std::endl;

    return 0;
}

#endif
