#ifndef CIRCUITGEN
#define CIRCUITGEN

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <math.h>

#include "cycle_finder.cpp" 
#include "types.h"
#include "graph_printer.cpp" 

#define CIRCUITGENDEBUG

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

    double edge_p = 0.4; // Probability of edge between two vertices

    RandomCalculator rand;

    std::vector<Edge> edges_v;

    Vertex *vertices = (Vertex *) malloc(sizeof(Vertex) * vertex_count);

    //int prev = vertex_count-1;
    for (int i = 0; i < vertex_count; ++i) {
        vertices[i] = Vertex(rand.vertex_weight());
        //edges_v.push_back(Edge(prev, i, rand.edge_weight())); // Connect outer circuit

        //if(i > 0) {
        // Random edges from current vertex to higher-index vertices
        double multiplier = 1;
        for (int j = i+1; j < vertex_count; ++j) {
            if(rand.edge() < edge_p * multiplier) {
                edges_v.push_back(Edge(i, j, rand.edge_weight()));
                --j; multiplier /= 3;// Reroll same edge, with 1/3 the chance
            } else {
                multiplier = 1;
            }
        }
        //}

        // Edge to first vertex
        if(i > 0) {
            bool f = true;
            while(f) {
                if(rand.edge() < edge_p * multiplier) {
                    edges_v.push_back(Edge(i, 0, rand.edge_weight()));
                    multiplier /= 3; f = true;
                } else { 
                    f = false;
                }
            }
        }

        //prev = i;
    }

    // Check for unconnected graphs (components)
    int components;
    std::vector<int> component(vertex_count);
    {
        using namespace boost;
        typedef adjacency_list <vecS, vecS, undirectedS> BGLGraph;

        BGLGraph g(vertex_count);

        for(int i = 0; i < edges_v.size(); ++i) {
            int from = edges_v[i].from;
            add_edge(from, edges_v[i].to, g);
        }

        components = connected_components(g, &component[0]);

#ifdef CIRCUITGENDEBUG
        std::vector<int>::size_type i;
        std::cout << "Total number of components: " << components << std::endl;
        for (i = 0; i != component.size(); ++i)
            std::cout << "Vertex " << i <<" is in component " << component[i] << std::endl;
        std::cout << std::endl; 
#endif
    }

    // Calculate root by doing a shortest nall path and checking from which node I can get to the most nodes.
    int root = 0;
    {
        typedef adjacency_list<vecS, vecS, directedS, no_property, property<edge_weight_t, int>> BGLGraph;

        BGLGraph g(vertex_count);

        for(int i = 0; i < edges_v.size(); ++i) {
            int from = edges_v[i].from;
            add_edge(from, edges_v[i].to, edges_v[i].weight, g);
        }
        int maxweight = MAXINT;
        std::vector<std::vector<int>> D(vertex_count, std::vector<int>(vertex_count));

        johnson_all_pairs_shortest_paths(g, D, distance_inf(maxweight).distance_zero(0));

#ifdef CIRCUITGENDEBUG
        printf("All pairs shortest paths:\n");
        std::cout << "     ";
        for (int k = 0; k < vertex_count; ++k)
            std::cout << std::setw(5) << k;
        std::cout << "\n" << std::endl;
        for (int i = 0; i < vertex_count; ++i) {
            std::cout << i << " -> ";
            for (int j = 0; j < vertex_count; ++j) {
                int d = D[i][j];
                if (d == maxweight)
                    std::cout << std::setw(5) << "inf";
                else
                    std::cout << std::setw(5) << D[i][j];
            }
            std::cout << std::endl;
        }
#endif
        // Get root
        int reach = 0;
        int c_reach = 0;
        for (int i = 0; i < vertex_count; ++i) {

            // TODO we probably want to set the weight of vertices that dont have incoming edges to 0, but maybe do this after connecting the components.

            for (int j = 0; j < vertex_count; ++j) {
                int d = D[i][j];
                if(d < maxweight) {
                    ++c_reach;
                }
            }
            if(c_reach == vertex_count) {
                root = i;
                break;
            } else if (c_reach > reach) {
                root = i;
                reach = c_reach;
            }
            c_reach = 0;
        }

        // Set root weight to 0
        vertices[root].weight = 0;

#ifdef CIRCUITGENDEBUG
        printf("Selected root: %d\n", root);
#endif
        
        // Connect the components if there are multiple.
        if (components > 1) {
            std::vector<std::vector<int>> component_vertices(components, std::vector<int>());
            for (int i = 0; i < component.size(); ++i) {
                component_vertices[component[i]].push_back(i);
            }

            int root_component = component[root];
            // Edge from the root_component to a node in another component
            for (int i = 0; i < components; ++i) {
                if(i == root_component) continue;
                int from = rand.uniform(0, component_vertices[root_component].size()-1);
                int to = rand.uniform(0, component_vertices[i].size()-1);
                edges_v.push_back(Edge(component_vertices[root_component][from], component_vertices[i][to], rand.edge_weight()));
#ifdef CIRCUITGENDEBUG
                printf("Adding edge %d -> %d\n", component_vertices[root_component][from], component_vertices[i][to]);
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

    return 0;
}

#endif
