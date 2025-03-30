# Mazes

Maze Visualizer in C++ with support for visualization and sonification (audibilization) of various pathfinding and maze generation algorithms, customization and extensibility.

## Build

Dependencies: C++23, [SFML](https://github.com/SFML/SFML), Qt, Boost, [stk](https://github.com/thestk/stk).

Install dependencies using your system package manager or using conan, and then run:
```
cmake -S . -B build
cmake --build build --config Release
cd build/Desktop-Release/
```

## Usage

Supported grids: orthogonal, hexagonal, Voronoi diagram.

Supported maze generation algorithms: Recursive Backtracker algorithm, Eller's algorithm, Kruskals algorithm, Prim's algorithm, recursive division algorithm, Aldous-Broder algorithm, Wilson's algorithm, Hunt-and-Kill algorithm, Growing tree algorithm.

Supported pathfinding algorithms: BFS, A*.

Interface languages supported: English, Russian.

## Demo

[![Demo](http://i.ytimg.com/vi/Yr6sOimsF-E/hqdefault.jpg)](https://www.youtube.com/watch?v=Yr6sOimsF-E)

[![Bonus](http://i.ytimg.com/vi/bniqlOn-6-4/hqdefault.jpg)](https://www.youtube.com/watch?v=bniqlOn-6-4)

## Releases

You can download the Windows release on this page: https://github.com/Kirkezz/Mazes/releases

## Support the Development

If you found this app interesting and would like to support the development of it or my other projects, you can make a donation: https://github.com/Kirkezz/Kirkezz/blob/main/DONATEME.md

To contact me, create an issue or find my contacts on my GitHub profile.
