#pragma once
#include "PointAnimation.h"
#include "Space.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <thread>
#include <vector>

#include <SFML/Graphics.hpp>

#define __OS_LINUX__
#include <stk/RtAudio.h>
#include <stk/RtWvOut.h>
#include <stk/SineWave.h>

/// A class that handles `Space` rendering, visualization and sonification, mouse and keyboard events.
class SpaceRenderer {
  public:
    Space* space;
    sf::RenderWindow& window;

    int antialiasingLevel = 4;
    /** \brief Resizes window based on `newSize` and data stored in `Space`. Fills `shapes` correspondingly
      * @param newSize for AMORPHOUS tiling is a full window size, for other tilings it is a single cell size */
    void loadSpaceProps(sf::Vector2f newShapeSize, bool create = false);
    /// loads assets and calls `loadSpaceProps`
    SpaceRenderer(Space& space, sf::RenderWindow& window, sf::Vector2f shapeSize);
    void setSpace(Space* space, sf::Vector2f shapeSize) {
        this->space = space;
        Space::Node::space = space;
        loadSpaceProps(shapeSize);
    }
    /// updates points, handles `fillStep()` and `pathStep()`, starts and stops audio stream
    void update();
    bool isGridDrawn = true, isCurveDrawn = true, isPathDrawn = true, isPointsDrawn = true, isDebugInfoDrawn = false, isMazeDrawn = true;
    void draw();
    /// determines the minimum time between iterations of the main loop (in milliseconds)
    double frameDuration = 16.666;
    // events
    size_t lastLMBPressed, lastRMBPressed;
    void LMBPressed(sf::Vector2f pos);
    /// adds a point if mouse is released in the same cell it was pressed, otherwise links corresponding nodes
    void LMBReleased(sf::Vector2f pos);
    void RMBPressed(sf::Vector2f pos);
    /// inits pathfinding and moves a point or unlinks nodes
    void RMBReleased(sf::Vector2f pos);
    /// performs `disintegrate()` for a cell
    void MMBReleased(sf::Vector2f pos);
    void deleteSelectedPoint(); // Keyboard::Delete
    void selectNextPoint();     // Keyboard::Tab
    bool manualFillStep = false, manualPathStep = false;
    double spaceFillStepListDelay = 0.0, spacePathStepListDelay = 0.0, voronoiVisualizationDelay = 0.1;
    // manualStep ? Keyboard::Space : every spaceStepListDelay seconds
    /// step types: LINK, UNLINK, SETCOLOR (displayed when `isGridDrawn` is on) or SETVALUE (`isDebugInfoDrawn`)
    void fillStep(bool recursion = false);
    /// step types: SETCOLOR, SETNEXTPATH (`Space::getNextPath()`), SETPARENT (BFS `isDebugInfoDrawn` arrows)
    void pathStep(bool recursion = false);
    class SpaceAudioStream {
        SpaceRenderer* renderer;
        unsigned frames;
        RtAudio dac;
        bool playing = false;
      public:
        SpaceAudioStream(SpaceRenderer* renderer);
        void play();
        void stop();
        ~SpaceAudioStream() { stop(); }
      private:
        static int sonificationTick(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status,
                                    void* dataPointer) {
            static SpaceAudioStream* data;
            data = (SpaceAudioStream*) dataPointer;
            static std::function<double(size_t)> getFrequency = [&](size_t p) {
                auto [width, height] = data->renderer->window.getSize();
                // return 65.41 * pow(pow(2., (1. / 12)), (Point2Df(0, 0).distance(Point2Df(get2DWindowCoordinates(p))) * 84 / (Point2Df(0, 0).distance(Point2Df(width, height)))));
                return 120 + 1200. * Point2Df(width / 2, height / 2).distance(Point2Df(data->renderer->get2DWindowCoordinates(p))) / std::max(width, height);
            };
            auto res = data->renderer->space->sonificationTick((double*) outputBuffer, nBufferFrames, stk::Stk::sampleRate(), getFrequency);
            //for (int i = 0; i < nBufferFrames; ++i)
            //    std::cout << ((double*) outputBuffer)[i] << " ";
            //std::cout << std::endl;
            return res;
        }
    } audioStream;
    void audioInit();
    void startAudioStream();
    void stopAudioStream();
    // customization
    enum ColorScheme {
        DEFAULTCOLOR = 0,
        COLOR1,
        COLOR2,
        COLOR3,
        COLOR4,
        OUTLINECOLOR,
        CURVECOLOR,
        PATHCOLOR,
        MAZECOLOR,
        BACKGROUNDCOLOR,
        POINTSCOLOR,
        TEXTCOLOR,
        NUM_COLORS
    };
    std::array<sf::Color, NUM_COLORS> colorScheme;
    float outlineThickness = 2.0f, curveThickness = 2.0f, pathThickness = 3.0f, pointSpeed = 8.0f, mazeThickness = 4.0f;
    std::vector<sf::ConvexShape> shapes;
    sf::Vector2f shapeSize;
    size_t selectedPathAlg = 0;
    struct PathfindingSettings {
        int curCalcWeightFunc;
        MetaMap mi_1 = {{.v = &curCalcWeightFunc, .name = Tr("Heuristic"), .setter = [&](auto v) { *mi_1.v = v; }},
                        {{Tr("Manhattan distance"), Tr("Euclidean distance")}}};
    } pathSets;
    std::list<size_t> path;
    size_t getPointsSize();
    std::vector<Point2Df> getPoints();

