#pragma once
#include "PointAnimation.h"
#include "Space.h"

#include <vector>

#include <SFML/Graphics.hpp>
class SpaceRenderer {
public:
	void loadSpaceProps();
	SpaceRenderer(Space& space, sf::RenderWindow& window);
    void LMBReleased(Point2Df pos);
    void RMBReleased(Point2Df pos);
    void selectNextPoint();
	void update();
	void draw();
    float outlineThickness = 1.0f,
          curveThickness = 2.0f,
          pointSpeed = 8.0f;
    sf::Color rectangleColor = sf::Color(206, 240, 239),
			  outlineColor = sf::Color(47, 189, 185),
              curveColor = sf::Color::Magenta;
private:
    Space& space;
    sf::RenderWindow& window;
	sf::Vector2f rectSize;
    Point2Df get2DWindowCoordinates(size_t i);
    Point2Df get2DWindowCoordinates(Point2Du p);
    Point2Du get2DCoordinates(Point2Df p);
    size_t get1DCoordinates(Point2Df p);
	void drawGrid();
    void drawCurve();
    struct AnimatedPoint {
        sf::CircleShape s;
        PointAnimation PA;
        Point2Df end;
    };
    AnimatedPoint defaultPoint;
    std::vector<AnimatedPoint> points;
    void addPoint(Point2Df p);
    size_t selectedPoint = 0;
	void drawPoints();
};

