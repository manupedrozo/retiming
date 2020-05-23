#ifndef CYCLEFINDER
#define CYCLEFINDER

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/hawick_circuits.hpp>
#include <vector>
#include "types.h"

//#define CYCLEFINDERDEBUG
#ifdef CYCLEFINDERDEBUG
#include <iostream>
#endif


struct cycle_visitor
{
    std::vector<std::vector<Edge *>> *cycles;
    Edge *edges;
    int edge_count;

    cycle_visitor(std::vector<std::vector<Edge *>> *cycles, Edge *edges, int edge_count){
        this->edges = edges; 
        this->edge_count = edge_count;
        this->cycles = cycles;
    }

    template <typename Path, typename Graph>
    void cycle(Path const& p, Graph const& g) {
        using namespace boost;
        if (p.empty())
            return;

        // Get the property map containing the vertex indices
        typedef typename boost::property_map<Graph, boost::vertex_index_t>::const_type IndexMap;

        IndexMap indices = get(boost::vertex_index, g);

        int cycle_length = p.size();

        std::vector<Edge *> cycle(cycle_length);

        // Check if the cycle edges are weight 0
        int u = get(indices, p[cycle_length-1]);
        int v;
        for(int i = 0; i < cycle_length; ++i) {
            v = get(indices, p[i]);

            // Check if there is at least one 0 weight edge between the vertices
            // This is not optimized at all 
            bool zero_edge = false;
            for(int j = 0; j < edge_count; ++j) {
                Edge edge = edges[j];

                if(edge.from == u && edge.to == v) {
                    if(edge.weight == 0) {
                        zero_edge = true;
                        cycle[i] = &edges[j];
#ifdef CYCLEFINDERDEBUG
                        printf("Adding %d, %d [%d]\n", edge.from, edge.to, edge.weight);
#endif
                    }
                }
            }
            if(!zero_edge) return;

            u = v;
        }

        // 0 weighted cycle
        cycles->push_back(cycle);
#ifdef CYCLEFINDERDEBUG
        printf("Added cycle: %d\n", (int) cycles->size());
#endif
    }
};

void find_zero_weight_cycles(std::vector<std::vector<Edge *>> *cycles, Graph &graph) {
    using namespace boost;
    typedef adjacency_list<vecS, vecS, directedS, no_property, no_property> BGLGraph;
    BGLGraph g(graph.vertex_count);
    int edge_count = 0;
    for(int i = 0; i < graph.edge_count; ++i) {
        if(graph.edges[i].weight == 0) {
            add_edge(graph.edges[i].from, graph.edges[i].to, g);
            ++edge_count;
        }
    }
#ifdef CYCLEFINDERDEBUG
        printf("Checking cycles in graph with %d 0-weight edges\n", edge_count);
#endif

    cycle_visitor visitor(cycles, graph.edges, graph.edge_count);
    boost::hawick_circuits(g, visitor);
}

#ifdef CYCLEFINDERDEBUG
int main_cycle() {
    const int vertex_count = 3;
    const int edge_count = 6;

    Vertex vertices[] = {
        Vertex(0),
        Vertex(3),
        Vertex(3),
    };

    Edge edges[] = { 
        Edge(0, 1, 0),
        Edge(0, 1, 0),
        Edge(1, 0, 0),
        Edge(1, 0, 0),
        Edge(1, 2, 0),
        Edge(2, 0, 0),
    };

    Graph graph(vertices, edges, vertex_count, edge_count);

    std::vector<std::vector<Edge *>> cycles;
    find_zero_weight_cycles(&cycles, graph);

    printf("cycles: %d\n", (int) cycles.size());
    for(int i = 0; i < cycles.size(); ++i) {
        for(int j = 0; j < cycles[i].size(); ++j) {
            Edge *edge = cycles[i][j];
            printf("(%d, %d, [%d]) \n", edge->from, edge->to, edge->weight);
        }
        std::cout << "\n";
    }

    return 0;
}
#endif

#endif
