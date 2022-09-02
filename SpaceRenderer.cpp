#include "SpaceRenderer.h"
#include <cassert>
using namespace sf;
void SpaceRenderer::loadSpaceProps() {
    rectSize = Vector2f(window.getSize().x / space.width, window.getSize().y / space.height);
    defaultPoint.s.setRadius(std::min(rectSize.x, rectSize.y) / 8.0f);
    defaultPoint.s.setFillColor(curveColor);
    defaultPoint.s.setOrigin(defaultPoint.s.getRadius(), defaultPoint.s.getRadius());
    defaultPoint.s.setOutlineColor(outlineColor);
    points.clear();
    selectedPoint = 0;
    addPoint(get2DWindowCoordinates(space.begin));
    points[selectedPoint].s.setOutlineThickness(outlineThickness);
}
SpaceRenderer::SpaceRenderer(Space& space, RenderWindow& window) : space(space), window(window) {
    loadSpaceProps();
}
void SpaceRenderer::LMBReleased(Point2Df pos) {
    addPoint(get2DWindowCoordinates(Point2Du(floor(pos.x / rectSize.x), floor(pos.y / rectSize.y))));
}
void SpaceRenderer::RMBReleased(Point2Df pos) {
    size_t to1D = get1DCoordinates(pos), i = get1DCoordinates(points[selectedPoint].end);
    if(to1D == i) {
        return;
    }
    points[selectedPoint].PA.addPoint(points[selectedPoint].end);
    while(i != to1D) {
        i = (space[to1D].value > space[i].value) ? space[i].next : space[i].prev;
        points[selectedPoint].PA.addPoint(get2DWindowCoordinates(i));
    }
    points[selectedPoint].end = get2DWindowCoordinates(get2DCoordinates(pos));
    points[selectedPoint].PA.constructPath(pointSpeed);
}
void SpaceRenderer::selectNextPoint() {
    points[selectedPoint].s.setOutlineThickness(0);
    points[++selectedPoint %= points.size()].s.setOutlineThickness(outlineThickness);
}
void SpaceRenderer::update() {
    for(auto& i : points) {
        Point2Df p = i.PA.getNextCoordinates();
        i.s.setPosition(p ? sf::Vector2f(p.x, p.y) : sf::Vector2f(i.end.x, i.end.y));
    }
}
void SpaceRenderer::draw() {
    drawGrid();
    if (space.begin == Space::NaN)
        return;
    drawCurve();
    drawPoints();
}
Point2Df SpaceRenderer::get2DWindowCoordinates(size_t i) { return get2DWindowCoordinates(space.get2DCoordinates(i)); }
Point2Df SpaceRenderer::get2DWindowCoordinates(Point2Du p) { return Point2Df(p.x * rectSize.x + rectSize.x / 2.0f, p.y * rectSize.y + rectSize.y / 2.0f); }
Point2Du SpaceRenderer::get2DCoordinates(Point2Df p) { return Point2Du(floor(p.x / rectSize.x), floor(p.y / rectSize.y)); }
size_t SpaceRenderer::get1DCoordinates(Point2Df p) { return space.get1DCoordinates(get2DCoordinates(p)); }
void SpaceRenderer::drawGrid() {
    RectangleShape rect;
    rect.setSize(rectSize);
    rect.setOutlineThickness(outlineThickness);
    rect.setOutlineColor(outlineColor);
    rect.setFillColor(rectangleColor);
    for (size_t x = 0; x < space.width; ++x) {
        for (size_t y = 0; y < space.height; ++y) {
            rect.setPosition(x * rectSize.x, y * rectSize.y);
            window.draw(rect);
        }
    }
}
void SpaceRenderer::drawCurve() {
    size_t i = space.begin;
    Point2D i2d = space.get2DCoordinates(i), next2d = space.get2DCoordinates(space[i].next);
    while (space(i).next != Space::NaN) {
        i = space(i).next;
        Vertex line[2] = { Vertex({i2d.x * rectSize.x + rectSize.x / 2, i2d.y * rectSize.y + rectSize.y / 2}, curveColor),
                           Vertex({next2d.x * rectSize.x + rectSize.x / 2, next2d.y * rectSize.y + rectSize.y / 2}, curveColor) }; // TODO: curveThickness
        window.draw(line, 2, Lines);
        i2d = std::move(next2d);
        next2d = space.get2DCoordinates(space[i].next);
    }
}
void SpaceRenderer::addPoint(Point2Df p) {
    points.push_back(defaultPoint);
    points.back().s.setPosition(sf::Vector2f(p.x, p.y));
    points.back().end = p;
}
void SpaceRenderer::drawPoints() {
    for(auto& i : points) {
        window.draw(i.s);
    }
}
