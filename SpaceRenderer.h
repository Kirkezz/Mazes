#pragma once
#include "PointAnimation.h"
#include "Space.h"

#include <vector>

#include <SFML/Graphics.hpp>
class SpaceRenderer {
public:
    void preInit();
    sf::Vector2f rectSize;
    void rectSizeInit();
    void loadSpaceProps();
    void clear();
	SpaceRenderer(Space& space, sf::RenderWindow& window);
    SpaceRenderer(Space& space, sf::RenderWindow& window, sf::Vector2f rectSize);
    void LMBPressed(Point2Df pos);
    void LMBReleased(Point2Df pos);
    std::list<size_t> path;
    void RMBPressed(Point2Df pos);
    void RMBReleased(Point2Df pos);
    void MMBReleased(Point2Df pos);
    void selectNextPoint();
    float spaceStepListDelay = 0.5f;
    void step(bool recursion = false);
    void update();
	void draw();
    void setVisible(Point2Du upLeftCorner, Point2Du downRightCorner);
    float outlineThickness = 1.0f,
          curveThickness = 2.0f,
          pathThickness = 3.0f,
          pointSpeed = 8.0f,
          mazeThickness = 2.0f;
    sf::Color rectangleColor = sf::Color(0, 30, 50),
              outlineColor = sf::Color(26, 113, 185),
              curveColor = sf::Color(255, 42, 109),
              pathColor = sf::Color(223, 101, 148),
              mazeColor = sf::Color(5, 217, 232),
              pointsColor = curveColor;
    bool curve = true, maze = false, manualStep = false;
private:
    Space& space;
    sf::RenderWindow& window;
    bool constWindowSize;
    Point2Du lastLMBPressed, lastRMBPressed;
    Point2Df get2DWindowCoordinates(size_t i);
    Point2Df get2DWindowCoordinates(Point2Du p);
    Point2Du get2DCoordinates(Point2Df p);
    size_t get1DCoordinates(Point2Df p);
    void drawThickLine(Point2Df a, Point2Df b, float thickness, sf::Color color);
	void drawGrid();
    void drawCurve();
    void drawPath();
    struct AnimatedPoint {
        sf::CircleShape s;
        PointAnimation PA;
        Point2Df end;
    };
    AnimatedPoint defaultPoint;
    std::vector<AnimatedPoint> points;
    void addPoint(Point2Df p);
    size_t selectedPoint = Space::NaN;
	void drawPoints();
    void drawValues();
    sf::Font font;
    sf::Text valueAsText;
    void drawMaze();
};

