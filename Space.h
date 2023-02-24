#pragma once
#include "Point2D.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <queue>
#include <random>
#include <ranges>
#include <set>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/polygon/segment_data.hpp>
#include <boost/polygon/voronoi.hpp>
class Space {
public:
    const static size_t NaN = -1, MAX_VALUES = 3;
    enum TilingType { AMORPHOUS = 0, TRIANGLE = 3, SQUARE = 4, HEXAGON = 6 };
    void resize(size_t newWidth, size_t newHeight, TilingType newTiling);
    typedef boost::polygon::point_data<int> VPoint;
    typedef boost::polygon::segment_data<int> VSegment;
    boost::polygon::voronoi_diagram<double> vd;
    std::vector<VPoint> VoronoiPoints;
    Point2Du VoronoiWindowSize;
    float minDistForEdgeAdjacency = 5.f;
    void resize(Point2Df windowSize, int smoothness = 3); // for AMORPHOUS tiling
    Space(size_t width, size_t height, TilingType tiling = SQUARE);
    size_t size() const;
    TilingType tiling() const;
    size_t width() const;
    size_t height() const;
    bool link(size_t i, size_t with, bool endOfStep = true);
    bool unlink(size_t i, size_t with, bool endOfStep = true);
    void disintegrate(size_t i);
private: // forward declaration
    class Node;
public:
    Node& operator[](size_t i);
    Node& operator()(size_t x, size_t y);
    Node& operator()(Point2Du p);
    Point2Du get2DCoordinates(size_t i) const;
    size_t get1DCoordinates(Point2Du p) const;
    bool stepByStepFill = false, stepByStepPath = false;
    enum StepType { SETVALUE1 = 0, SETVALUE2, SETVALUE3, SETCOLOR, LINK, UNLINK, SETNEXTPATH };
    struct Step {
        Point2Du stepValue;
        StepType stepType;
        bool endOfStep;
    };
    std::list<Step> fillStepList;
    std::optional<Step> getNextFillStep();
    std::list<Step> pathStepList;
    std::optional<Step> getNextPathStep();
    std::list<size_t> getNextPath();
    void defaultAllValues();
    void prePathAlgInit();
    std::random_device rd;
    std::default_random_engine dre;
    // filling algorithms
    void clear();
    void floodFill();
    void horizontally();
    void recursiveBacktrackerMaze();
    double EllersMazeVerticalProbability = 0.5;
    void EllersMaze();
    void KruskalsMaze();
    void PrimsMaze();
    void recursiveDivisionMaze();
    void AldousBroderMaze();
    void WilsonsMaze();
    void huntAndKillMaze();
    static constexpr std::array fillArr = {&Space::clear,          &Space::recursiveBacktrackerMaze, &Space::EllersMaze,       &Space::KruskalsMaze,
                                           &Space::PrimsMaze,      &Space::recursiveDivisionMaze,    &Space::AldousBroderMaze, &Space::WilsonsMaze,
                                           &Space::huntAndKillMaze};
    // traversing/pathfinding algorithms
    std::list<size_t> BFSFind(size_t from, size_t to);
    std::function<float(size_t, size_t)> calcWeightFunc; // user-defined function
    std::list<size_t> AStarFind(size_t from, size_t to);
    void DFS(size_t from);
    static constexpr std::array pathArr = {&Space::BFSFind, &Space::AStarFind};
private:
    TilingType _tiling;
    size_t _width, _height;
    void addFillStep(Point2Du stepValue, StepType stepType, bool endOfStep = true);
    void addPathStep(Point2Du stepValue, StepType stepType, bool endOfStep = true);
    struct Node {
        Node(size_t i);
        virtual ~Node() {}
        virtual std::vector<size_t> getAvailableDirs() const;
        bool empty() const;
        bool linked(size_t with) const;
        virtual size_t offset(size_t dir) const = 0; // returns NaN if there is a wall in dir
        bool link(size_t with);
        std::set<size_t>::size_type unlink(size_t with);
        std::set<size_t> next;
        static const size_t defaultValue;
        std::vector<size_t> values;
        size_t i;
        static Space* space;
    };
    struct SquareNode : public Node {
        using Node::Node;
        enum Directions { RIGHT, DOWN, LEFT, UP };
        size_t offset(size_t dir) const override;
        size_t offsetR() const;
        size_t offsetD() const;
        size_t offsetL() const;
        size_t offsetU() const;
        bool wallR() const;
        bool wallD() const;
        bool wallL() const;
        bool wallU() const;
    };
    struct HexagonNode : public Node {
        using Node::Node;
        enum Directions { RIGHT, DOWNRIGHT, DOWNLEFT, LEFT, UPLEFT, UPRIGHT };
        size_t offset(size_t dir) const override;
        size_t offsetR() const;
        size_t offsetDR() const;
        size_t offsetDL() const;
        size_t offsetL() const;
        size_t offsetUL() const;
        size_t offsetUR() const;
        bool wallR() const;
        bool wallDR() const;
        bool wallDL() const;
        bool wallL() const;
        bool wallUL() const;
        bool wallUR() const;
    };
    struct AmorphousNode : public Node {
        std::map<size_t, size_t> neighbours;
        AmorphousNode(size_t i, std::map<size_t, size_t> neighbours);
        std::vector<size_t> getAvailableDirs() const override;
        size_t offset(size_t dir) const override;
    };
    std::vector<std::unique_ptr<Node>> grid;
    // helper functions
    bool moveFrom(size_t& from, size_t to);
    struct selectRandomDirRT {
        size_t next, dir;
    };
    selectRandomDirRT selectRandomDir(
        size_t i, std::function<bool(size_t)> condition = [](size_t i) { return true; });
    std::optional<Point2Df> lineSegmentsIntersection(Point2Df AV0, Point2Df AV1, Point2Df BV0, Point2Df BV1);
    Point2Df lineWindowIntersection(Point2Df AV0, Point2Df AV1, Point2Df windowSize);
    // void visualizePath(const std::list<size_t>& path);
    void setValue(size_t i, size_t vi, size_t v, bool endOfStep = true);
    std::list<std::list<size_t>> paths;
    void addPath(std::list<size_t> path);
    std::list<size_t> constructPath(const std::vector<size_t>& parent, size_t from, size_t to);
};
