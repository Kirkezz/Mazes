#pragma once
#include "PointAnimation.h"
#include "Space.h"

#include <vector>

#include <SFML/Graphics.hpp>
class SpaceRenderer {
public:
    void clear();
    void loadSpaceProps(sf::Vector2f newShapeSize);
    SpaceRenderer(Space& space, sf::RenderWindow& window, sf::Vector2f shapeSize);
    void update();
    bool isGridDrawn = true, isCurveDrawn = true, isPathDrawn = true, isPointsDrawn = true, isDebugInfoDrawn = false, isMazeDrawn = true;
    void draw();
    // events
    size_t lastLMBPressed, lastRMBPressed;
    void LMBPressed(sf::Vector2f pos);
    void LMBReleased(sf::Vector2f pos);
    void RMBPressed(sf::Vector2f pos);
    void RMBReleased(sf::Vector2f pos);
    void MMBReleased(sf::Vector2f pos);
    void selectNextPoint(); // Keyboard::Tab
    bool manualStep = false;
    float spaceStepListDelay = 0.5f;
    void step(bool recursion = false); // manualStep ? Keyboard::Space : every spaceStepListDelay seconds
    // customisation
    float outlineThickness = 2.0f, curveThickness = 2.0f, pathThickness = 3.0f, pointSpeed = 8.0f, mazeThickness = 2.0f;
    sf::Color shapeColor = sf::Color(0, 30, 50), outlineColor = sf::Color(26, 113, 185), curveColor = sf::Color(255, 42, 109), pathColor = sf::Color(223, 101, 148),
              mazeColor = sf::Color(5, 217, 232), backgroundColor = sf::Color(1, 25, 15), pointsColor = curveColor;
    std::vector<sf::ConvexShape> shapes;
    std::list<size_t> path;
private:
    size_t _tiling;
    Space& space;
    sf::RenderWindow& window;
    sf::Vector2f shapeSize;
    void drawGrid();
    void drawCurve();
    void drawPath();
    struct AnimatedPoint {
        sf::CircleShape s;
        PointAnimation PA;
        size_t end;
    } samplePoint;
    std::vector<AnimatedPoint> points;
    void addPoint(sf::Vector2f p);
    size_t selectedPoint = Space::NaN;
    void drawPoints();
    sf::Font font;
    sf::Text valueAsText;
    void drawValues();
    void drawMaze();
    // helper functions
    double radiansToDegrees(double r) const;
    double degreesToRadians(double d) const;
    sf::Vector2f get2DWindowCoordinates(Point2Du p) const;
    sf::Vector2f get2DWindowCoordinates(size_t i) const;
    Point2Du mouseTo2DSpaceCoordinates(sf::Vector2f p) const;
    size_t mouseTo1DSpaceCoordinates(sf::Vector2f p) const;
    void drawThickLine(Point2Df a, Point2Df b, float thickness, sf::Color color);
    // if cells are adjacent, draws a curve through the center of the adjacent edge
    void drawThickSpaceLine(size_t a, size_t b, float thickness, sf::Color color);
    sf::Vector2f getValueCoordinates(size_t i, size_t vi) const;
    bool isPointInsideShape(Point2Df p, const sf::ConvexShape& shape) const;
    std::optional<Point2Df> lineSegmentsIntersection(Point2Df AV0, Point2Df AV1, Point2Df BV0, Point2Df BV1);
    Point2Df lineWindowIntersection(Point2Df AV0, Point2Df AV1);
};
