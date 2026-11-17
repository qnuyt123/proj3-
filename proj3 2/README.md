# Project 3 README

Student 1: Qianyu Guo (927297132)

Student 2: Huanchen Ji (927576963)

## Project Status

The project iplements the graph and circuit trace/net comoonents.

The following tests passed when running "make":
 -Graph:14 tests
 -MutableGraph:15 tests
 -CircuitTrace:34 tests
 -CircuitNetaa:55 tests
 -total:118 tests passed

 The generated coverage report shows 94.6% line coverage and 93.5% function coverage.


## Known Issues

All 118 tests run by "make" pass.

# Code References

* ECS36C Discussion 11/06: `AdjacencyListGraph.h`, `GraphFactory.h` (used as given)
* ECS36C Discussion 11/13: `AdjacencyListGraph.cpp` (constructor, vertex/edge functions, BreadthFirstSearchUnweighted, DepthFirstSearch) and `Makefile` (extended with grid graph, trace and net targets)
* ECS36C Discussions 9/25 - 10/23: Makefile structure and the `SImplementation` (PImpl) style
* Lecture Notes 4 (symbol tables) and Lecture Notes 5 (adjacency lists, BFS/DFS, Dijkstra, connected components, symbol graph)
* [Dijkstra's shortest path algorithm](https://www.geeksforgeeks.org/dsa/dijkstras-shortest-path-algorithm-greedy-algo-7/)
* [std::priority_queue (min heap with std::greater)](https://en.cppreference.com/w/cpp/container/priority_queue)
* [Makefile tuto](https://makefiletutorial.com/)


## Generative AI Use

We used Claude to help write and check the implementation of this project.

### Prompt 1

check our code to see why it isn’t working properly 

### Response 1

Claude made a few changes

### Prompt 2

How to upload the project to GitHub, and give the suggested commit messages 


### Response 2

Claude explained the git commands and gave the suggested commit messages.

### Prompt 3

Go through Lecture Notes and the discussion code with us, help me study this in depth and explain the concepts in them, and explain how they are applied in this project.

### Response 3

Claude went through the lecture notes and discussion code and explained how each concept is used in the project