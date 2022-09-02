#pragma once
#include "Point2D.h"

#include <vector>
#include <random>
#include <ctime>
enum Directions { // (→ ← ↑ ↓ ↖ ↗ ↙ ↘)
    RIGHT = 0,
    LEFT,
    UP,
    DOWN,
    UPLEFT,
    UPRIGHT,
    DOWNLEFT,
    DOWNRIGHT
};

struct Node {
    size_t prev = -1, value = 0, next = -1;
};

class Space {
public:
	const static size_t NaN = -1;
	Space(size_t width, size_t height);
	size_t width, height, begin = NaN;
	size_t size();
    Node& operator[](size_t i);
    Node operator()(size_t i);
    Node operator()(size_t x, size_t y);
    Node operator()(Point2Du p);
	Point2Du get2DCoordinates(size_t i);
	size_t get1DCoordinates(Point2Du p);
	void link(size_t i, size_t with);
	void mirrorX();
	void mirrorY();
    // filling algorithms
	void horizontally();
    void zigzag();
    void spiral();
private:
    std::vector<Node> field;
	void initBegin(size_t i);
	size_t mirrorX(size_t i); // get the mirrored coords for one point
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
	// functions for filling algorithms
	size_t right(size_t pos);
	size_t left(size_t pos);
	size_t up(size_t pos);
	size_t down(size_t pos);
    size_t upleft(size_t pos);
    size_t upright(size_t pos);
    size_t downleft(size_t pos);
    size_t downright(size_t pos);
    bool movefrom(size_t& pos, size_t dir);
};
