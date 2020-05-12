//#define OPT1DEBUG

#include "types.h"
#include "wd.cpp" 
#include "opt1.cpp" 
#include "graph_printer.cpp" 

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

        to_dot(retimed, "opt1.dot");

        free(retimed.vertices);
        free(retimed.edges);
    } else {
        printf("--- No retiming found --- \n");
    }

    free(WD);

    return 0;
}
