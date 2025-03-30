#pragma once
#include "Point2D.h"
#include "SpaceSound.h"
#include "utility.h"

#include <array>
#include <cassert>
#include <cmath>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <ranges>
#include <random>
#include <set>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/polygon/segment_data.hpp>
#include <boost/polygon/voronoi.hpp>

class SpaceRenderer;

/// The main class in which all calculations are performed. It is used by the renderer, but does not depend on it.
class Space {
    friend class SpaceRenderer;
    friend class SpaceGUI;
    Space();

  public:
    enum TilingType { AMORPHOUS = 0, TRIANGLE = 3 /* not implemented */, SQUARE = 4, HEXAGON = 6 };
    TilingType tiling() const;
    size_t width() const;
    size_t height() const;
    size_t size() const;
    /// fills the grid for TRIANGLE, SQUARE and HEXAGON tilings.
    void resize(size_t newWidth, size_t newHeight, TilingType newTiling);

    using VPoint = boost::polygon::point_data<int>;
    using VSegment = boost::polygon::segment_data<int>;
    boost::polygon::voronoi_diagram<double> vd;
    std::vector<VPoint> VoronoiPoints;
    Point2Du VoronoiWindowSize;
    float minDistForEdgeAdjacency = 1.f;
    bool saveIntermediateVPs = true;
    std::list<std::vector<VPoint>> intermediateVPs;
    using Segment = boost::polygon::segment_data<int>;
    /** \brief Constructs Voronoi diagram for AMORPHOUS tiling and fills the grid based on it.
     *  Supports smoothing with Lloyd's algorithm */
    void resize(Point2Df windowSize, int smoothness);

    Space(size_t width, size_t height, TilingType tiling = SQUARE);

    Space(Point2Df windowSize, int smoothness = 3);

    /** \brief Adds a biderected edge in a `grid` graph.
     * When visualisation is enabled, saves steps that can be retrieved later in `getNextFillStep()` */
    bool link(size_t i, size_t with, bool endOfStep = true);
    /// similar to `link()`
    bool unlink(size_t i, size_t with, bool endOfStep = true);
    /// calls `unlink()` for all edges associated with `grid[i]`, resets the values array in it to `Node::defaultValue`
    void disintegrate(size_t i);

    static constexpr size_t NaN = -1, MAX_VALUES = 3;

  private: // forward declaration
    class Node;

