# Mazes

Maze Visualizer in C++ with support for visualization of pathfinding algorithms and maze generation, customization, and screenshot saving.

## Build

Dependencies: C++20, SFML, Qt, Boost.

```
cmake -S . -B build
cmake --build build --config Release
cd build/bin/
```

## Usage

Supported grids: orthogonal, hexagonal, Voronoi diagram.

Supported maze generation algorithms: Recursive Backtracker algorithm, Eller's algorithm, Kruskals algorithm, Prim's algorithm, recursive division algorithm, Aldous-Broder algorithm, Wilson's algorithm, Hunt-and-Kill algorithm.

Supported pathfinding algorithms: BFS, A*.

Interface languages supported: Russian, English (translation is not finished yet, but will be available soon).
