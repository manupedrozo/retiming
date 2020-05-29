#ifndef CIRCUITGEN
#define CIRCUITGEN

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <vector>
#include <random>
#include <math.h>

#include "cycle_finder.cpp" 
#include "types.h"

#ifdef CIRCUITGENDEBUG
#include <iostream>
#include <iomanip>
#include "graph_printer.cpp" 
#endif

//#define CIRCUITGENDEBUG

struct RandomCalculator {
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> edge_dis; // Uniform distribution for edge between two vertices.
    std::normal_distribution<> edge_weight_dis; // Normal distribution for edge weight.
    std::chi_squared_distribution<> vertex_weight_dis; // Chi-sq distribution for vertex weight.

    RandomCalculator(int edge_u, int edge_phi): gen(rd()),
                        edge_dis(0, 1), 
                        edge_weight_dis(edge_u, edge_phi), 
                        vertex_weight_dis(4.0) {}

    int vertex_weight() {
        return vertex_weight_dis(gen);
    }

    double edge() {
        return edge_dis(gen);
    }

    int edge_weight() {
        return std::abs(edge_weight_dis(gen));
    }

    int uniform(int from, int to) {
        std::uniform_real_distribution<> uniform_dis(from, to);
        return uniform_dis(gen);
    }
};

Graph generate_circuit(int vertex_count) {
    int edge_u = 0;
    int edge_phi = 5;
    // Probability of edge between two vertices (around 2 edges per vertex)
    double edge_p = 2.0 / vertex_count; 

    RandomCalculator rand(edge_u, edge_phi);

    std::vector<Edge> edges_v;

    Vertex *vertices = (Vertex *) malloc(sizeof(Vertex) * vertex_count);

    // Random circuit
    for (int i = 0; i < vertex_count; ++i) {
        vertices[i] = Vertex(rand.vertex_weight());

        // Random edges from current vertex
        double multiplier = 1;
        for (int j = 0; j < vertex_count; ++j) {
            if(i == j) continue;
            if(rand.edge() < edge_p * multiplier) {
                edges_v.push_back(Edge(i, j, rand.edge_weight()));
                --j; multiplier /= 3;// Reroll same edge (parallel edges), with 1/3 the chance
            } else {
                multiplier = 1;
            }
        }
    }

    // Check for unconnected graphs (components)
    {
        using namespace boost;
        typedef adjacency_list <vecS, vecS, undirectedS> BGLGraph;

        BGLGraph g(vertex_count);

        for(int i = 0; i < edges_v.size(); ++i) {
            int from = edges_v[i].from;
            add_edge(from, edges_v[i].to, g);
        }

        std::vector<int> component(vertex_count);
        int components = connected_components(g, &component[0]);

#ifdef CIRCUITGENDEBUG
        std::vector<int>::size_type i;
        std::cout << "Total number of components: " << components << std::endl;
        for (i = 0; i != component.size(); ++i)
            std::cout << "Vertex " << i <<" is in component " << component[i] << std::endl;
        std::cout << std::endl; 
#endif

        // Connect the components if there are multiple.
        if (components > 1) {
            std::vector<std::vector<int>> component_vertices(components, std::vector<int>());
            for (int i = 0; i < component.size(); ++i) {
                component_vertices[component[i]].push_back(i);
            }

            // Generate edges from a component to the following component
            for (int i = 0; i < components-1; ++i) {
                int from = rand.uniform(0, component_vertices[i].size()-0.01);
                int to = rand.uniform(0, component_vertices[i+1].size()-0.01);
                edges_v.push_back(Edge(component_vertices[i][from], component_vertices[i+1][to], rand.edge_weight()));
#ifdef CIRCUITGENDEBUG
                printf("Adding edge %d -> %d\n", component_vertices[i][from], component_vertices[i+1][to]);
#endif
            }
        }
    }

    int edge_count = edges_v.size();

    // Allocate and copy edges
    Edge *edges = (Edge *) malloc(sizeof(Edge) * edge_count);
    for (int i = 0; i < edge_count; ++i) {
        edges[i] = edges_v[i];
    }

    // Create graph
    Graph graph(vertices, edges, vertex_count, edge_count);

    // Find 0-weight cycles in circuit
    bool check_cycles = true;
    while (check_cycles) {
        std::vector<std::vector<Edge *>> cycles;
        find_zero_weight_cycles(&cycles, graph);

        int cycle_count = cycles.size();
        if(cycle_count == 0) check_cycles = false;

#ifdef CIRCUITGENDEBUG
        printf("cycles: %d\n", cycle_count);
        for(int i = 0; i < cycle_count; ++i) {
            printf("cycle: %d\n", i);
            for(int j = 0; j < cycle_count; ++j) {
                Edge *edge = cycles[i][j];
                printf("(%d, %d, [%d]) \n", edge->from, edge->to, edge->weight);
            }
            std::cout << "\n";
        }
#endif

        // We could be rerolling more than necessary if the same edge is in two different cycles.
        // Since these cycles are rare and the graph is random, this doesn't really matter.
        for(int i = 0; i < cycle_count; ++i) {
            // Reroll weight of a random edge in the cycle
            int e = rand.uniform(0, cycles[i].size()-0.01);
            int weight = rand.edge_weight();
            while(weight == 0) { // Make sure weight > 0
                weight = rand.edge_weight();
            }
            cycles[i][e]->weight = weight;

#ifdef CIRCUITGENDEBUG
            Edge *edge = cycles[i][e];
            printf("Rerolled (%d, %d, [0 -> %d]) \n", edge->from, edge->to, edge->weight);
#endif
        }
    }

    return graph;
}


#ifdef CIRCUITGENDEBUG
int main_gen() {
    Graph graph = generate_circuit(10);
    to_dot(graph, "random_circuit.dot");

    free(graph.vertices);
    free(graph.edges); 

    return 0;
}
#endif

#endif