  public:
    /// returns a reference to `grid[i]`
    Node& operator[](size_t i);
    /// returns a reference to `grid[get1DCoordinates(x, y)]`
    Node& operator()(size_t x, size_t y);
    /// return a reference to `grid[get1DCoordinates(p.x, p.y)]`
    Node& operator()(Point2Du p);
    /// converts 1D array coordinates to corresponding 2D coordinates (AMORPHOUS tiling is not supported)
    Point2Du get2DCoordinates(size_t i) const;
    /// similar to `get2DCoordinates()`
    size_t get1DCoordinates(Point2Du p) const;
    // these variables determine whether the data is saved for visualisation or sonification
    bool stepByStepFill = false, stepByStepPath = false, isSonification = false;
    enum StepType { SETVALUE1 = 0, SETVALUE2, SETVALUE3, SETCOLOR, LINK, UNLINK, SETNEXTPATH, SETPARENT };
    struct Step {
        Point2Du stepValue;
        StepType stepType;
        bool endOfStep;
    };
    std::list<Step> processedSteps;
    std::mutex processedStepsMutex;
    std::list<Step> fillStepList;
    /** \brief If `fillStepList` is empty, returns nullptr, returns the front element otherwise.
      * When `isSonification` is true, saves steps for further processing in `sonificationTick()` */
    std::optional<Step> getNextFillStep();
    std::list<Step> pathStepList;
    /// similar to `getNextFillStep()`
    std::optional<Step> getNextPathStep();
    struct SonificationSettings {
        unsigned samplesN = 1024;
        MetaUint mi_1 = {{.v = &samplesN, .name = Tr("Number of samples")}, {.min = 1, .max = 44100, .step = 64}};
        /** \brief Defines the minimum required number of active oscillators to prevent sound distortion.
          *  Additional oscillators are being obtained by reusing expired oscillators */
        unsigned oscillatorsTarget = 5;
        MetaUint mi_2 = {{.v = &oscillatorsTarget, .name = Tr("Oscillators reuse rate")}, {.step = 5}};
        /// maximum number of oscillators so as not to overload the callback and replace old ones
        unsigned maxOscillatorsN = 48;
        MetaUint mi_3 = {{.v = &maxOscillatorsN, .name = Tr("Oscillators limit")}, {.min = 1, .max = 99999, .step = 8}};
        /// number of samples for which each oscillator operates
        unsigned oscDuration = 1247;
        MetaUint mi_4 = {{.v = &oscDuration, .name = Tr("Oscillators duration")}, {.max = 44100, .step = 100}};
        double freq_mult = 1;
        MetaDouble mi_5 = {{.v = &freq_mult, .name = Tr("Frequency multiplier")}, {.max = 16.0, .step = 0.1}};
        int freq_add = 0;
        MetaInt mi_6 = {{.v = &freq_add, .name = Tr("Frequency addend")}, {.min = -32000, .max = 32000, .step = 500}};
        MetaDouble mi_7 = {{.v = &SpaceSound::oscCompletionThreshold, .name = Tr("Oscillator reuse threshold")}, {.max = 1.0, .step = 0.05}};
        MetaDouble mi_8 = {{.v = &SpaceSound::oscAliveThreshold, .name = Tr("Oscillator contribution threshold")}, {.max = 1.0, .step = 0.05}};
        MetaDouble mi_9 = {{.v = &SpaceSound::volumeSmoothingFactor, .name = Tr("Volume smoothing factor")}, {.max = 1.0, .step = 0.05}};
        struct OscillatorSettings {
            MetaMap mi_1 = {{.v = &SpaceSound::Oscillator::currentWave, .name = Tr("Waveform")}, {{Tr("sine wave"), Tr("sin^3 wave"), Tr("triangle wave")}}};
            MetaDouble mi_2 = {{.v = &SpaceSound::Oscillator::attack, .name = Tr("Attack (% of duration)")}, {.max = 1.0, .step = 0.05}};
            MetaDouble mi_3 = {{.v = &SpaceSound::Oscillator::decay, .name = Tr("Decay (% of duration)")}, {.max = 1.0, .step = 0.05}};
            MetaDouble mi_4 = {{.v = &SpaceSound::Oscillator::sustain, .name = Tr("Sustain (% of amplitude)")}, {.max = 1.0, .step = 0.05}};
            MetaDouble mi_5 = {{.v = &SpaceSound::Oscillator::release, .name = Tr("Release (% of duration)")}, {.max = 1.0, .step = 0.05}};
        } oscSets;
        MetaStruct<OscillatorSettings> mi_10 = {{.v = &oscSets, .name = Tr("Oscillator sound characteristics")}};
    } sonSets;
    SpaceSound sCalc;
    /** \brief This method can be called by stk or some other audio library.
     *  Fills `samples` array with values between -1.0 and 1.0 based on processedSteps */
    int sonificationTick(double* samples, unsigned int nBufferFrames, unsigned sampleRate, std::function<double(size_t)> getFrequency);
    /// returns the oldest path from `paths` (used by A* visualization for a lightning-like effect)
    std::list<size_t> getNextPath();
    /// assigns `defaultValue` to all values in all nodes in `grid`
    void defaultAllValues();
    void prePathAlgInit();
    static thread_local inline std::random_device rd;
    static thread_local inline std::default_random_engine dre;
    size_t gridResolution = 1;
    // filling algorithms
    /// `disintegrate()`'s all nodes
    void clear();
    /// clears colors in reverse order
    void clearFillColors(int color = 2, bool removeAllJustOnce = false /*set rate for every generator*/);
    /// links all nodes with all corresponding available dirs offsets
    void floodFill();
    // *** for a description of the algorithms and related links, see the pdfs folder or use the corresponding button in the interface to open them ***
    void recursiveBacktrackerMaze();
    // TODO: Wrap algorithms and their properties.
    double EllersMazeVerticalProbability = 0.5;
    void EllersMaze();
    void KruskalsMaze();
    void PrimsMaze();
    void recursiveDivisionMaze();
    void AldousBroderMaze();
    void WilsonsMaze();
    void huntAndKillMaze();
    struct GrowingTreeMazeSettings {
        static size_t newestMiddleOldestRandom(const std::deque<size_t>& p, unsigned pNewest, unsigned pMiddle, unsigned pOldest, unsigned pRandom);
        static size_t newest(const std::deque<size_t>& p) { return newestMiddleOldestRandom(p, 1, 0, 0, 0); }
        static size_t middle(const std::deque<size_t>& p) { return newestMiddleOldestRandom(p, 0, 1, 0, 0); }
        static size_t oldest(const std::deque<size_t>& p) { return newestMiddleOldestRandom(p, 0, 0, 1, 0); }
        static size_t random(const std::deque<size_t>& p) { return newestMiddleOldestRandom(p, 0, 0, 0, 1); }
        static inline std::array presets = {newest, middle, oldest, random};
        std::function<size_t(const std::deque<size_t>&)> selectPointAlgorithm = newest; // set externally in accordance with the user choice
        int currentAlgorithm = 0;
        int newestProp = 12, middleProp = 48, oldestProp = 34, randomProp = 37;
        MetaInt mi_2 = {{.v = &newestProp, .name = Tr("Share of newest points"), .enabled = false}, {.min = 0}};
        MetaInt mi_3 = {{.v = &middleProp, .name = Tr("Share of middle points"), .enabled = false}, {.min = 0}};
        MetaInt mi_4 = {{.v = &oldestProp, .name = Tr("Share of oldest points"), .enabled = false}, {.min = 0}};
        MetaInt mi_5 = {{.v = &randomProp, .name = Tr("Share of random points"), .enabled = false}, {.min = 0}};
        MetaMap mi_1 = {{.v = &currentAlgorithm,
                         .name = Tr("Cell selection method"),
                         .setter =
                             [&](int v) {
                                 currentAlgorithm = v;
                                 if (v < presets.size()) {
                                     mi_2.enabled = mi_3.enabled = mi_4.enabled = mi_5.enabled = false;
                                     selectPointAlgorithm = presets[v];
                                 } else {
                                     using namespace std::placeholders;
                                     mi_2.enabled = mi_3.enabled = mi_4.enabled = mi_5.enabled = true;
                                     selectPointAlgorithm = std::bind(newestMiddleOldestRandom, _1, std::ref(newestProp), std::ref(middleProp), std::ref(oldestProp),
                                                                      std::ref(randomProp));
                                 }
                                 (*mi_1.regenerateDialog)();
                             }},
                        {{Tr("Newest (recursive backtracker)"), Tr("Middle"), Tr("Oldest"), Tr("Random (identical to Prim's)"), Tr("Combination")}}};
    } growingTreeMazeSets;
    void growingTreeMaze();
    static constexpr std::array fillArr = {&Space::clear,           &Space::recursiveBacktrackerMaze, &Space::EllersMaze,       &Space::KruskalsMaze,
                                           &Space::PrimsMaze,       &Space::recursiveDivisionMaze,    &Space::AldousBroderMaze, &Space::WilsonsMaze,
                                           &Space::huntAndKillMaze, &Space::growingTreeMaze};
    static constexpr std::array<std::array<bool, fillArr.size()>, 5> availableFillings = {
        {{1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 0, 1, 1, 0, 1, 1, 1, 1}, {1, 1, 0, 1, 1, 0, 1, 1, 1, 1}, {1, 1, 0, 1, 1, 0, 1, 1, 1, 1}}};
    void merge(const std::vector<std::unique_ptr<Node>>& grid, const std::vector<std::vector<bool>>& mask); // todo: for other grids (application?)
    // traversing/pathfinding algorithms
    std::list<size_t> BFSFind(size_t from, size_t to);
    std::function<float(size_t, size_t)> calcWeightFunc; // set externally in accordance with the tiling used
    std::list<size_t> AStarFind(size_t from, size_t to);
    void DFS(size_t from);
    static constexpr std::array pathArr = {&Space::BFSFind, &Space::AStarFind};

