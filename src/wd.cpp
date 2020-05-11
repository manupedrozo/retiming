#include <boost/config.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>

#include "types.h"

using namespace boost;

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
 * Internally builds a bgl graph from the provided vertices and edges.
 * The edges built into the graph are weighted according to the WD algorithm requirements.
 * Returns a WDEntry matrix.
 */
WDEntry* wd_algorithm(Graph &graph) {
    //typedef the graph
    typedef adjacency_list<vecS, vecS, directedS, no_property, property<edge_weight_t, WDEdgeWeight, property<edge_weight2_t, WDEdgeWeight>>> BGLGraph;

    Edge* edges = graph.edges;
    Vertex* vertices = graph.vertices;
    int vertex_count = graph.vertex_count;
    int edge_count = graph.edge_count;

    //create graph
    BGLGraph g(vertex_count);

    //add properties to vertices
    /*
    for(int i = 0; i < vertex_count; ++i) {
        g[i].weight = vertices[i].weight;
    }
    */

    //add edges
    for(int i = 0; i < edge_count; ++i) {
        int from = edges[i].from;
        add_edge(from, edges[i].to, WDEdgeWeight((edges[i].weight), -vertices[from].weight), g);
    }

    //print graph to check its properly built
    /*
    std::cout << "VERTICES:" << std::endl; 
    for(int i = 0; i < vertex_count; ++i) {
        Vertex v = g[i];
        std::cout << "Vertex " << i << " [weight: " << v.weight << "]"<< std::endl; 
    }

    std::cout << "EDGES:" << std::endl; 
    graph_traits<BGLGraph>::edge_iterator ei, ei_end;
    for (tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei)
        std::cout << source(*ei, g) << " -> " << target(*ei, g)
            << " [weight: " << get(edge_weight, g)[*ei].weight << "]" << std::endl;
    */


    int maxweight = std::numeric_limits<int>::max();
    WDEdgeWeight max = WDEdgeWeight(maxweight, maxweight);

    /* TODO@remove unnecessary
    std::vector<WDEdgeWeight >d(vertex_count, (WDEdgeWeight(maxweight, maxweight)));
    distance_map(&d[0]).distance_inf....
    */

    //distance matrix, initialized with max value
    std::vector<std::vector<WDEdgeWeight>> D(vertex_count, std::vector<WDEdgeWeight>(vertex_count));

    //call johnson all shortest paths
    johnson_all_pairs_shortest_paths(g, D, distance_inf(max).distance_zero(WDEdgeWeight(0, 0)));

    //compute result into a WDEntry matrix
    int size = vertex_count * vertex_count;
    WDEntry* WD = (WDEntry*) malloc(sizeof(WDEntry) * size);

    for (int i = 0; i < vertex_count; ++i) { 
        for (int j = 0; j < vertex_count; ++j) {
            WDEntry* entry = &WD[i * vertex_count + j];
            entry->W = D[i][j].weight;
            entry->D = vertices[j].weight - D[i][j].negdu;
            //printf("%d,%d: %d\n", i, j , WD[i * vertex_count + j].W);
        }
    } 

    return WD;
}


int _main() {

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
}
