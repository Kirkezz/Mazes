#pragma once
#include "Point2D.h"
#include <cmath>
#include <list>
#include <map>
#include <string>
class PointAnimation {
public:
    PointAnimation();
    void addPoint(Point2Df p);
    void constructPath(float speed = 1.0f); // pixels per frame
    Point2Df getNextCoordinates();
    static float distance(Point2Df a, Point2Df b);
    static Point2Df abs(Point2Df p);
private:
    Point2Df coords;
    std::list<Point2Df> checkpoints;
    std::list<Point2Df> path;
};
