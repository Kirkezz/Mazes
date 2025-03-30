#pragma once
#include "Point2D.h"
#include <cmath>
#include <iostream>
#include <list>
#include <map>
#include <string>

/// A class used by `SpaceRenderer` to calculate the trajectory of a point along some path
class PointAnimation {
  public:
    PointAnimation() = default;
    /// adds the points visited by the moving point during its movement along the path
    void addPoint(Point2Df p);
    /// based on the points previously added via `addPoint`, constructs the path of the moving point
    void constructPath(float speed = 1.0f); // pixels per frame
    /// returns the subsequent intermediate coordinates or Point2Df(NAN, NAN)
    Point2Df getNextCoordinates();

  private:
    Point2Df coords;
    std::list<Point2Df> checkpoints;
    std::list<Point2Df> path;
};
