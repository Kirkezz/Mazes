#include "PointAnimation.h"
#include <cassert>
using namespace std;
PointAnimation::PointAnimation() {}
void PointAnimation::addPoint(Point2Df p) { checkpoints.push_back(p); }
void PointAnimation::constructPath(float speed) {
    assert(checkpoints.size() > 1);
    Point2Df a = checkpoints.front(), b(0.0f, 0.0f);
    checkpoints.pop_front();
    while (!checkpoints.empty()) {
        b = checkpoints.front();
        checkpoints.pop_front();
        Point2Df dif = b - a;
        float dist = sqrt(pow(dif.x, 2) + pow(dif.y, 2));
        Point2Df offset = Point2Df(dif.x / dist, dif.y / dist) * Point2Df(speed, speed);
        size_t i = 1;
        Point2Df t;
        do {
            t = a + offset * Point2Df(i, i);
            path.push_front(t);
            ++i;
        } while (distance(t, b) >= speed);
        a = move(b);
    }
}
Point2Df PointAnimation::getNextCoordinates() {
    if (path.empty()) {
        return Point2Df(NAN, NAN);
    }
    Point2Df t = path.back();
    path.pop_back();
    return t;
}
float PointAnimation::distance(Point2Df a, Point2Df b) { return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2)); }
Point2Df PointAnimation::abs(Point2Df p) { return Point2Df(std::abs(p.x), std::abs(p.y)); }
