#include "SpaceRenderer.h"
#define _USE_MATH_DEFINES
#include <math.h>
using namespace std;
using namespace sf;
// public
void SpaceRenderer::clear() {
    points.clear();
    selectedPoint = Space::NaN;
    path.clear();
}
void SpaceRenderer::loadSpaceProps(Vector2f newShapeSize) {
    clear();
    bool shapeSizeChanged = shapeSize != newShapeSize, tilingChanged = _tiling != space.tiling();
    if(shapeSizeChanged) {
        shapeSize = newShapeSize;
        valueAsText.setCharacterSize((space.tiling() == Space::AMORPHOUS) ? (8 /*temp value*/) : min(shapeSize.x, shapeSize.y) / 6);
        samplePoint.s.setRadius((space.tiling() == Space::AMORPHOUS || space.size() == 1) ? (4 /*temp value*/) : min(shapeSize.x, shapeSize.y) / 8.0f);
        samplePoint.s.setOrigin(samplePoint.s.getRadius(), samplePoint.s.getRadius());
    }
    _tiling = space.tiling();
    Vector2u newWindowSize;
    switch(_tiling) {
    case Space::AMORPHOUS:
        newWindowSize = Vector2u(space.VoronoiWindowSize.x, space.VoronoiWindowSize.y);
        break;
    case Space::TRIANGLE:
        break;
    case Space::SQUARE:
        newWindowSize = Vector2u(space.width() * shapeSize.x + 2, space.height() * shapeSize.y + 2);
        break;
    case Space::HEXAGON:
        newWindowSize = Vector2u(space.width() * shapeSize.x + (shapeSize.x / 2) + 2, space.height() * (shapeSize.y * 0.75) + shapeSize.y * 0.25 + 2);
        break;
    }
    if(window.getSize() != newWindowSize)
        window.create(VideoMode(newWindowSize.x, newWindowSize.y), L"▦");
    if(shapeSizeChanged || tilingChanged || shapes.size() != space.size() || _tiling == Space::AMORPHOUS) {
        shapes.clear();
        ConvexShape shape;
        if(_tiling != Space::AMORPHOUS) {
            shapes.resize(space.size());
            for(size_t i = 0; i < space.size(); ++i) {
                shape.setPointCount(space.tiling());
                for(int i = 0; i < space.tiling(); ++i) {
                    double angle = degreesToRadians(360 / space.tiling() * i - 180 / space.tiling());
                    shape.setPoint(i, Vector2f(1 + cos(angle) * shapeSize.x / 2, 1 + sin(angle) * shapeSize.y / 2));
                }
                shape.setScale(shapeSize.x / shape.getLocalBounds().width, shapeSize.y / shape.getLocalBounds().height);
                shape.setPosition(get2DWindowCoordinates(i));
                shapes[i] = shape;
            }
        } else {
            for(auto& cell : space.vd.cells()) {
                if(!cell.contains_point())
                    continue;
                vector<Space::VPoint> vertexPoints;
                auto edge = cell.incident_edge();
                do {
                    if(edge->is_primary())
                        vertexPoints.push_back(Space::VPoint(edge->vertex0()->x(), edge->vertex0()->y()));
                    edge = edge->next();
                } while(edge != cell.incident_edge());
                if(vertexPoints.size() > 2) {
                    shape.setPointCount(vertexPoints.size());
                    for(int i = 0; i < shape.getPointCount(); ++i)
                        shape.setPoint(i, sf::Vector2f(vertexPoints[i].x() - space.VoronoiPoints[cell.source_index()].x(),
                                                       vertexPoints[i].y() - space.VoronoiPoints[cell.source_index()].y()));
                    shape.setPosition(space.VoronoiPoints[cell.source_index()].x(), space.VoronoiPoints[cell.source_index()].y());
                    shape.setOutlineThickness(outlineThickness);
                    shapes.push_back(shape);
                }
            }
        }
    }
    colors.clear();
    colors.resize(space.size());
    parent.clear();
    parent.resize(space.size(), -1);
    if(_tiling == Space::SQUARE)
        space.calcWeightFunc = [&](size_t a, size_t b) { // Manhattan distance
            return (std::abs(int(a / space.width()) - int(b / space.width())) * shapeSize.x +
                    std::abs(int(a % space.width()) - int(b % space.width())) * shapeSize.y);
        };
    else // Euclidean distance
        space.calcWeightFunc = [&](size_t a, size_t b) { return Point2Df(shapes[a].getPosition()).distance(Point2Df(shapes[b].getPosition())); };
    // TODO: http://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html "Manhattan distance adapted to hexagonal grids."
}
SpaceRenderer::SpaceRenderer(Space& space, RenderWindow& window, Vector2f shapeSize) : space(space), window(window) {
    font.loadFromFile("resources/arial_bolditalicmt.ttf");
    valueAsText.setFont(font);
    t_arrowR.loadFromFile("resources/arrowR.png");
    s_arrowR.setTexture(t_arrowR);
    s_arrowR.setOrigin(256, 256);
    loadSpaceProps(shapeSize);
}
void SpaceRenderer::update() {
    if(space.size() == 1)
        return;
    for(auto& i : points) {
        Point2Df p = i.PA.getNextCoordinates();
        i.s.setPosition(p ? p : shapes[i.end].getPosition());
    }
    if(!manualFillStep)
        fillStep();
    if(!manualPathStep) {
        pathStep();
    }
}
bool isGridDrawn = true, isCurveDrawn = true, isPathDrawn = true, isPointsDrawn = true, isDebugInfoDrawn = true, isMazeDrawn = false;
void SpaceRenderer::draw() {
    if(isGridDrawn)
        drawGrid();
    if(isCurveDrawn)
        drawCurve();
    if(isPathDrawn)
        drawPath();
    if(isPointsDrawn)
        drawPoints();
    if(isDebugInfoDrawn)
        drawDebugInfo();
    if(isMazeDrawn)
        drawMaze();
}
void SpaceRenderer::LMBPressed(Vector2f pos) { lastLMBPressed = mouseTo1DSpaceCoordinates(pos); }
void SpaceRenderer::LMBReleased(Vector2f pos) {
    size_t p = mouseTo1DSpaceCoordinates(pos);
    if(lastLMBPressed == Space::NaN || p == Space::NaN)
        return;
    if(lastLMBPressed == p) {
        addPoint(space.size() == 1 ? Vector2f(pos) : shapes[p].getPosition());
    } else if(!space.stepByStepFill) {
        space.link(lastLMBPressed, p);
    }
}
void SpaceRenderer::RMBPressed(Vector2f pos) { lastRMBPressed = mouseTo1DSpaceCoordinates(pos); }
void SpaceRenderer::RMBReleased(Vector2f pos) {
    size_t p = mouseTo1DSpaceCoordinates(pos);
    if(lastRMBPressed == Space::NaN || p == Space::NaN)
        return;
    if(space.size() == 1 && !points.empty()) {
        deleteSelectedPoint();
        addPoint(pos);
    } else if(lastRMBPressed == p && selectedPoint != Space::NaN) {
        for(size_t i = 0; i < space.size(); ++i) {
            colors[i] = DEFAULTCOLOR;
        }
        parent.clear();
        parent.resize(space.size(), -1);
        space.prePathAlgInit();
        path = (space.*(space.pathArr[selectedPathAlg]))(points[selectedPoint].end, p);
        space.defaultAllValues();
        points[selectedPoint].end = p;
        for(auto i : path) {
            points[selectedPoint].PA.addPoint(Point2Df(shapes[i].getPosition()));
        }
        points[selectedPoint].PA.constructPath(pointSpeed);
    } else if(!space.stepByStepFill) {
        space.unlink(lastRMBPressed, p);
    }
}
void SpaceRenderer::MMBReleased(Vector2f pos) {
    if(size_t p = mouseTo1DSpaceCoordinates(pos); p != Space::NaN && !space.stepByStepFill) {
        space.disintegrate(p);
        colors[p] = DEFAULTCOLOR;
        parent[p] = -1;
    }
}
void SpaceRenderer::deleteSelectedPoint() {
    if(selectedPoint < points.size())
        points.erase(points.begin() + selectedPoint);
    selectNextPoint();
}
void SpaceRenderer::selectNextPoint() {
    if(points.empty())
        return;
    if(selectedPoint < points.size())
        points[selectedPoint].s.setOutlineThickness(0);
    points[++selectedPoint %= points.size()].s.setOutlineThickness(outlineThickness);
}
void SpaceRenderer::fillStep(bool recursion) {
    static Clock spaceFillStepListTimer;
    if(space.stepByStepFill && (manualFillStep || spaceFillStepListTimer.getElapsedTime().asSeconds() > spaceFillStepListDelay || recursion)) {
        spaceFillStepListTimer.restart();
        space.stepByStepFill = false;
        auto s = space.getNextFillStep();
        if(s) {
            switch(s->stepType) {
            case Space::LINK:
                space.link(s->stepValue.x, s->stepValue.y);
                break;
            case Space::UNLINK:
                space.unlink(s->stepValue.x, s->stepValue.y);
                break;
            case Space::SETCOLOR:
                colors[s->stepValue.x] = s->stepValue.y;
                break;
            default:
                space[s->stepValue.x].values[s->stepType] = s->stepValue.y;
                break;
            }
            space.stepByStepFill = true;
            if(!s->endOfStep)
                fillStep(true);
        }
    }
}
void SpaceRenderer::pathStep(bool recursion) {
    static Clock spacePathStepListTimer;
    static int count = 0;
    if(space.stepByStepPath && (manualPathStep || spacePathStepListTimer.getElapsedTime().asSeconds() > spacePathStepListDelay || recursion)) {
        spacePathStepListTimer.restart();
        auto s = space.getNextPathStep();
        if(s) {
            if(s->stepType == Space::SETCOLOR)
                colors[s->stepValue.x] = s->stepValue.y;
            else if(s->stepType == Space::SETNEXTPATH)
                path = space.getNextPath();
            else if(s->stepType == Space::SETPARENT)
                parent[s->stepValue.x] = s->stepValue.y;
            else
                space[s->stepValue.x].values[s->stepType] = s->stepValue.y;
            if(!s->endOfStep)
                pathStep(true);
        }
    }
}
size_t SpaceRenderer::getPointsSize() { return points.size(); }
vector<Point2Df> SpaceRenderer::getPoints() {
    vector<Point2Df> result;
    result.reserve(points.size());
    for(auto& i : points) {
        result.push_back(Point2Df(i.s.getPosition()));
    }
    return result;
}
// private
void SpaceRenderer::drawGrid() {
    for(size_t i = 0; i < shapes.size(); ++i) {
        shapes[i].setOutlineColor(colorScheme[OUTLINECOLOR]);
        shapes[i].setOutlineThickness(outlineThickness);
        shapes[i].setFillColor(colorScheme[colors[i]]);
        window.draw(shapes[i]);
    }
}
void SpaceRenderer::drawThickSpaceLine(size_t a, size_t b, float thickness, Color color) {
    if(a == b)
        return;
    size_t edge = Space::NaN;
    for(auto i : space[a].getAvailableDirs()) {
        if(space[a].offset(i) == b) {
            edge = i;
            break;
        }
    }
    Vector2f aPos = shapes[a].getPosition(), bPos = shapes[b].getPosition();
    if(edge == Space::NaN || _tiling != Space::AMORPHOUS) {
        drawThickLine({aPos.x, aPos.y}, {bPos.x, bPos.y}, thickness, color);
    } else {
        Point2Df p1 = Point2Df(shapes[a].getPoint(edge)) * Point2Df(shapes[a].getScale()) + Point2Df(shapes[a].getPosition());
        Point2Df p2 = Point2Df(shapes[a].getPoint((edge + 1) % shapes[a].getPointCount())) * Point2Df(shapes[a].getScale()) + Point2Df(shapes[a].getPosition());
        Vector2u windowSize = window.getSize();
        if(!(p1.x > 0 && p1.y > 0 && p1.x < windowSize.x && p1.y < windowSize.y)) {
            swap(p1, p2);
        }
        Point2Df inter = lineWindowIntersection(p1, p2);
        Point2Df centerAdjEdgePos((p1.x + inter.x) / 2, (p1.y + inter.y) / 2);
        drawThickLine({aPos.x, aPos.y}, {centerAdjEdgePos.x, centerAdjEdgePos.y}, thickness, color);
        drawThickLine({centerAdjEdgePos.x, centerAdjEdgePos.y}, {bPos.x, bPos.y}, thickness, color);
    }
}
void SpaceRenderer::drawWindowBorders() { // palliative
    static const std::array<Point2Du, 5> borders = {Point2Du(0, 0), Point2Du(1, 0), Point2Du(1, 1), Point2Du(0, 1)};
    auto w = window.getSize();
    for(size_t i = 0; i < 4; ++i) {
        drawThickLine(Point2Df(w.x * borders[i].x, w.y * borders[i].y), Point2Df(w.x * borders[(i + 1) % 4].x, w.y * borders[(i + 1) % 4].y), outlineThickness * 2,
                      colorScheme[MAZECOLOR]);
    }
}
void SpaceRenderer::drawCurve() {
    for(size_t i = 0; i < space.size(); ++i)
        for(size_t j : space[i].next)
            if(i < j)
                drawThickSpaceLine(i, j, curveThickness, colorScheme[CURVECOLOR]);
}
void SpaceRenderer::drawPath() {
    size_t current = *path.begin();
    for(auto it = path.begin(); it != path.end(); current = *it, ++it) {
        drawThickSpaceLine(current, *it, pathThickness, colorScheme[PATHCOLOR]);
    }
}
void SpaceRenderer::addPoint(Vector2f p) {
    samplePoint.s.setFillColor(colorScheme[POINTSCOLOR]);
    samplePoint.s.setOutlineColor(colorScheme[OUTLINECOLOR]);
    points.push_back(samplePoint);
    points.back().s.setPosition(p);
    points.back().end = mouseTo1DSpaceCoordinates(p);
    if(selectedPoint != Space::NaN)
        points[selectedPoint].s.setOutlineThickness(0);
    points.back().s.setOutlineThickness(outlineThickness);
    selectedPoint = points.size() - 1;
}
void SpaceRenderer::drawPoints() {
    for(auto& i : points)
        window.draw(i.s);
}
void SpaceRenderer::drawDebugInfo() {
    valueAsText.setFillColor(colorScheme[TEXTCOLOR]);
    for(size_t i = 0; i < space.size(); ++i) {
        for(size_t j = 0; j < space[i].values.size(); ++j) {
            valueAsText.setOrigin((valueAsText.getLocalBounds().left + valueAsText.getLocalBounds().width) / 2,
                                  (valueAsText.getLocalBounds().height + valueAsText.getLocalBounds().height) / 2);
            if(space[i].values[j] != space[i].defaultValue) {
                valueAsText.setString(to_string(space[i].values[j]));
                valueAsText.setOrigin((valueAsText.getLocalBounds().left + valueAsText.getLocalBounds().width) / 2,
                                      (valueAsText.getLocalBounds().height + valueAsText.getLocalBounds().height) / 2);
                valueAsText.setPosition(getValueCoordinates(i, j));
                //  valueAsText.setPosition(getValueCoordinates(i, j == 0 ? 0 : j == 1 ? (shapes[i].getPointCount()) / 2 : shapes[i].getPointCount() - 1));
                window.draw(valueAsText);
            }
        }
    }
    s_arrowR.setPosition(get2DWindowCoordinates(0));
    s_arrowR.setScale(shapes[0].getLocalBounds().width / 768., shapes[0].getLocalBounds().height / 768.);
    s_arrowR.setScale(0.05, 0.05);
    for(size_t i = 0; i < space.size(); ++i) {
        if(parent[i] != Space::NaN) {
            s_arrowR.setPosition(shapes[i].getPosition());
            s_arrowR.setRotation(getDegrees(shapes[i].getPosition(), shapes[parent[i]].getPosition()));
            window.draw(s_arrowR);
        }
    }
}
void SpaceRenderer::drawMaze() {
    for(size_t i = 0; i < space.size(); ++i)
        for(size_t j = 0; j < shapes[i].getPointCount(); ++j)
            if(size_t offset = space[i].offset(j); (offset == Space::NaN || !space[i].linked(offset)) && i < offset) {
                Point2Df p1 = Point2Df(shapes[i].getPoint(j)) * Point2Df(shapes[i].getScale()) + Point2Df(shapes[i].getPosition()),
                         p2 = Point2Df(shapes[i].getPoint((j + 1) % shapes[i].getPointCount())) * Point2Df(shapes[i].getScale()) + Point2Df(shapes[i].getPosition());
                drawThickLine(p1, p2, mazeThickness, colorScheme[MAZECOLOR]);
            }
    if(space.tiling() == Space::AMORPHOUS)
        drawWindowBorders();
}
// helper functions
double SpaceRenderer::radiansToDegrees(double r) const { return r * (180 / M_PI); }
double SpaceRenderer::degreesToRadians(double d) const { return d * M_PI / 180; }
double SpaceRenderer::getDegrees(sf::Vector2f a, sf::Vector2f b) const { return radiansToDegrees(atan2(b.y - a.y, b.x - a.x)); }
Vector2f SpaceRenderer::get2DWindowCoordinates(Point2Du p) const { return get2DWindowCoordinates(space.get1DCoordinates(p)); }
Vector2f SpaceRenderer::get2DWindowCoordinates(size_t i) const {
    if(space.tiling() == Space::AMORPHOUS)
        return shapes[i].getPosition();
    Point2Du p = space.get2DCoordinates(i);
    if(space.tiling() == Space::TRIANGLE)
        return {};
    if(space.tiling() == Space::SQUARE)
        return Vector2f(p.x * shapeSize.x + shapeSize.x / 2.0f, p.y * shapeSize.y + shapeSize.y / 2.0f);
    if(space.tiling() == Space::HEXAGON)
        return Vector2f((p.x + 0.5 * (p.y & 1)) * shapeSize.x + shapeSize.x / 2, p.y * shapeSize.y * 0.75 + shapeSize.y / 2);
}
Point2Du SpaceRenderer::mouseTo2DSpaceCoordinates(Vector2f p) const { return space.get2DCoordinates(mouseTo1DSpaceCoordinates(p)); }
size_t SpaceRenderer::mouseTo1DSpaceCoordinates(Vector2f p) const {
    if(!isPointInsideWindow(Point2Df(p.x, p.y)))
        return Space::NaN;
    if(_tiling == Space::SQUARE) {
        size_t t = space.get1DCoordinates(Point2Du(floor(p.x / shapeSize.x), floor(p.y / shapeSize.y)));
        return t >= space.size() ? Space::NaN : t;
    }
    for(size_t i = 0; i < shapes.size(); ++i) {
        if(isPointInsideShape(Point2Df(p), shapes[i])) {
            return i;
        }
    }
    return Space::NaN;
}
void SpaceRenderer::drawThickLine(Point2Df a, Point2Df b, float thickness, Color color) {
    Point2Df v = b - a, p(v.y, -v.x);
    float length = sqrt(p.x * p.x + p.y * p.y);
    Point2Df n = v / Point2Df(length), np = p / Point2Df(length); // normalized perpendicular
    ConvexShape convex(4);
    convex.setPoint(0, a + np * Point2Df(thickness / 2) - n * Point2Df(thickness / 2));
    convex.setPoint(1, a - np * Point2Df(thickness / 2) - n * Point2Df(thickness / 2));
    convex.setPoint(2, b - np * Point2Df(thickness / 2) + n * Point2Df(thickness / 2));
    convex.setPoint(3, b + np * Point2Df(thickness / 2) + n * Point2Df(thickness / 2));
    convex.setFillColor(color);
    window.draw(convex);
}
Vector2f SpaceRenderer::getValueCoordinates(size_t i, size_t vi) const {
    return Vector2f(shapes[i].getPoint(vi).x * 0.66 * shapes[i].getScale().x, shapes[i].getPoint(vi).y * 0.66 * shapes[i].getScale().y) + shapes[i].getPosition();
}
// "each such cell is obtained from the intersection of half-spaces, and hence it is a (convex) polyhedron", so it works for Voronoi diagram (Space::AMORPHOUS)
bool SpaceRenderer::isPointInsideShape(Point2Df p, const ConvexShape& shape) const {
    bool c = false;
    for(int i = 0; i < shape.getPointCount(); ++i) {
        Point2Df p1 = Point2Df(shape.getPoint(i)) * Point2Df(shape.getScale()) + Point2Df(shape.getPosition()),
                 p2 = Point2Df(shape.getPoint((i + 1) % shape.getPointCount())) * Point2Df(shape.getScale()) + Point2Df(shape.getPosition());
        if((((p1.y <= p.y) && (p.y < p2.y)) || ((p2.y <= p.y) && (p.y < p1.y))) &&
           (((p2.y - p1.y) != 0) && (p.x > ((p2.x - p1.x) * (p.y - p1.y) / (p2.y - p1.y) + p1.x))))
            c = !c;
    }
    return c;
}
const double EPS = 1E-9;

