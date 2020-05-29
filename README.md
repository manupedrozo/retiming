# Retiming
Project based on the Retiming Synchronous Circuitry paper by Leiserson and Saxe.

The implemented algorithms are:
- CP
- WD
- OPT1
- FEAS
- OPT2

## Dependencies
**boost BGL**: 
[https://www.boost.org/doc/libs/1_73_0/libs/graph/doc/index.html](https://www.boost.org/doc/libs/1_73_0/libs/graph/doc/index.html)

No need to build, BGL is a header-only library.

**Google benchmark:** (For performance testing only)
[https://github.com/google/benchmark](https://github.com/google/benchmark)

Follow the installation instructions in the provided link.

## Compilation
Using an unity build since the project is very simple.

To compile **main.cpp**:
```bash
g++ -I <path_to_boost> [-g] -o3 src/main.cpp -o build/main
```
Performance benchmark (**performance_bench_main.cpp**):
```bash
g++ -I <path_to_boost> performance_bench_main.cpp -isystem <path_to_gbenchmark>/include -L<path_to_gbenchmark>/build/src -lbenchmark -lpthread -o3 -o build/performance_bench_main
```
Space benchmark (**space_bench_main.cpp**):
```bash
g++ -I <path_to_boost> [-g] -o3 space_bench_main.cpp -o build/space_bench_main
```

## Documentation
Each algorithm is well annotated, so refer to them for more details.

What is in each file:
- ***types.h***: General types declaration, including the Graph struct.
	- **Graph**: composed by an array of *vertices* and an array of *edges*. The variables *vertex_count* and *edge_count* indicate the length of said arrays.
	- **Vertex**: A Vertex has a weight, thats it. The vertex id is its index in the graphs array.
	- **Edge**: An Edge is composed by the *from* and *to* vertex ids and a *weight* value.

- ***cp.cpp***: CP algorithm.
	- **int cp(Graph &graph, int \*deltas)**
		- graph: Graph to calculate cp on.
		- deltas: array of size graph.vertex_count to calculate CP deltas on.
		- Returns the graphs clock period.

- ***wd.cpp***: WD algorithm.
	- **WDEntry \*wd(Graph &graph)**
		- graph: Graph to calculate wd on.
		- Returns the the resulting WD matrix.

- ***feas.cpp***: FEAS algorithm.
	- **FeasResult feas(Graph &graph, int target_c, int \*deltas)**
		- graph: Graph to calculate feas on.
		- target_c: The target clock period.
		- deltas: Array to calculate CP deltas.
		- Returns a FeasResult with the minimized clock period and the retimed graph.

- ***opt.cpp***: bellman, OPT1 and OPT2 algorithms.
	- **bool bellman(Graph &graph, int \*distance)**
		- graph: Graph to calculate bellman on.
		- distance: Array of size graph.vertex_count+1 to store distances as calculated by bellman.
		- Returns true if no negative cycles were found.
	- **OptResult opt1(Graph &graph, WDEntry \*WD)**
		- graph: Graph to calculate opt1 on.
		- WD: WD matrix as returned by wd algorithm.
		- Returns an OptResult with the minimized clock period and the retimed graph.
	- **OptResult opt2(Graph &graph, WDEntry \*WD)**
		- graph: Graph to calculate opt2 on.
		- WD: WD matrix as returned by wd algorithm.
		- Returns an OptResult with the minimized clock period and the retimed graph.
- ***retiming_checker.cpp***: Check if a retiming is legal.
	- **bool check_legal(Graph &graph, Graph &retimed, int c, WDEntry \*WD)**
		- graph: Base graph.
		- retimed: Retimed graph.
		- c: Clock period of the retimed graph.
		- WD: WD matrix of the base graph.
		- Returns true if the retiming is legal.

- ***circuit_generator.cpp***: Generate a random circuit graph.
	- **Graph generate_circuit(int vertex_count)**
		- vertex_count: Amount of vertex for the generated graph.
		- Returns a randomly generated citcuit graph that is connected and has no 0 weight cycles.

- ***cycle_finder.cpp***: Find 0 weight cycles in a graph.
	- **void find_zero_weight_cycles(std::vector<std::vector<Edge\*>> \*cycles, Graph &graph)**
		- cycles: Vector where to store the 0 weight cycles found.

- ***graph_printer.cpp***: Print a graph or generate a dot file.
	- **void print_graph(Graph &graph, std::string name)**
		- graph: Graph to print.
		- name: Graphs name.
	- **void to_dot(Graph &graph, std::string path)**
		- graph: Graph to translate into the dot file.
		- path: Path to the dot file.
	
- ***performance_bench_main.cpp***: Performance benchmark of the algorithms.
	- **void BM_topology(benchmark::State& state)**
	- **void BM_cp(benchmark::State& state)**
	- **void BM_wd(benchmark::State& state)**
	- **void BM_bellman(benchmark::State& state)**
	- **void BM_opt1(benchmark::State& state)**
	- **void BM_feas(benchmark::State& state)**
	- **void BM_opt2(benchmark::State& state)**

- ***space_bench_main.cpp***: Space benchmark of the algorithms.
	- **void SBM_cp()**
	- **void SBM_wd()**
	- **void SBM_opt1()**
	- **void SBM_feas()**
	- **void SBM_opt2()**

- ***space_bench.cpp***: Structs required to keep track of allocations and deallocations for a running space benchmark.