  private:
    TilingType _tiling;
    size_t _width, _height;
    void addFillStep(Point2Du stepValue, StepType stepType, bool endOfStep = true);
    void addPathStep(Point2Du stepValue, StepType stepType, bool endOfStep = true);
    struct Node {
        Node(size_t i);
        Node(const Node&) = default;
        virtual ~Node() {}
        virtual std::vector<size_t> getAvailableDirs() const;
        bool empty() const;
        bool linked(size_t with) const;
        virtual size_t offset(size_t dir) const = 0; // returns NaN if there is a wall in dir
        bool link(size_t with);
        std::set<size_t>::size_type unlink(size_t with);
        std::set<size_t> next;
        static const size_t defaultValue;
        std::vector<size_t> values;
        size_t i;
        static thread_local Space* space;
    };
    struct SquareNode : public Node {
        using Node::Node;
        enum Directions { RIGHT, DOWN, LEFT, UP };
        size_t offset(size_t dir) const override;
        size_t offsetR() const;
        size_t offsetD() const;
        size_t offsetL() const;
        size_t offsetU() const;
        bool wallR() const;
        bool wallD() const;
        bool wallL() const;
        bool wallU() const;
    };
    struct HexagonNode : public Node {
        using Node::Node;
        enum Directions { RIGHT, DOWNRIGHT, DOWNLEFT, LEFT, UPLEFT, UPRIGHT };
        size_t offset(size_t dir) const override;
        size_t offsetR() const;
        size_t offsetDR() const;
        size_t offsetDL() const;
        size_t offsetL() const;
        size_t offsetUL() const;
        size_t offsetUR() const;
        bool wallR() const;
        bool wallDR() const;
        bool wallDL() const;
        bool wallL() const;
        bool wallUL() const;
        bool wallUR() const;
    };
    struct AmorphousNode : public Node {
        std::map<size_t, size_t> neighbours;
        AmorphousNode(size_t i, std::map<size_t, size_t> neighbours);
        std::vector<size_t> getAvailableDirs() const override;
        size_t offset(size_t dir) const override;
    };
    std::vector<std::unique_ptr<Node>> grid;
    // helper functions
    bool moveFrom(size_t& from, size_t to);
    struct selectRandomDirRT {
        size_t next, dir;
    };
    selectRandomDirRT selectRandomDir(size_t i, std::function<bool(size_t)> condition = [](size_t i) { return true; });
    std::optional<Point2Df> lineSegmentsIntersection(Point2Df AV0, Point2Df AV1, Point2Df BV0, Point2Df BV1);
    Point2Df lineWindowIntersection(Point2Df AV0, Point2Df AV1, Point2Df windowSize);
    void setValue(size_t i, size_t vi, size_t v, bool endOfStep = true);
    std::list<std::list<size_t>> paths;
    void addPath(std::list<size_t> path);
    std::list<size_t> constructPath(const std::vector<size_t>& parent, size_t from, size_t to);
};
