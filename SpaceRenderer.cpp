#include "SpaceRenderer.h"
#include <unordered_set>
#include <cassert>
#include <list>
using namespace std;
using namespace sf;
void SpaceRenderer::preInit() {
    defaultPoint.s.setFillColor(pointsColor);
    defaultPoint.s.setOutlineColor(outlineColor);
    font.loadFromFile("arial_bolditalicmt.ttf");
    valueAsText.setFont(font);
    valueAsText.setFillColor(Color(127, 127, 127));
}
void SpaceRenderer::rectSizeInit() {
    valueAsText.setCharacterSize(std::min(rectSize.x, rectSize.y) / 4);
    defaultPoint.s.setRadius(std::min(rectSize.x, rectSize.y) / 8.0f);
    defaultPoint.s.setOrigin(defaultPoint.s.getRadius(), defaultPoint.s.getRadius());
}
void SpaceRenderer::loadSpaceProps() {
    if(constWindowSize) {
        rectSize = Vector2f(window.getSize().x / space.width, window.getSize().y / space.height);
        rectSizeInit();
    } else {
        window.create(VideoMode(space.width * rectSize.x, space.height * rectSize.y), L"▦");
    }
    clear();
}
void SpaceRenderer::clear() {
    points.clear();
    selectedPoint = Space::NaN;
    path.clear();
}
SpaceRenderer::SpaceRenderer(Space& space, RenderWindow& window) : space(space), window(window) {
    constWindowSize = true;
    preInit();
    loadSpaceProps();
}
SpaceRenderer::SpaceRenderer(Space& space, sf::RenderWindow& window, sf::Vector2f rectSize) : space(space), window(window), rectSize(rectSize) {
    constWindowSize = false;
    preInit();
    rectSizeInit();
    loadSpaceProps();
}
void SpaceRenderer::LMBPressed(Point2Df pos) {
    lastLMBPressed = Point2Du(floor(pos.x / rectSize.x), floor(pos.y / rectSize.y));
}
void SpaceRenderer::LMBReleased(Point2Df pos) {
    Point2Du p = Point2Du(floor(pos.x / rectSize.x), floor(pos.y / rectSize.y));
    if(!space.isValid(p) || !space.isValid(lastLMBPressed))
        return;
    if(lastLMBPressed == p) {
        addPoint(get2DWindowCoordinates(p));
    } else if(!space.stepByStepFilling || space.stepList.empty() && !manualStep) {
        space.link(space.get1DCoordinates(lastLMBPressed), space.get1DCoordinates(p));
    }
}
void SpaceRenderer::RMBPressed(Point2Df pos) {
    lastRMBPressed = Point2Du(floor(pos.x / rectSize.x), floor(pos.y / rectSize.y));
}
void SpaceRenderer::RMBReleased(Point2Df pos) {
    Point2Du p = Point2Du(floor(pos.x / rectSize.x), floor(pos.y / rectSize.y));
    if(!space.isValid(p) || !space.isValid(lastRMBPressed)) {
        return;
    }
    if(lastRMBPressed == p && selectedPoint != Space::NaN) {
        path = space.BFSfind(space.get1DCoordinates(Point2Du(floor(points[selectedPoint].end.x / rectSize.x), floor(points[selectedPoint].end.y / rectSize.y))), space.get1DCoordinates(p));
        points[selectedPoint].end = get2DWindowCoordinates(p);
        for(auto i : path) {
            points[selectedPoint].PA.addPoint(get2DWindowCoordinates(i));
        }
        points[selectedPoint].PA.constructPath(pointSpeed);
    } else if(!space.stepByStepFilling || space.stepList.empty() && !manualStep) {
        space.unlink(space.get1DCoordinates(lastRMBPressed), space.get1DCoordinates(p));
    }
}
void SpaceRenderer::MMBReleased(Point2Df pos) {
    Point2Du p = Point2Du(floor(pos.x / rectSize.x), floor(pos.y / rectSize.y));
    if(!space.isValid(p) || (space.stepByStepFilling && !space.stepList.empty()) || manualStep)
        return;
    space.disintegrate(space.get1DCoordinates(p));
}
void SpaceRenderer::selectNextPoint() {
    if(points.empty())
        return;
    if(selectedPoint != Space::NaN)
        points[selectedPoint].s.setOutlineThickness(0);
    points[++selectedPoint %= points.size()].s.setOutlineThickness(outlineThickness);
}
void SpaceRenderer::step(bool recursion) {
    static Clock spaceStepListTimer;
    if(space.stepByStepFilling && !space.stepList.empty() && (manualStep || (recursion || spaceStepListTimer.getElapsedTime().asSeconds() > spaceStepListDelay))) {
        spaceStepListTimer.restart();
        space.stepByStepFilling = false;
        Space::Step s = space.stepList.front();
        switch(s.stepType) {
        case Space::LINK:
            space.link(s.stepValue.x, s.stepValue.y);
            break;
        case Space::UNLINK:
            space.unlink(s.stepValue.x, s.stepValue.y);
            break;
        case Space::SETVALUE:
            space[s.stepValue.x].value = s.stepValue.y;
            break;
        }
        space.stepByStepFilling = true;
        space.stepList.pop_front();
        if(!s.endOfStep)
            step(true);
    }
}
void SpaceRenderer::update() {
    for(auto& i : points) {
        Point2Df p = i.PA.getNextCoordinates();
        i.s.setPosition(p ? p : i.end);
    }
    if(!manualStep) step();
}
void SpaceRenderer::draw() {
    drawGrid();
    if(curve) drawCurve();
    drawPath();
    drawPoints();
    drawValues();
    if(maze) drawMaze();
}
void SpaceRenderer::setVisible(Point2Du upLeftCorner, Point2Du downRightCorner) {
    View view = window.getView();
    view.setCenter((upLeftCorner.x + downRightCorner.x) / 2 * rectSize.x, (upLeftCorner.y + downRightCorner.y) / 2 * rectSize.y);
    window.setSize(Vector2u((downRightCorner.x - upLeftCorner.x) * rectSize.x, (downRightCorner.y - upLeftCorner.y) * rectSize.y));
    view.setSize((downRightCorner.x - upLeftCorner.x) * rectSize.x, (downRightCorner.y - upLeftCorner.y) * rectSize.y);
    window.setView(view);
}
Point2Df SpaceRenderer::get2DWindowCoordinates(size_t i) { return get2DWindowCoordinates(space.get2DCoordinates(i)); }
Point2Df SpaceRenderer::get2DWindowCoordinates(Point2Du p) { return Point2Df(p.x * rectSize.x + rectSize.x / 2.0f, p.y * rectSize.y + rectSize.y / 2.0f); }
Point2Du SpaceRenderer::get2DCoordinates(Point2Df p) { return Point2Du(floor(p.x / rectSize.x), floor(p.y / rectSize.y)); }
size_t SpaceRenderer::get1DCoordinates(Point2Df p) { return space.get1DCoordinates(get2DCoordinates(p)); }
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
void SpaceRenderer::drawGrid() {
    RectangleShape rect;
    rect.setSize(rectSize);
    rect.setOutlineThickness(outlineThickness);
    rect.setOutlineColor(outlineColor);
    rect.setFillColor(rectangleColor);
    for(size_t x = 0; x < space.width; ++x) {
        for(size_t y = 0; y < space.height; ++y) {
            if(space.stepByStepFilling) {
                switch(space(x, y).value) {
                case 1:
                    rect.setFillColor(Color::White);
                    break;
                case 2:
                    rect.setFillColor(Color::Blue);
                    break;
                case 3:
                    rect.setFillColor(Color(173, 255, 152));
                    break;
                default:
                    rect.setFillColor(rectangleColor);
                    break;
                }
            }
            rect.setPosition(x * rectSize.x, y * rectSize.y);
            window.draw(rect);
        }
    }
}
void SpaceRenderer::drawCurve() {
    for(size_t i = 0; i<space.size(); ++i) {
        for(size_t j : space[i].next) {
            if(i < j) {
                if(curveThickness != 1) {
                    drawThickLine(get2DWindowCoordinates(i), get2DWindowCoordinates(j), curveThickness, curveColor);
                } else {
                    Vertex line[2] = {Vertex(get2DWindowCoordinates(i), curveColor), Vertex(get2DWindowCoordinates(j), curveColor)};
                    window.draw(line, 2, Lines);
                }
            }
        }
    }
}
void SpaceRenderer::drawPath() {
    size_t current = *path.begin();
    for(auto it = path.begin(); it != path.end(); ++it) {
        if(pathThickness != 1) {
            drawThickLine(get2DWindowCoordinates(current), get2DWindowCoordinates(*it), pathThickness, pathColor);
        } else {
            Vertex line[2] = {Vertex(get2DWindowCoordinates(current), pathColor), Vertex(get2DWindowCoordinates(*it), pathColor)};
            window.draw(line, 2, Lines);
        }
        current = *it;
    }
}
void SpaceRenderer::addPoint(Point2Df p) {
    points.push_back(defaultPoint);
    points.back().s.setPosition(p);
    points.back().end = p;
    if(selectedPoint != Space::NaN)
        points[selectedPoint].s.setOutlineThickness(0);
    points.back().s.setOutlineThickness(outlineThickness);
    selectedPoint = points.size() - 1;
}
void SpaceRenderer::drawPoints() {
    for(auto& i : points) {
        window.draw(i.s);
    }
}
void SpaceRenderer::drawValues() {
    for(size_t i = 0; i<space.size(); ++i) {
        valueAsText.setString(String(to_string(space[i].value)));
        Point2Du c2D = space.get2DCoordinates(i);
        Vector2f c2DWindow = Vector2f(c2D.x * rectSize.x + outlineThickness, c2D.y * rectSize.y + outlineThickness);
        valueAsText.setPosition(c2DWindow);
        window.draw(valueAsText);
    }
}
void SpaceRenderer::drawMaze() {
    for(size_t x = 0; x<space.width; ++x) {
        for(size_t y = 0; y<space.height; ++y) {
            size_t i = space.get1DCoordinates(Point2Du(x, y));
            Point2Df p = Point2Df(x * rectSize.x, y * rectSize.y);
            if(!space[i].linked(space.offset(i, UP)))
                drawThickLine(p, p + Point2Df(rectSize.x, 0), mazeThickness, mazeColor);
            if(!space[i].linked(space.offset(i, LEFT)))
                drawThickLine(p, p + Point2Df(0, rectSize.y), mazeThickness, mazeColor);
        }
    }
}
