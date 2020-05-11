#include <boost/config.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>

#include "types.h"
#include "wd.cpp" 

using namespace boost;

struct BellmanResult {
    bool r; //no negative cycle
    int *distance; //array of distances from root_vertex to each vertex
};

/**
 * Bellman ALGORITHM
 * Internally builds a bgl graph from the provided vertices and edges.
 * Vertices weights are ignored.
 * Returns a BellmanResult.
 */
BellmanResult bellman(Graph &graph, const int root_vertex) {
    typedef adjacency_list <vecS, vecS, directedS, no_property, property<edge_weight_t, int>> BGLGraph;

    Edge *edges = graph.edges;
    int vertex_count = graph.vertex_count;

    BGLGraph g(vertex_count);

    for(int i = 0; i < graph.edge_count; ++i) {
        add_edge(edges[i].from, edges[i].to, edges[i].weight, g);
    }

    int *distance = make_array(vertex_count, 0);//, std::numeric_limits<int>::max());
    distance[root_vertex] = 0;

    bool r = bellman_ford_shortest_paths(g, distance_map(distance).root_vertex(root_vertex));

    return {r, distance};
}

int main() {
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

    // OPT1 start

    int c = 13; //Hardcoded for now

    //Binary search D
    // while(bot < top)
    // c = c_candidates[b];
    // do all the edge stuff and bellman
    // save result.distance as the best so far if result.r = true (no need calculate retimed edges yet, wait until the while is done)
    // keep going in the search until the while is done, the last saved result is the best one

    //Get edges for 7.1 (the same for every c)
    Edge *opt_edges1 = (Edge *) malloc(sizeof(Edge) * edge_count);
    for (int i = 0; i < edge_count; ++i) {
        opt_edges1[i] = Edge(edges[i].to, edges[i].from, edges[i].weight);
    }

    //Get edges for 7.2
    std::list<Edge> opt_edges2;
    WDEntry entry;
    for (int u = 0; u < vertex_count; ++u) {
        for (int v = 0; v < vertex_count; ++v) {
            entry = WD[u * vertex_count + v];
            //check the requirements on D(u,v)
            if(entry.D > c && (entry.D - vertices[u].weight <= c) && (entry.D - vertices[v].weight <= c)) {
                //add the edge v -> u with weight W(u, v) - 1
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

    Graph opt_graph(vertices, opt_edges, vertex_count, opt_edge_count);

    BellmanResult result = bellman(opt_graph, root_vertex);
    int *distance = result.distance;

    if (result.r)
        for (int i = 0; i < vertex_count; ++i)
            std::cout << i <<  ": " << std::setw(3) << distance[i] << " " << std::endl;
    else
        std::cout << "Negative cycle" << std::endl;

    //Calculate edge weights of the retimed graph: wr(e) = w(e) + r(v) - r(u)
    //Reusing opt_edges1
    Edge *retimed_edges = opt_edges1;
    for (int i = 0; i < edge_count; ++i) {
        int from = edges[i].from;
        int to = edges[i].to;
        retimed_edges[i] = Edge(from, to, edges[i].weight + distance[to] - distance[from]);
    }

    //return retimed_edges;
    
    //OPT1 end


    /*
    */
    printf("--- OPT EDGES --- \n");
    for (int i = 0; i < opt_edge_count; ++i) {
        printf("(%d, %d, [%d]) \n", opt_edges[i].from, opt_edges[i].to, opt_edges[i].weight);
    }

    printf("--- RETIMED EDGES --- \n");
    for (int i = 0; i < edge_count; ++i) {
        printf("(%d, %d, [%d]) \n", retimed_edges[i].from, retimed_edges[i].to, retimed_edges[i].weight);
    }
    

    free(distance);


    return 0;
}

int test_correlator1() {
    //bgl example
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

    BellmanResult result = bellman(graph, root_vertex);
    int *distance = result.distance;

    if (result.r)
        for (int i = 0; i < vertex_count; ++i)
            std::cout << i <<  ": " << std::setw(3) << distance[i] << " " << std::endl;
    else
        std::cout << "Negative cycle" << std::endl;

    free(distance);

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

    BellmanResult result = bellman(graph, root_vertex);
    int* distance = result.distance;

    if (result.r)
        for (int i = 0; i < vertex_count; ++i)
            std::cout << i <<  ": " << std::setw(3) << distance[i] << " " << std::endl;
    else
        std::cout << "Negative cycle" << std::endl;

    free(distance);

    return 0;
}

/*
{
    if (r)
        for (int i = 0; i < N; ++i)
            std::cout << name[i] << ": " << std::setw(3) << distance[i] << " " << std::endl;
    else
        std::cout << "negative cycle" << std::endl;

    //dot stuff
    std::ofstream dot_file("bellman.dot");
    dot_file << "digraph D {\n"
        << "  rankdir=LR\n"
        << "  size=\"5,3\"\n"
        << "  ratio=\"fill\"\n"
        << "  edge[style=\"bold\"]\n" << "  node[shape=\"circle\"]\n";

    {

        graph_traits < BGLGraph >::edge_iterator ei, ei_end;

        for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
            graph_traits < BGLGraph >::edge_descriptor e = *ei;
            graph_traits < BGLGraph >::vertex_descriptor
                u = source(e, g), v = target(e, g);
            // VC++ doesn't like the 3-argument get function, so here
            // we workaround by using 2-nested get()'s.
            dot_file << name[u] << " -> " << name[v]
                << "[label=\"" << get(get(edge_weight, g), e) << "\"";
            //if (parent[v] == u)
            //std::cout << ", color=\"black\"";
            //else
            dot_file << ", color=\"grey\"";
            dot_file << "]";
        }
    }
    dot_file << "}";
    return EXIT_SUCCESS;
}
*/