  private:
    size_t _tiling;
    std::vector<int8_t> colors;
    std::vector<size_t> parent;
    void updateAmorphousShapes(const boost::polygon::voronoi_diagram<double>& vd, const std::vector<Space::VPoint>& VoronoiPoints);
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
    void drawDebugInfo();
    void drawMaze();
    sf::Texture t_arrowR;
    sf::Sprite s_arrowR;
    // helper functions
    double radiansToDegrees(double r) const;
    double degreesToRadians(double d) const;
    double getDegrees(sf::Vector2f a, sf::Vector2f b) const;
    sf::Vector2f get2DWindowCoordinates(Point2Du p) const;
    sf::Vector2f get2DWindowCoordinates(size_t i) const;
    Point2Du mouseTo2DSpaceCoordinates(sf::Vector2f p) const;
    size_t mouseTo1DSpaceCoordinates(sf::Vector2f p) const;
    void drawThickLine(Point2Df a, Point2Df b, float thickness, sf::Color color);
    // if cells are adjacent, draws a curve through the center of the adjacent edge
    void drawThickSpaceLine(size_t a, size_t b, float thickness, sf::Color color);
    void drawWindowBorders();
    sf::Vector2f getValueCoordinates(size_t i, size_t vi) const;
    bool isPointInsideShape(Point2Df p, const sf::ConvexShape& shape) const;
    int det(int a, int b, int c, int d);
    bool between(int a, int b, double c);
    bool intersect_1(int a, int b, int c, int d);
    std::optional<Point2Df> lineSegmentsIntersection(Point2Df AV0, Point2Df AV1, Point2Df BV0, Point2Df BV1);
    Point2Df lineWindowIntersection(Point2Df AV0, Point2Df AV1);
    bool isPointInsideWindow(Point2Df p) const;

  public:
    bool saveScreenshot(const std::string& filename) {
        sf::Texture texture;
        texture.create(window.getSize().x, window.getSize().y);
        texture.update(window);
        return texture.copyToImage().saveToFile(filename);
    }
    void setAllPoints() {
        for (size_t i = 0; i < shapes.size(); ++i) {
            addPoint(shapes[i].getPosition());
        }
    }
    void convertVideoFrame(int frame) { loadSpaceProps(Point2Df(3, 3)); }
};
