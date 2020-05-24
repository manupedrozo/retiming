//#define OPT1DEBUG
//#define OPT2DEBUG
//#define CIRCUITGENDEBUG
//#define CYCLEFINDERDEBUG
//#define FEASDEBUG
//#define CPDEBUG
#define DEBUGRETCHECKER

#include <iostream>
#include <iomanip>

#include "types.h"
#include "graph_printer.cpp" 
#include "circuit_generator.cpp" 
#include "wd.cpp" 
#include "opt.cpp" 
#include "cp.cpp"
#include "feas.cpp"
#include "retiming_checker.cpp"

void print_wd(WDEntry *WD, int vertex_count) {
    std::cout << "---- W ----" << std::endl;
    std::cout << "     ";
    for (int k = 0; k < vertex_count; ++k)
        std::cout << std::setw(5) << k;
    std::cout << "\n" << std::endl;
    for (int i = 0; i < vertex_count; ++i) {
        std::cout << i << " -> ";
        for (int j = 0; j < vertex_count; ++j) {
            int w = WD[i * vertex_count + j].W;
            if (w > MAXINT>>1)
                std::cout << std::setw(5) << "inf";
            else if (w < 0)
                std::cout << std::setw(5) << "-inf";
            else
                std::cout << std::setw(5) << w;
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
            int d = WD[i * vertex_count + j].D;
            if (d > MAXINT>>1)
                std::cout << std::setw(5) << "inf";
            else if (d < 0)
                std::cout << std::setw(5) << "-inf";
            else
                std::cout << std::setw(5) << d;       
        }
        std::cout << std::endl;
    }
}

int test_feas() {
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

    int c = 13;//best retiming

    int deltas[vertex_count];
    FeasResult result = feas(graph, c, deltas);

    Graph retimed = result.graph;

    if(result.r) {
        printf("C = %d\n", result.c);
        printf("--- RETIMED EDGES --- \n");
        for (int i = 0; i < edge_count; ++i) {
            printf("(%d, %d, [%d]) \n", retimed.edges[i].from, retimed.edges[i].to, retimed.edges[i].weight);
        }
        printf("--- r(Vi) --- \n");
        for (int i = 0; i < vertex_count; ++i) {
            printf("r(V%d) = %d\n", i, retimed.vertices[i].weight);
        }

        free(retimed.vertices);
        free(retimed.edges);
    } else {
        printf("--- No retiming found --- \n");
    }

    to_dot(retimed, "feas.dot");


    //free(deltas);
    return 0;
}

