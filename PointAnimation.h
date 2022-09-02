#pragma once
#include "Point2D.h"
#include <string>
#include <cmath>
#include <list>
#include <map>
class PointAnimation {
public:
    PointAnimation();
    void addPoint(Point2Df p);
    void constructPath(float speed = 1.0f); // pixels per frame
    Point2Df getNextCoordinates();
private:
    Point2Df coords;
    std::list<Point2Df> checkpoints;
    std::list<Point2Df> path;
    float distance(Point2Df a, Point2Df b);
    Point2Df abs(Point2Df p);
};
