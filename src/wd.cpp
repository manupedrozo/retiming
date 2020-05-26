#ifndef WDALG
#define WDALG

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <vector>
#include "types.h"

//define WDDEBUG

#ifdef WDDEBUG
#include <iomanip>
#include <iostream>
#endif

// Edge weight for the WD algorithm which is the pair (w(e), -d(u))
struct WDEdgeWeight { 
    int weight; //w(e)
    int negdu;  //-d(u)

    WDEdgeWeight(int weight, int negdu) {
        this->weight = weight;
        this->negdu = negdu;
    }

    WDEdgeWeight(){}

    bool operator<(const WDEdgeWeight& rhs) const {
        if (weight < rhs.weight || weight == rhs.weight && negdu < rhs.negdu)
            return true;
        return false;
    }

    WDEdgeWeight operator+(const WDEdgeWeight& rhs) const {
        return WDEdgeWeight(weight + rhs.weight, negdu + rhs.negdu);
    }

    WDEdgeWeight operator-(const WDEdgeWeight& rhs) const {
        return WDEdgeWeight(weight - rhs.weight, negdu - rhs.negdu);
    }

    bool operator==(const WDEdgeWeight &rhs) const {
        if(weight == rhs.weight && negdu == rhs.negdu)
            return true;
        return false;
    }
};

/**
 * WD ALGORITHM
 * Internally builds a bgl graph from the provided Graph.
 * The edges built into the graph are weighted according to the WD algorithm requirements.
 * Returns a WDEntry matrix.
 */
WDEntry* wd_algorithm(Graph &graph) {
    using namespace boost;
    //typedef the graph
    typedef adjacency_list<vecS, vecS, directedS, no_property, property<edge_weight_t, WDEdgeWeight, property<edge_weight2_t, WDEdgeWeight>>> BGLGraph;

#ifdef SPACEBENCH
    space_bench->push_stack();
#endif

    Edge* edges = graph.edges;
    Vertex* vertices = graph.vertices;
    int vertex_count = graph.vertex_count;
    int edge_count = graph.edge_count;

    //create graph
    BGLGraph g(vertex_count);

    //add edges
    for(int i = 0; i < edge_count; ++i) {
        int from = edges[i].from;
        add_edge(from, edges[i].to, WDEdgeWeight((edges[i].weight), -vertices[from].weight), g);
    }

    int maxweight = std::numeric_limits<int>::max();
    WDEdgeWeight max = WDEdgeWeight(maxweight, maxweight);

    //distance matrix, initialized with max value
    std::vector<std::vector<WDEdgeWeight>> D(vertex_count, std::vector<WDEdgeWeight>(vertex_count));

    //call johnson all shortest paths
    johnson_all_pairs_shortest_paths(g, D, distance_inf(max).distance_zero(WDEdgeWeight(0, 0)));

    //compute result into a WDEntry matrix
    int size = vertex_count * vertex_count;
    WDEntry* WD = (WDEntry*) malloc(sizeof(WDEntry) * size);

#ifdef SPACEBENCH
    space_bench->allocated(sizeof(int) * graph.vertex_count, false, BGLVERTEX, "BGL graph vertices");
    space_bench->allocated(sizeof(int) * 2 * graph.edge_count, false, BGLEDGE, "BGL graph edges");
    space_bench->allocated(sizeof(WDEntry) * graph.vertex_count * graph.vertex_count, true, INT, "WD matrix");
#endif

    for (int i = 0; i < vertex_count; ++i) { 
        for (int j = 0; j < vertex_count; ++j) {
            WDEntry* entry = &WD[i * vertex_count + j];
            entry->W = D[i][j].weight;
            entry->D = vertices[j].weight - D[i][j].negdu;
            //printf("%d,%d: %d\n", i, j , WD[i * vertex_count + j].W);
        }
    } 

#ifdef SPACEBENCH
    space_bench->pop_stack();
#endif

    return WD;
}


#ifdef WDDEBUG
int main_wd() {

    //Correlator1
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

    WDEntry* WD = wd_algorithm(graph);

    //print result
    std::cout << "---- W ----" << std::endl;
    std::cout << "     ";
    for (int k = 0; k < vertex_count; ++k)
        std::cout << std::setw(5) << k;
    std::cout << "\n" << std::endl;
    for (int i = 0; i < vertex_count; ++i) {
        std::cout << i << " -> ";
        for (int j = 0; j < vertex_count; ++j) {
            std::cout << std::setw(5) << WD[i * vertex_count + j].W;
        }
        std::cout << std::endl;
    }

    std::cout << "---- D ----" << std::endl;
    std::cout << "     ";
    for (int k = 0; k < vertex_count; ++k)
        std::cout << std::setw(5) << k;
    std::cout << "\n" << std::endl;
    for (int i = 0; i < vertex_count; ++i) { 
        std::cout << i << " -> ";
        for (int j = 0; j < vertex_count; ++j) {
            std::cout << std::setw(5) << WD[i * vertex_count + j].D;
        }
        std::cout << std::endl;
    }
    
    free(WD);

    return 0;
}
#endif

#endif