int test_opt1() {
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
        printf("C = %d\n", result.c);
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

int test_opt1_2() {
    //Correlator1
    const int vertex_count = 5;
    const int edge_count = 6;
    const int root_vertex = 0;

    Vertex vertices[] = {
        Vertex(1),
        Vertex(0),
        Vertex(2),
        Vertex(1),
        Vertex(0),
    };

    Edge edges[] = { 
        Edge(0, 4, 2),
        Edge(1, 4, 1),
        Edge(1, 2, 0),
        Edge(2, 4, 1),
        Edge(2, 4, 1),
        Edge(2, 3, 0),
    };


    Graph graph(vertices, edges, vertex_count, edge_count);

    to_dot(graph, "graph.dot");

    int *deltas = (int *) malloc(sizeof(int) * vertex_count);
    int c = cp(graph, deltas);
    printf("initial C = %d\n", c);
    free(deltas);

    WDEntry* WD = wd_algorithm(graph);

    print_wd(WD, vertex_count);

    std::cout << "------ OPT1 ------" << std::endl;
    OptResult result = opt1(graph, WD);

    if(result.r) {
        printf("C = %d\n", result.c);
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

    std::cout << "------ OPT2 ------" << std::endl;
    result = opt2(graph, WD);

    if(result.r) {
        printf("C = %d\n", result.c);
        Graph retimed = result.graph;
        printf("--- RETIMED EDGES --- \n");
        for (int i = 0; i < edge_count; ++i) {
            printf("(%d, %d, [%d]) \n", retimed.edges[i].from, retimed.edges[i].to, retimed.edges[i].weight);
        }
        printf("--- r(Vi) --- \n");
        for (int i = 0; i < vertex_count; ++i) {
            printf("r(V%d) = %d\n", i, retimed.vertices[i].weight);
        }

        to_dot(retimed, "opt2.dot");

        free(retimed.vertices);
        free(retimed.edges);
    } else {
        printf("--- No retiming found --- \n");
    }

    printf("original = %d\nretimed = %d\n", c, result.c);

    free(WD);

    return 0;

    return 0;
}

int test_opt2() {
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

    OptResult result = opt2(graph, WD);

    if(result.r) {
        printf("C = %d\n", result.c);
        Graph retimed = result.graph;
        printf("--- RETIMED EDGES --- \n");
        for (int i = 0; i < edge_count; ++i) {
            printf("(%d, %d, [%d]) \n", retimed.edges[i].from, retimed.edges[i].to, retimed.edges[i].weight);
        }
        printf("--- r(Vi) --- \n");
        for (int i = 0; i < vertex_count; ++i) {
            printf("r(V%d) = %d\n", i, retimed.vertices[i].weight);
        }

        to_dot(retimed, "opt2.dot");

        free(retimed.vertices);
        free(retimed.edges);
    } else {
        printf("--- No retiming found --- \n");
    }

    free(WD);

    return 0;
}

int test_random() {
    // Random circuit
    Graph graph = generate_circuit(50);
    int vertex_count = graph.vertex_count;
    int edge_count = graph.edge_count;

    to_dot(graph, "random_circuit.dot");

    int *deltas = (int *) malloc(sizeof(int) * vertex_count);
    int c = cp(graph, deltas);
    printf("initial C = %d\n", c);
    free(deltas);

    WDEntry* WD = wd_algorithm(graph);
 
    print_wd(WD, vertex_count);

    std::cout << "------ OPT1 ------" << std::endl;
    OptResult result = opt1(graph, WD);

    if(result.r) {
        printf("C = %d\n", result.c);
        Graph retimed = result.graph;
        printf("--- RETIMED EDGES --- \n");
        for (int i = 0; i < edge_count; ++i) {
            printf("(%d, %d, [%d]) \n", retimed.edges[i].from, retimed.edges[i].to, retimed.edges[i].weight);
        }
        printf("--- r(Vi) --- \n");
        for (int i = 0; i < vertex_count; ++i) {
            printf("r(V%d) = %d\n", i, retimed.vertices[i].weight);
        }

        to_dot(retimed, "random_circuit_retimed.dot");

        if(check_legal(graph, retimed, result.c, WD))
            printf("- LEGAL RETIMING - \n");
        else 
            printf("- ILLEGAL RETIMING!!! - \n");

        free(retimed.vertices);
        free(retimed.edges);
    } else {
        printf("--- No retiming found --- \n");
    }

    std::cout << "------ OPT2 ------" << std::endl;
    result = opt2(graph, WD);

    if(result.r) {
        printf("C = %d\n", result.c);
        Graph retimed = result.graph;
        printf("--- RETIMED EDGES --- \n");
        for (int i = 0; i < edge_count; ++i) {
            printf("(%d, %d, [%d]) \n", retimed.edges[i].from, retimed.edges[i].to, retimed.edges[i].weight);
        }
        printf("--- r(Vi) --- \n");
        for (int i = 0; i < vertex_count; ++i) {
            printf("r(V%d) = %d\n", i, retimed.vertices[i].weight);
        }

        to_dot(retimed, "random_circuit_retimed2.dot");

        if(check_legal(graph, retimed, result.c, WD))
            printf("- LEGAL RETIMING - \n");
        else 
            printf("- ILLEGAL RETIMING!!! - \n");

        free(retimed.vertices);
        free(retimed.edges);
    } else {
        printf("--- No retiming found --- \n");
    }

    printf("original = %d\nretimed = %d\n", c, result.c);

    free(graph.vertices);
    free(graph.edges);
    free(WD);

    return 0;
}

void test_n_random(int n, int vertex_count) {
    printf("- Testing %d graphs with %d vertex -\n", n, vertex_count);
    for(int i = 0; i < n; ++i) {
        printf("- CIRCUIT %d -\n", i);
        Graph graph = generate_circuit(vertex_count);
        int edge_count = graph.edge_count;
        printf("Edges: %d\n", edge_count);

        int *deltas = (int *) malloc(sizeof(int) * vertex_count);
        int c = cp(graph, deltas);
        printf("initial C = %d\n", c);
        free(deltas);

        WDEntry* WD = wd_algorithm(graph);

        OptResult result1 = opt1(graph, WD);
        OptResult result2 = opt2(graph, WD);

        if(result1.r && result1.c < c) {
            printf("OPT1: Retiming found - C: %d\tLegal: %d\n", result1.c, check_legal(graph, result1.graph, result1.c, WD));
        } else {
            printf("OPT1: No retiming found\n");
        }
        if(result2.r && result2.c < c) {
            printf("OPT2: Retiming found - C: %d\tLegal: %d\n", result2.c, check_legal(graph, result2.graph, result2.c, WD));
        } else {
            printf("OPT2: No retiming found\n");
        }

        if(result1.r) {
            free(result1.graph.vertices);
            free(result1.graph.edges);
        }
        if(result2.r) {
            free(result2.graph.vertices);
            free(result2.graph.edges);
        }

        free(graph.vertices);
        free(graph.edges);
        free(WD);

    }
}

int main() {
    //printf("------------ TEST FEAS ------------\n");
    //test_feas();

    /*
    printf("\n\n------------ TEST OPT2 ------------\n");


    printf("\n\n------------ TEST OPT1 ------------\n");
    test_opt1();

    test_opt1_2();

    printf("\n\n------------ TEST RANDOM ------------\n");
    test_random();
    test_opt2();
    */
    test_n_random(10, 500);
}