int det(int a, int b, int c, int d) { return a * d - b * c; }
bool between(int a, int b, double c) { return min(a, b) <= c + EPS && c <= max(a, b) + EPS; }
bool intersect_1(int a, int b, int c, int d) {
    if(a > b)
        swap(a, b);
    if(c > d)
        swap(c, d);
    return max(a, c) <= min(b, d);
}
optional<Point2Df> SpaceRenderer::lineSegmentsIntersection(Point2Df AV0, Point2Df AV1, Point2Df BV0, Point2Df BV1) {
    float s1_x = AV1.x - AV0.x, s1_y = AV1.y - AV0.y, s2_x = BV1.x - BV0.x, s2_y = BV1.y - BV0.y,
          s = (-s1_y * (AV0.x - BV0.x) + s1_x * (AV0.y - BV0.y)) / (-s2_x * s1_y + s1_x * s2_y),
          t = (s2_x * (AV0.y - BV0.y) - s2_y * (AV0.x - BV0.x)) / (-s2_x * s1_y + s1_x * s2_y);
    return (s >= 0 && s <= 1 && t >= 0 && t <= 1) ? Point2Df(AV0.x + (t * s1_x), AV0.y + (t * s1_y)) : optional<Point2Df>{};
}
Point2Df SpaceRenderer::lineWindowIntersection(Point2Df AV0, Point2Df AV1) {
    Vector2u windowSize = window.getSize();
    optional<Point2Df> result;
    return (result = lineSegmentsIntersection(AV0, AV1, Point2Df(0, 0), Point2Df(windowSize.x, 0)))                         ? *result
           : (result = lineSegmentsIntersection(AV0, AV1, Point2Df(windowSize.x, 0), Point2Df(windowSize.x, windowSize.y))) ? *result
           : (result = lineSegmentsIntersection(AV0, AV1, Point2Df(windowSize.x, windowSize.y), Point2Df(0, windowSize.y))) ? *result
           : (result = lineSegmentsIntersection(AV0, AV1, Point2Df(0, windowSize.y), Point2Df(0, 0)))                       ? *result
                                                                                                                            : AV1;
}
bool SpaceRenderer::isPointInsideWindow(Point2Df p) const { return p.x > 0 && p.y > 0 && p.x < window.getSize().x && p.y < window.getSize().y; }
