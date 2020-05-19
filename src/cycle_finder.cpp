#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/hawick_circuits.hpp>

#include <vector>
#include <iostream>

#include "types.h"

using namespace boost;

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

            // This is not optimized at all 
            for(int j = 0; j < edge_count; ++j) {
                Edge edge = edges[j];
                // Assuming there cannot be multiple edges between two vertices.
                if(edge.from == u && edge.to == v) {
                    if(edge.weight > 0) return;
                    else {
                        //printf("Adding %d, %d [%d]\n", edge.from, edge.to, edge.weight);
                        cycle[i] = &edges[j];
                    }
                }
            }

            u = v;
        }

        // 0 weighted cycle
        cycles->push_back(cycle);
        //printf("Added cycle: %d\n", cycle_length);
    }
};

void find_zero_weight_cycles(std::vector<std::vector<Edge *>> *cycles, Graph &graph) {
    typedef adjacency_list<vecS, vecS, directedS, no_property, no_property> BGLGraph;
    BGLGraph g(graph.vertex_count);
    for(int i = 0; i < graph.edge_count; ++i) {
        add_edge(graph.edges[i].from, graph.edges[i].to, g);
    }

    cycle_visitor visitor(cycles, graph.edges, graph.edge_count);
    boost::hawick_circuits(g, visitor);
}

int main_cycle() {
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
        Edge(0, 1, 0),
        Edge(1, 2, 0),
        Edge(1, 7, 0),
        Edge(2, 3, 0),
        Edge(2, 6, 1),
        Edge(3, 4, 0),
        Edge(3, 5, 0),
        Edge(4, 5, 0),
        Edge(5, 6, 0),
        Edge(6, 7, 0),
        Edge(7, 0, 0),
    };

    Graph graph(vertices, edges, vertex_count, edge_count);

    std::vector<std::vector<Edge *>> cycles;
    find_zero_weight_cycles(&cycles, graph);

    /*
    typedef adjacency_list<vecS, vecS, directedS, no_property, no_property> BGLGraph;
    BGLGraph g(vertex_count);
    for(int i = 0; i < edge_count; ++i) {
        add_edge(edges[i].from, edges[i].to, g);
    }

    cycle_visitor visitor(&cycles, edges, edge_count);
    boost::hawick_circuits(g, visitor);
    */

    printf("cycles: %d\n", (int) cycles.size());
    for(int i = 0; i < cycles.size(); ++i) {
        for(int j = 0; j < cycles[i].size(); ++j) {
            Edge *edge = cycles[i][j];
            printf("(%d, %d, [%d]) \n", edge->from, edge->to, edge->weight);
        }
        std::cout << "\n";
    }

    return EXIT_SUCCESS;
}

