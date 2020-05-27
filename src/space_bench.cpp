#include <iostream>
#include <vector>
#include <assert.h>

//#define DEBUGSPACEBENCH

const int types_count = 5;

enum AllocType {
    //OTHER,
    INT,
    VERTEX,
    EDGE,
    BGLVERTEX,
    BGLEDGE
};

const char *type_strings[] = {
    //"other",
    "int",
    "vertex",
    "edge",
    "BGLVertex",
    "BGLEdge"
};

struct Allocation {
    size_t size;
    AllocType type;
    std::string extra_info;
};

//Keeps track of allocations and deallocations for the current space benchmark.
struct SpaceBench{
    std::vector<Allocation> stack_allocations;
    std::vector<Allocation> heap_allocations;
    int stack_bases[10];
    int current_stack = -1;

    size_t heap_size = 0;
    size_t max_heap = 0;
    size_t stack_size = 0;
    size_t max_stack = 0;

    size_t per_type[types_count] = {0}; //allocated memory per type
    size_t max_per_type[types_count] = {0};

    SpaceBench() {}

    void push_stack() {
        ++current_stack;
        stack_bases[current_stack] = stack_allocations.size();
#ifdef DEBUGSPACEBENCH
        printf("Stack %d pushed\n", current_stack);
#endif
    }

    void pop_stack() {
        for(int i = stack_allocations.size()-1; i >= stack_bases[current_stack]; --i) {
            Allocation allocation = stack_allocations[i];
            stack_size -= allocation.size;
            per_type[allocation.type] -= allocation.size;
            stack_allocations.pop_back();
#ifdef DEBUGSPACEBENCH
            printf("Deallocated: Stack -> %zu bytes (%s) (%s)\n", allocation.size, type_strings[allocation.type], allocation.extra_info.c_str());
#endif
        }
#ifdef DEBUGSPACEBENCH
        printf("Stack %d popped\n", current_stack);
#endif
        --current_stack;
    }

    void allocated(size_t size, bool heap, AllocType type, std::string extra_info = "") {
        if (heap) {
            heap_size += size;
            if(heap_size > max_heap) max_heap = heap_size;
            heap_allocations.push_back({size, type, extra_info});
#ifdef DEBUGSPACEBENCH
            printf("Allocated:   Heap -> %zu bytes (%s) (%s)\n", size, type_strings[type], extra_info.c_str());
#endif
        } else {
            stack_size += size;
            if(stack_size > max_stack) max_stack = stack_size;
            stack_allocations.push_back({size, type, extra_info});
#ifdef DEBUGSPACEBENCH
            printf("Allocated:   Stack -> %zu bytes (%s) (%s)\n", size, type_strings[type], extra_info.c_str());
#endif
        }
        per_type[type] += size;
        if(per_type[type] > max_per_type[type]) max_per_type[type] = per_type[type];
    }

    void deallocated(size_t size, AllocType type, std::string extra_info = "") {
        assert(heap_size > heap_size - size);
        heap_size -= size;
        per_type[type] -= size;
#ifdef DEBUGSPACEBENCH
        printf("Deallocated: Heap -> %zu bytes (%s) (%s)\n", size, type_strings[type], extra_info.c_str());
#endif
    }

    void print_state() {
        printf("--- Space Benchmark ---\n");
        printf("Current heap usage: %zu bytes\n", heap_size);
        printf("Current stack usage: %zu bytes\n", stack_size);
        printf("Max heap usage: %zu bytes\n", max_heap);
        printf("Max stack usage: %zu bytes\n", max_stack);
        printf("Max total usage: %zu bytes\n", max_heap + max_stack);
        printf("- Per type -\n");
        for(int i = 0; i < types_count; ++i) {
            printf("Max %s allocated: %zu bytes\n", type_strings[i], max_per_type[i]);
        }
    }

    double ON(size_t N) {
        double c = (double)(max_heap + max_stack) / N;
        printf("%.2f N\n", c);
        return c;
    }
};

//global SpaceBench for the current benchmark, probably not the best idea, but no need to complicate things
SpaceBench *space_bench; 

