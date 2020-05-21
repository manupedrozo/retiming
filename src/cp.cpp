#ifndef CPALG
#define CPALG

#include "types.h"
#include "stdlib.h"
#include <list>
#include "iostream"
#include <algorithm>
#include <vector>
#include <set>
#include <queue>

//#define CPDEBUG

/*
 * CP ALGORITHM
 * Computes clock period deltas for the given graph into the deltas array.
 * deltas array has to be of length = vertex_count
 * Returns an array of deltas in vertex order.
 */
int cp(Graph &graph, int *deltas) {
    Edge *edges = graph.edges;
    Vertex *vertices = graph.vertices;
    int vertex_count = graph.vertex_count;


    //Get edges with weight 0 and their vertices
    std::set<int> g0_vertices;
    std::list<Edge *> g0_edges;
    for (int i = graph.edge_count-1; i >= 0; --i) {
        if(edges[i].weight == 0) {
            g0_edges.push_front(&edges[i]);
            g0_vertices.insert(edges[i].from);
            g0_vertices.insert(edges[i].to);
        }
    }

    //Initialize deltas as 0
    //We could do this in the while loop below but it would overcomplicate things
    for (int i = 0; i < vertex_count; ++i) {
        deltas[i] = 0;
    }

    int c = 0; //clock period (max delta)

    //Instead of sorting and calculating each delta, we check if the vertex dependenies for calculating its delta are met.
    //If they are, the delta is calculated
    //If not, we check again on the next array pass

    std::vector<bool> calculated(vertex_count, false); //is delta calculated, indexed by vertex id

    std::set<int>::iterator it = g0_vertices.begin();
    int calccount = g0_vertices.size();
    while(calccount > 0) {
        int id = *it;
        if(calculated[id]){
            it++;
            if(it == g0_vertices.end()) it = g0_vertices.begin();
            continue; //already calculated
        }

#ifdef CPDEBUG
        printf("%d:", id);
#endif

        //get vertex dependencies
        bool satisfied = true;
        std::set<int> dependencies;
        for (Edge *edge : g0_edges) {
            if(edge->to == id) {
                int d = edge->from;
                if(!calculated[d]) {
                    satisfied = false;
                    break;
                }
                dependencies.insert(edge->from);
            }
        }
        //print dependencies
        /*
        printf("%d dependencies: \t", *it);
        for (int i = 0; i < deps; ++i) {
            printf("%d, ", dependencies[i]);
        }
        printf("\n");
         */

        if(satisfied) {
            //find max delta of dependencies
            int max_delta = 0;
            if(dependencies.size() > 0) {
                for (int d: dependencies) {
                    int d_delta = deltas[d];
                    if(d_delta > max_delta) max_delta = d_delta;
                }
            } 

            //calculate delta 
            deltas[id] = vertices[id].weight + max_delta;

            if(deltas[id] > c) c = deltas[id]; //set clock period to current delta if greater
            
            //mark as calculated
            calculated[id] = true;
            --calccount;

#ifdef CPDEBUG
            printf("dependencies satisfied, delta = %d\n", deltas[id]);
#endif
        } else {
#ifdef CPDEBUG
            printf("unsatisfied dendencies\n");
#endif
        } 

        it++;
        if(it == g0_vertices.end()) it = g0_vertices.begin();
    }

    return c;     
}

int main_cp1() {
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

    int deltas[vertex_count];
    int c = cp(graph, deltas);

    return 0;
}

int main_cp2() {
    const int vertex_count = 8;
    const int edge_count = 10;

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
        Edge(1, 7, 1),
        Edge(7, 2, 0),
        Edge(2, 5, 0),
        Edge(2, 3, 0),
        Edge(5, 6, 0),
        Edge(5, 4, 1),
        Edge(6, 4, 0),
        Edge(4, 3, 0),
        Edge(3, 0, 1),
    };

    Graph graph(vertices, edges, vertex_count, edge_count);

    int deltas[vertex_count];
    int c = cp(graph, deltas);

    return 0;
}

#endif
