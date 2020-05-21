#ifndef CIRCUITGEN
#define CIRCUITGEN

#include <iostream>
#include <vector>
#include <random>
#include <math.h>

#include "cycle_finder.cpp" 
#include "types.h"
#include "graph_printer.cpp" 

//#define CIRCUITGENDEBUG

struct RandomCalculator {
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<>  edge_dis; // Uniform distribution for edge between two vertices.
    std::normal_distribution<> edge_weight_dis; // Normal distribution for edge weight.
    std::chi_squared_distribution<> vertex_weight_dis; // Chi-sq distribution for vertex weight.

    RandomCalculator(): gen(rd()),
                        edge_dis(0, 1), 
                        edge_weight_dis(0, 1.2), 
                        vertex_weight_dis(3.0) {}

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

    double edge_p = 0.2; // Probability of edge between two vertices

    RandomCalculator rand;

    std::vector<Edge> edges_v;

    Vertex *vertices = (Vertex *) malloc(sizeof(Vertex) * vertex_count);

    int prev = vertex_count-1;
    for (int i = 0; i < vertex_count; ++i) {
        vertices[i] = Vertex(rand.vertex_weight());
        edges_v.push_back(Edge(prev, i, rand.edge_weight())); // Connect outer circuit

        // Random edges from current vertex to higher-index vertices
        for (int j = i+2; j < vertex_count; ++j) {
            if(rand.edge() < edge_p) {
                edges_v.push_back(Edge(i, j, rand.edge_weight()));
            }
        }
        // Edge to first vertex
        //if(i > 0 && i < vertex_count-1 && rand.edge() < edge_p) {
        //    edges_v.push_back(Edge(i, 0, rand.edge_weight()));
        //}

        prev = i;
    }

    int edge_count = edges_v.size();

    // Allocate and copy edges
    Edge *edges = (Edge *) malloc(sizeof(Edge) * edge_count);
    for (int i = 0; i < edge_count; ++i) {
        edges[i] = edges_v[i];
    }

    Graph graph(vertices, edges, vertex_count, edge_count);

    // Find 0-weight cycles in circuit
    std::vector<std::vector<Edge *>> cycles;
    find_zero_weight_cycles(&cycles, graph);

#ifdef CIRCUITGENDEBUG
    printf("cycles: %d\n", (int) cycles.size());
    for(int i = 0; i < cycles.size(); ++i) {
        printf("cycle: %d\n", i);
        for(int j = 0; j < cycles[i].size(); ++j) {
            Edge *edge = cycles[i][j];
            printf("(%d, %d, [%d]) \n", edge->from, edge->to, edge->weight);
        }
        std::cout << "\n";
    }
#endif

    // We could be rerolling more than necessary if the same edge is in two different cycles.
    // Since these cycles are rare and the graph is random, this doesn't really matter.
    for(int i = 0; i < cycles.size(); ++i) {
        // Reroll weight of a random edge in the cycle
        int e = rand.uniform(0, cycles[i].size()-1);
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

    return graph;
}

int main_gen() {
    Graph graph = generate_circuit(5);
    to_dot(graph, "random_circuit.dot");

    free(graph.vertices);
    free(graph.edges);
}

#endif
