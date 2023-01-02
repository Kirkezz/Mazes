#pragma once
#include "Point2D.h"

#include <functional>
#include <iostream>
#include <vector>
#include <random>
#include <array>
#include <queue>
#include <cmath>
#include <list>
#include <set>
enum Directions { // (→ ← ↑ ↓ ↖ ↘ ↗ ↙)
    RIGHT = 0,
    LEFT,
    UP,
    DOWN,
    UPLEFT,
    DOWNRIGHT,
    UPRIGHT,
    DOWNLEFT
};
struct Node {
    std::set<size_t> next;
    size_t value = 0;
    bool empty() { return next.empty(); }
    bool linked(size_t i) { return next.find(i) != next.end(); }
};
class Space {
public:
	const static size_t NaN = -1;
	Space(size_t width, size_t height);
    size_t width, height;
	size_t size();
    void resize(size_t new_width, size_t new_height);
    bool isValid(size_t i);
    bool isValid(Point2Du p);
    Node& operator[](size_t i);
    Node operator()(size_t i);
    Node operator()(size_t x, size_t y);
    Node operator()(Point2Du p);
    std::vector<Node>& getField();
	Point2Du get2DCoordinates(size_t i);
	size_t get1DCoordinates(Point2Du p);
    bool link(size_t i, size_t with, bool endOfStep = true);
    bool unlink(size_t i, size_t with, bool endOfStep = true);
    void disintegrate(size_t i);
	void mirrorX();
	void mirrorY();
    // filling algorithms
    void clear();
    void horizontally();
    void spiral();
    void zigzag();
    void recursiveBacktrackerMaze();
    double EllersMazeVerticalProbability = 0.5;
    void EllersMaze();
    void KruskalsMaze();
    void PrimsMaze();
    void recursiveDivisionMaze();
    void AldousBroderMaze();
    void WilsonsMaze();
    void huntAndKillMaze();
    static constexpr std::array fillArr = {&Space::clear, &Space::horizontally, &Space::spiral, &Space::zigzag, &Space::recursiveBacktrackerMaze, &Space::EllersMaze, &Space::KruskalsMaze, &Space::PrimsMaze,
                                           &Space::recursiveDivisionMaze, &Space::AldousBroderMaze, &Space::WilsonsMaze, &Space::huntAndKillMaze};
    bool wall(size_t i, size_t dir);
    size_t offset(size_t i, size_t dir);
    // traversing/pathfinding algorithms
    std::vector<size_t> BFS(size_t from);
    std::list<size_t> BFSfind(size_t from, size_t to);
    void DFS(size_t from);
    // sandbox
    double radiansToDegrees(double r);
    double degreesToRadians(double d);
    void completeGraph(size_t verticles, float offset);
    void graphTraversal(size_t from);
    std::vector<Point2Du> combineIntervals(const std::vector<Point2Du>& s);
    void connectEdges();
    void splitEdges();
    bool stepByStepFilling = false;
    enum StepTypes {
        LINK = 0,
        UNLINK = 1,
        SETVALUE = 2
    };
    struct Step {
        Point2Du stepValue;
        size_t stepType;
        bool endOfStep;
    };
    std::list<Step> stepList;
    void addStep(Point2Du stepValue, size_t stepType, bool endOfStep = true);
private:
    std::vector<Node> field; // undirected graph
    // get the mirrored coords for one point
    size_t mirrorX(size_t i);
	size_t mirrorY(size_t i);
    // wall in the directions (→ ← ↑ ↓ ↖ ↗ ↙ ↘)
    bool wallR(size_t i);
    bool wallL(size_t i);
    bool wallU(size_t i);
    bool wallD(size_t i);
    bool wallUL(size_t i);
    bool wallUR(size_t i);
    bool wallDL(size_t i);
    bool wallDR(size_t i);
    size_t right(size_t i);
    size_t left(size_t i);
    size_t up(size_t i);
    size_t down(size_t i);
    size_t upleft(size_t i);
    size_t upright(size_t i);
    size_t downleft(size_t i);
    size_t downright(size_t i);
    bool sameHorizontal(size_t a, size_t b);
    bool sameVertical(size_t a, size_t b);
    bool sameDiagonal(size_t a, size_t b);
    bool sameAntiDiagonal(size_t a, size_t b);
    bool moveFrom(size_t& pos, size_t dir);
    size_t isBasicDir(size_t i, size_t with);
    size_t reverseDir(size_t dir);
    struct selectRandomDirRT { size_t next, dir; };
    selectRandomDirRT selectRandomDir(size_t i, std::function<bool(size_t)> condition = [](size_t i){return true;}, bool shuffle = true);
    std::random_device rd;
    std::default_random_engine dre;
};
