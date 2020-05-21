//#define OPT1DEBUG
//#define OPT2DEBUG
#define CIRCUITGENDEBUG
//#define FEASDEBUG
//#define CPDEBUG

#include "types.h"
#include "graph_printer.cpp" 
#include "circuit_generator.cpp" 
#include "wd.cpp" 
#include "opt.cpp" 
#include "cp.cpp"
#include "feas.cpp"

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
    const int edge_count = 5;
    const int root_vertex = 0;

    Vertex vertices[] = {
        Vertex(0),
        Vertex(7),
        Vertex(1),
        Vertex(3),
        Vertex(0),
    };

    Edge edges[] = { 
        Edge(0, 1, 2),
        Edge(1, 2, 1),
        Edge(2, 3, 0),
        Edge(3, 4, 1),
        Edge(4, 0, 0),
    };


    Graph graph(vertices, edges, vertex_count, edge_count);

    to_dot(graph, "graph.dot");

    int *deltas = (int *) malloc(sizeof(int) * vertex_count);
    int c = cp(graph, deltas);
    printf("initial C = %d\n", c);
    free(deltas);

    WDEntry* WD = wd_algorithm(graph);

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
    Graph graph = generate_circuit(5);
    int vertex_count = graph.vertex_count;
    int edge_count = graph.edge_count;

    to_dot(graph, "random_circuit.dot");

    int *deltas = (int *) malloc(sizeof(int) * vertex_count);
    int c = cp(graph, deltas);
    printf("initial C = %d\n", c);
    free(deltas);

    WDEntry* WD = wd_algorithm(graph);

    
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

int main() {
    //printf("------------ TEST FEAS ------------\n");
    //test_feas();

    /*
    printf("\n\n------------ TEST OPT2 ------------\n");
    test_opt2();

    printf("\n\n------------ TEST OPT1 ------------\n");
    test_opt1();


    test_opt1_2();
    */

    printf("\n\n------------ TEST RANDOM ------------\n");
    test_random();

}

