#include "Space.h"
using namespace std;
namespace ranges = std::ranges;
namespace views = ranges::views;

Space::Space() : sCalc(sonSets.maxOscillatorsN) {
    Node::space = this;
    dre.seed(rd());
}
// public
Space::TilingType Space::tiling() const { return _tiling; }
size_t Space::width() const { return _width / gridResolution; }
size_t Space::height() const { return _height / gridResolution; }
size_t Space::size() const { return /*grid.size()*/ tiling() == AMORPHOUS ? grid.size() : (width() * height()); }
void Space::resize(size_t newWidth, size_t newHeight, TilingType newTiling) {
    grid.clear();

    _width = newWidth;
    _height = newHeight;
    grid.reserve(_width * _height);

    _tiling = newTiling;
    for (size_t i = 0; i < _width * _height; ++i) {
        switch (_tiling) {
        case TRIANGLE: /*grid.push_back(make_unique<TriangleNode>());*/ break;
        case SQUARE: grid.push_back(make_unique<SquareNode>(i)); break;
        case HEXAGON: grid.push_back(make_unique<HexagonNode>(i)); break;
        case AMORPHOUS: break;
        }
    }
}
void Space::resize(Point2Df windowSize, int smoothness) {
    auto isInsideWindow = [windowSize](const boost::polygon::voronoi_edge<double>::voronoi_vertex_type* v) {
        return v && v->x() >= 0. && v->y() >= 0. && v->x() < windowSize.x && v->y() < windowSize.y;
    };

    VoronoiWindowSize = Point2Du(windowSize.x, windowSize.y);
    assert(!VoronoiPoints.empty());
    _width = NaN;
    _height = NaN;
    _tiling = AMORPHOUS;

    vector<Segment> segments;
    segments.emplace_back(VPoint(-windowSize.x * 3., -windowSize.y * 3.), VPoint(windowSize.x * 3., -windowSize.y * 3.));
    segments.emplace_back(VPoint(windowSize.x * 3., -windowSize.y * 3.), VPoint(windowSize.x * 3., windowSize.y * 3.));
    segments.emplace_back(VPoint(windowSize.x * 3., windowSize.y * 3.), VPoint(-windowSize.x * 3., windowSize.y * 3.));
    segments.emplace_back(VPoint(-windowSize.x * 3., windowSize.y * 3.), VPoint(-windowSize.x * 3., -windowSize.y * 3.));

    intermediateVPs.clear();
    vd.clear();

    for (size_t i = 0; i < smoothness; ++i) { // Lloyd's algorithm
        sort(VoronoiPoints.begin(), VoronoiPoints.end());
        boost::polygon::construct_voronoi(VoronoiPoints.begin(), VoronoiPoints.end(), segments.begin(), segments.end(), &vd);

        vector<VPoint> nextPoints;
        nextPoints.reserve(VoronoiPoints.size());
        for (auto& cell : vd.cells()) {
            if (!cell.contains_point()) continue;

            auto edge = cell.incident_edge();
            set<Point2Df> vertices;
            do {
                if (edge->is_primary()) {
                    auto v0 = edge->vertex0(), v1 = edge->vertex1();
                    bool insideV0 = isInsideWindow(v0), insideV1 = isInsideWindow(v1);
                    if (insideV0) vertices.insert(Point2Df(v0->x(), v0->y()));
                    if (insideV1) vertices.insert(Point2Df(v1->x(), v1->y()));
                    if (Point2Df t = lineWindowIntersection(Point2Df(v0->x(), v0->y()), Point2Df(v1->x(), v1->y()), windowSize); t && (insideV0 ^ insideV1)) {
                        vertices.insert(t);
                    }
                }
                edge = edge->next();
            } while (edge != cell.incident_edge());

            if (!vertices.empty()) {
                Point2Df result(0, 0);
                for (auto& i : vertices) {
                    result += i;
                }
                result /= vertices.size();
                nextPoints.push_back(VPoint(result.x, result.y));
            }
        }

        if (saveIntermediateVPs) intermediateVPs.push_back(std::move(VoronoiPoints));
        VoronoiPoints = std::move(nextPoints);
        vd.clear();
    }

    grid.clear();
    grid.reserve(VoronoiPoints.size());

    // "All the site objects are constructed and sorted at the algorithm initialization step".
    // I sort it before, because I want to get point by cell->source_index.
    sort(VoronoiPoints.begin(), VoronoiPoints.end());
    boost::polygon::construct_voronoi(VoronoiPoints.begin(), VoronoiPoints.end(), segments.begin(), segments.end(), &vd);
    for (auto& cell : vd.cells()) {
        if (!cell.contains_point()) continue;

        map<size_t, size_t> neighbours;
        auto edge = cell.incident_edge();
        int n = 0;
        do {
            const auto& twin = edge->twin();
            if (twin->is_primary() && twin->cell()->contains_point() && (isInsideWindow(twin->vertex0()) || isInsideWindow(twin->vertex1()))
                && Point2Df(twin->vertex0()->x(), twin->vertex0()->y()).distance(Point2Df(twin->vertex1()->x(), twin->vertex1()->y())) >= minDistForEdgeAdjacency)
                neighbours.insert({n, twin->cell()->source_index()});
            edge = edge->next();
            ++n;
        } while (edge != cell.incident_edge());

        if (!neighbours.empty() && n > 2) grid.push_back(make_unique<AmorphousNode>(grid.size(), neighbours));
    }
}
Space::Space(size_t width, size_t height, TilingType tiling) : Space() { resize(width, height, tiling); }
Space::Space(Point2Df windowSize, int smoothness) : Space() { resize(windowSize, smoothness); }
bool Space::link(size_t i, size_t with, bool endOfStep) {
    if (tiling() == SQUARE && gridResolution != 1) {
        addFillStep({i, with}, LINK, endOfStep);
        bool result = true;
        bool right = grid[i]->offset(SquareNode::RIGHT) == with, left = grid[i]->offset(SquareNode::LEFT) == with;
        bool down = grid[i]->offset(SquareNode::DOWN) == with, up = grid[i]->offset(SquareNode::UP) == with;
        if (right || left) {
            for (size_t n = 0; n < gridResolution; ++n) {
                size_t iScaled = ((i % width()) * gridResolution) + ((i / width()) * gridResolution) * _width
                                 + (right ? (n * _width + gridResolution - 1) : (n * _width));
                size_t withScaled = ((with % width()) * gridResolution) + ((with / width()) * gridResolution) * _width
                                    + (right ? (n * _width) : (n * _width + gridResolution - 1));
                result &= grid[iScaled]->link(withScaled) && grid[withScaled]->link(iScaled);
            }
            return result;
        } else if (down || up) {
            for (size_t n = 0; n < gridResolution; ++n) {
                size_t iScaled = ((i % width()) * gridResolution) + ((i / width()) * gridResolution) * _width + (down ? (n + (_width * (gridResolution - 1))) : (n));
                size_t withScaled = ((with % width()) * gridResolution) + ((with / width()) * gridResolution) * _width
                                    + (down ? (n) : (n + (_width * (gridResolution - 1))));
                result &= grid[iScaled]->link(withScaled) && grid[withScaled]->link(iScaled);
            }
            return result;
        }
    }
    addFillStep({i, with}, LINK, endOfStep);
    return grid[i]->link(with) && grid[with]->link(i);
}
bool Space::unlink(size_t i, size_t with, bool endOfStep) {
    if (tiling() == SQUARE && gridResolution != 1) {
        addFillStep({i, with}, UNLINK, endOfStep);
        bool result = true;
        bool right = grid[i]->offset(SquareNode::RIGHT) == with, left = grid[i]->offset(SquareNode::LEFT) == with, down = grid[i]->offset(SquareNode::DOWN) == with,
             up = grid[i]->offset(SquareNode::UP) == with;
        if (right || left) {
            for (size_t n = 0; n < gridResolution; ++n) {
                size_t iScaled = ((i % width()) * gridResolution) + ((i / width()) * gridResolution) * _width
                                 + (right ? (n * _width + gridResolution - 1) : (n * _width));
                size_t withScaled = ((with % width()) * gridResolution) + ((with / width()) * gridResolution) * _width
                                    + (right ? (n * _width) : (n * _width + gridResolution - 1));
                result &= grid[iScaled]->unlink(withScaled) && grid[withScaled]->unlink(iScaled);
            }
            return result;
        } else if (down || up) {
            for (size_t n = 0; n < gridResolution; ++n) {
                size_t iScaled = ((i % width()) * gridResolution) + ((i / width()) * gridResolution) * _width + (down ? (n + (_width * (gridResolution - 1))) : (n));
                size_t withScaled = ((with % width()) * gridResolution) + ((with / width()) * gridResolution) * _width
                                    + (down ? (n) : (n + (_width * (gridResolution - 1))));
                result &= grid[iScaled]->unlink(withScaled) && grid[withScaled]->unlink(iScaled);
            }
            return result;
        }
    }
    addFillStep({i, with}, UNLINK, endOfStep);
    return grid[i]->unlink(with) && grid[with]->unlink(i);
}
void Space::disintegrate(size_t i) {
    for (auto it = grid[i]->next.begin(); it != grid[i]->next.end(); it = grid[i]->next.begin()) {
        unlink(i, *it);
    }
    grid[i]->values.assign(grid[i]->values.size(), Node::defaultValue);
}
Space::Node& Space::operator[](size_t i) { return *grid[i]; }
Space::Node& Space::operator()(size_t x, size_t y) { return operator[](get1DCoordinates(Point2Du(x, y))); }
Space::Node& Space::operator()(Point2Du p) { return operator[](get1DCoordinates(p)); }
Point2Du Space::get2DCoordinates(size_t i) const {
    switch (_tiling) {
    case SQUARE:
    case HEXAGON: return Point2Du(i % width(), i / width());
    default: return NaN;
    }
}
size_t Space::get1DCoordinates(Point2Du p) const {
    switch (_tiling) {
    case SQUARE:
    case HEXAGON: return p.x + p.y * width();
    default: return NaN;
    }
}
optional<Space::Step> Space::getNextFillStep() {
    if (fillStepList.empty()) return {};
    Step t = fillStepList.front();
    if (isSonification && t.stepType >= MAX_VALUES) {
        lock_guard<std::mutex> guard(processedStepsMutex);
        processedSteps.push_back(t);
    }
    fillStepList.pop_front();
    return t;
}
std::optional<Space::Step> Space::getNextPathStep() {
    if (pathStepList.empty()) return {};
    Step t = pathStepList.front();
    if (isSonification && t.stepType >= MAX_VALUES) {
        lock_guard<std::mutex> guard(processedStepsMutex);
        processedSteps.push_back(t);
    }
    pathStepList.pop_front();
    return t;
}
int Space::sonificationTick(double* samples, unsigned nBufferFrames, unsigned sampleRate, std::function<double(size_t)> getFrequency) { // todo: refactor
    // if (processedSteps.empty())
    //     return;
    if (sonSets.maxOscillatorsN != sCalc.maxOscillatorsN) sCalc = SpaceSound(sonSets.maxOscillatorsN);
    int notEnoughOscillators = 0;

    {
        lock_guard<std::mutex> guard(processedStepsMutex);

        double scale = (double) nBufferFrames / processedSteps.size();
        int i = 0;
        auto it = processedSteps.begin();
        static size_t maxSinceLast0 = 0;
        if (processedSteps.size() == 0) {
            maxSinceLast0 = 0;
        } else {
            maxSinceLast0 = max(maxSinceLast0, processedSteps.size());
        }
        while (it != processedSteps.end() && (i <= maxSinceLast0 / 6 /*|| abs(int(processedSteps.size()) - i) < 3*/)) {
            double freq;
            switch (it->stepType) {
            case Space::LINK: freq = getFrequency(it->stepValue.x) * sonSets.freq_mult + sonSets.freq_add; break;
            case Space::UNLINK: freq = getFrequency(it->stepValue.y) * sonSets.freq_mult + sonSets.freq_add; break;
            case Space::SETCOLOR: freq = getFrequency(it->stepValue.x) * sonSets.freq_mult + sonSets.freq_add; break;
            default: freq = 0; break;
            }
            if (freq != 0) {
                sCalc.addOscillator(freq, sCalc.curTick + 1, sCalc.curTick + i * scale, sonSets.oscDuration);
                ++i;
            }
            it = processedSteps.erase(it);
        }

        notEnoughOscillators = clamp(int(sonSets.oscillatorsTarget) - int(processedSteps.size()), 0, (int) sonSets.oscillatorsTarget);
    }

    sCalc.sampleRate = sampleRate;
    sCalc.wave(samples, nBufferFrames, notEnoughOscillators);
    return 0;
}
std::list<size_t> Space::getNextPath() {
    std::list<size_t> t = paths.front();
    paths.pop_front();
    return t;
}
void Space::defaultAllValues() {
    for (auto& i : grid) {
        i->values.assign(MAX_VALUES, Node::defaultValue);
    }
}
void Space::prePathAlgInit() {
    pathStepList.clear();
    defaultAllValues();
    paths.clear();
}
void Space::clear() {
    if (tiling() == SQUARE && gridResolution != 1) {
        size_t gr = gridResolution;
        gridResolution = 1;
        for (size_t y = 0; y + gr - 1 < _height; y += gr) {
            for (size_t x = 0; x + gr - 1 < _width; x += gr) {
                size_t i = get1DCoordinates({x, y});
                for (size_t dy = 0; dy < gr; ++dy) {
                    for (size_t dx = 0; dx < gr; ++dx) {
                        size_t current = i + dx + dy * _width;
                        if (dx + 1 < gr) {
                            size_t right = current + 1;
                            link(current, right);
                        }
                        if (dy + 1 < gr) {
                            size_t down = current + _width;
                            link(current, down);
                        }
                    }
                }
            }
        }
        gridResolution = gr;
    } else {
        for (size_t i = 0; i < size(); ++i) {
            disintegrate(i);
        }
    }
}
void Space::clearFillColors(int color, bool removeAllJustOnce) {
    if (fillStepList.empty()) return;
    int endEvery = _width / 6 * (_width / 16.) + 1;
    int n = 0;
    bool prevWasColor = false;
    vector<bool> removed(removeAllJustOnce * size(), false);
    for (auto it = fillStepList.rbegin(); it != fillStepList.rend(); ++it) {
        if (it->stepType != SETCOLOR) continue;
        if (it->stepValue.y == color) {
            if (!removeAllJustOnce || !removed[it->stepValue.x]) {
                addFillStep({it->stepValue.x, 0}, SETCOLOR, ++n % endEvery == 0);
                if (removeAllJustOnce) removed[it->stepValue.x] = true;
            }
            prevWasColor = true;
        } else {
            if (prevWasColor) fillStepList.back().endOfStep = true;
            prevWasColor = false;
        }
    }
    for (size_t i = 0; i < removed.size(); ++i) {
        if (!removed[i]) addFillStep({i, 0}, SETCOLOR, ++n % endEvery == 0);
    }
}
void Space::floodFill() {
    for (auto& i : grid) {
        for (auto& j : i->getAvailableDirs()) {
            if (size_t next = i->offset(j); next != NaN) link(i->i, next);
        }
    }
}
void Space::recursiveBacktrackerMaze() {
    uniform_int_distribution<> ud(0, size() - 1);

    size_t begin = ud(dre), cur;

    deque<size_t> points({begin});
    addFillStep({points.front(), 1}, SETCOLOR);

    vector<bool> visited(size(), false);
    while (!points.empty()) {
        visited[cur = points.front()] = true;

        auto randDir = selectRandomDir(cur, [&](size_t i) { return !visited[i]; });
        if (randDir.dir != NaN) {
            points.push_front(randDir.next);
            link(cur, points.front(), false);
            addFillStep({points.front(), 1}, SETCOLOR);
        } else {
            addFillStep({points.front(), 2}, SETCOLOR);
            points.pop_front();
        }
    }

    clearFillColors();
}
void Space::EllersMaze() {
    uniform_real_distribution<> ud(0.0, 1.0);

    size_t n = 0;
    vector<size_t> curLine(width(), 0), nextLine(width(), 0);
    for (size_t i = 0; i < height(); ++i) {
        for (size_t j = 0; j < width(); ++j) {
            if (!curLine[j]) {
                curLine[j] = ++n;
                if (stepByStepFill) addFillStep({i * width() + j, curLine[j]}, StepType(0), false);
            }
        }
        for (size_t j = 0; j < width() - 1; ++j) {
            if ((i == height() - 1 || ud(dre) < EllersMazeVerticalProbability) && curLine[j] != curLine[j + 1]) {
                link(i * width() + j, i * width() + j + 1);
                for (size_t c = 0, t = curLine[j + 1]; c < width(); ++c) {
                    if (curLine[c] == t) {
                        if (stepByStepFill) addFillStep({i * width() + c, curLine[j]}, StepType(0), false);
                        curLine[c] = curLine[j];
                    }
                }
            }
        }

        auto calcWalls = [&](size_t v) {
            for (auto [c, n] : views::zip(curLine, nextLine)) {
                if (c == v && n == v) return true;
            }
            return false;
        };
        if (i != height() - 1) {
            for (size_t j = 0; j < width(); ++j) {
                if (ud(dre) < EllersMazeVerticalProbability || count(curLine.begin(), curLine.end(), curLine[j]) == 1) {
                    link(i * width() + j, (i + 1) * width() + j);
                    if (stepByStepFill) addFillStep({(i + 1) * width() + j, curLine[j]}, StepType(0), false);
                    nextLine[j] = curLine[j];
                }
            }
            for (size_t j = 0; j < width(); ++j) {
                if (calcWalls(curLine[j]) == 0) {
                    link(i * width() + j, (i + 1) * width() + j);
                    if (stepByStepFill) addFillStep({(i + 1) * width() + j, curLine[j]}, StepType(0), false);
                    nextLine[j] = curLine[j];
                }
            }
        }

        if (stepByStepFill) {
            for (size_t j = 0; j < width(); ++j) {
                addFillStep({i * width() + j, Node::defaultValue}, StepType(0), false);
            }
        }

        curLine = std::move(nextLine);
        nextLine.resize(width());
    }
}
void Space::KruskalsMaze() {
    vector<Point2Du> edges;
    for (auto& i : grid) {
        for (size_t j : i->getAvailableDirs()) {
            if (size_t c = i->offset(j); c != NaN) edges.push_back(Point2Du(i->i, c));
        }
    }
    shuffle(edges.begin(), edges.end(), dre);

    vector<size_t> lkd(size()), lng(size(), 1); // weighted quick-union with path compression by halving
    iota(lkd.begin(), lkd.end(), 0);
    for (auto& [p, q] : edges) {
        size_t i, j;
        for (i = p; i != lkd[i]; i = lkd[i])
            lkd[i] = lkd[lkd[i]];
        for (j = q; j != lkd[j]; j = lkd[j])
            lkd[j] = lkd[lkd[j]];

        if (i != j) {
            if (lkd[i] > lkd[j]) swap(i, j);
            lkd[i] = j;
            lng[j] += lng[i];
            link(p, q);
        }
    }
}
void Space::PrimsMaze() {
    set<size_t> pre;
    auto mark = [&](size_t index) {
        auto dirs = grid[index]->getAvailableDirs();
        shuffle(dirs.begin(), dirs.end(), dre);
        for (auto dir : dirs) {
            if (size_t t = grid[index]->offset(dir); t != NaN && grid[t]->empty()) {
                pre.insert(t);
                addFillStep({t, 1}, SETCOLOR, false);
            }
        }
    };

    size_t cur = uniform_int_distribution<>(0, size() - 1)(dre);
    mark(cur);
    addFillStep({cur, 2}, SETCOLOR);
    while (!pre.empty()) {
        auto it = pre.begin();
        uniform_int_distribution<> udpre(0, pre.size() - 1);
        size_t from = (advance(it, udpre(dre)), *it);
        pre.erase(it);

        auto randDir = selectRandomDir(from, [&](size_t i) { return (!grid[i]->empty() || i == cur); });
        if (randDir.next != NaN) {
            link(from, randDir.next, false);
            addFillStep({from, 2}, SETCOLOR);
        }

        mark(from);
        if (stepByStepFill) fillStepList.back().endOfStep = true;
    }

    clearFillColors();
}
/*void Space::recursiveDivisionMaze() {
    for (size_t i = 0; i < size(); ++i) {
        if (size_t t = grid[i]->offset(SquareNode::RIGHT); t != NaN) link(i, t, false);
        if (size_t t = grid[i]->offset(SquareNode::DOWN); t != NaN) link(i, t, false);
    }

    uniform_int_distribution<> ud(0, 1);
    function<void(Point2Du)> divide = [&](Point2Du area) {
        auto [x1, y1] = get2DCoordinates(area.x);
        auto [x2, y2] = get2DCoordinates(area.y);
        size_t _width = x2 - x1, _height = y2 - y1;
        if (!_width || !_height) return;

        size_t dir = _width < _height ? SquareNode::RIGHT : _height < _width ? SquareNode::DOWN : ud(dre);
        size_t wallFrom, wallTo, wallGap, wallIt;
        if (dir == SquareNode::RIGHT) { // horizontal wall
            wallFrom = area.x + uniform_int_distribution<>(0, _width - 1)(dre) * this->_width;
            wallTo = wallFrom + _width;
            wallGap = wallFrom + uniform_int_distribution<>(0, _width)(dre);
        } else { // vertical wall
            wallFrom = area.x + uniform_int_distribution<>(0, _height - 1)(dre);
            wallTo = wallFrom + _height * this->_width;
            wallGap = wallFrom + uniform_int_distribution<>(0, _height)(dre) * this->_width;
        }

        wallIt = wallFrom;
        while (wallIt != wallTo) {
            unlink(wallIt, grid[wallIt]->offset(!dir), false);
            wallIt = grid[wallIt]->offset(dir);
        }
        unlink(wallIt, grid[wallIt]->offset(!dir), true);
        link(wallGap, grid[wallGap]->offset(!dir));

        divide(Point2Du(area.x, wallTo));
        divide(Point2Du(grid[wallFrom]->offset(!dir), area.y));
    };
    divide({0, size() - 1});
}*/
void Space::recursiveDivisionMaze() {
    for (size_t i = 0; i < size(); ++i) {
        if (size_t t = grid[i]->offset(SquareNode::RIGHT); t != NaN) link(i, t, false);
        if (size_t t = grid[i]->offset(SquareNode::DOWN); t != NaN) link(i, t, false);
    }

    uniform_int_distribution<> ud(0, 1);
    function<void(Point2Du)> divide = [&](Point2Du area) {
        auto [x1, y1] = get2DCoordinates(area.x);
        auto [x2, y2] = get2DCoordinates(area.y);
        size_t r_width = x2 - x1, r_height = y2 - y1;
        if (!r_width || !r_height) return;

        size_t dir = r_width < r_height ? SquareNode::RIGHT : r_height < r_width ? SquareNode::DOWN : ud(dre);
        size_t wallFrom, wallTo, wallGap, wallIt;
        if (dir == SquareNode::RIGHT) { // horizontal wall
            wallFrom = area.x + uniform_int_distribution<>(0, r_width - 1)(dre) * width();
            wallTo = wallFrom + r_width;
            wallGap = wallFrom + uniform_int_distribution<>(0, r_width)(dre);
        } else { // vertical wall
            wallFrom = area.x + uniform_int_distribution<>(0, r_height - 1)(dre);
            wallTo = wallFrom + r_height * width();
            wallGap = wallFrom + uniform_int_distribution<>(0, r_height)(dre) * width();
        }

        wallIt = wallFrom;
        while (wallIt != wallTo) {
            unlink(wallIt, grid[wallIt]->offset(!dir), false);
            wallIt = grid[wallIt]->offset(dir);
        }
        unlink(wallIt, grid[wallIt]->offset(!dir), true);
        link(wallGap, grid[wallGap]->offset(!dir));

        divide(Point2Du(area.x, wallTo));
        divide(Point2Du(grid[wallFrom]->offset(!dir), area.y));
    };
    divide({0, size() - 1});
}
void Space::AldousBroderMaze() {
    size_t pos = uniform_int_distribution<>(0, size() - 1)(dre), n = 0;
    addFillStep({pos, 1}, SETCOLOR);
    while (n != size() - 1) {
        size_t t = pos;
        addFillStep({pos, 2}, SETCOLOR, false);
        auto randDir = selectRandomDir(pos);
        pos = randDir.next;
        if (grid[pos]->empty()) {
            link(t, pos, false);
            ++n;
        }
        addFillStep({pos, 1}, SETCOLOR);
    }

    clearFillColors(2, true);
}
void Space::WilsonsMaze() {
    uniform_int_distribution<> ud(0, size() - 1);

    size_t firstMazePart = ud(dre), begin, n = 0;
    addFillStep({firstMazePart, 2}, SETCOLOR);
    while (n != size() - 1) {
        do {
            begin = ud(dre);
        } while (!grid[begin]->empty() || begin == firstMazePart);

        size_t pos = begin;
        addFillStep({pos, 3}, SETCOLOR);
        selectRandomDirRT randDir;
        vector<uint8_t> vDir(size(), 0);
        while (grid[pos]->empty() && pos != firstMazePart) {
            randDir = selectRandomDir(pos);
            vDir[pos] = randDir.dir;
            addFillStep({pos, 1}, SETCOLOR, false);
            addFillStep({randDir.next, 3}, SETCOLOR);
            pos = randDir.next;
        }

        pos = begin;
        addFillStep({pos, 3}, SETCOLOR);
        while (pos != randDir.next) {
            link(pos, grid[pos]->offset(vDir[pos]), false);
            addFillStep({pos, 2}, SETCOLOR, false);
            addFillStep({grid[pos]->offset(vDir[pos]), 3}, SETCOLOR);
            pos = grid[pos]->offset(vDir[pos]);
            ++n;
        }

        if (stepByStepFill) {
            fillStepList.back().stepValue.y = 2;
            for (size_t i = 0; i < size(); ++i) {
                if (grid[i]->empty()) { addFillStep({i, 0}, SETCOLOR, false); }
            }
            fillStepList.back().endOfStep = true;
        }
    }

    clearFillColors();
}
void Space::huntAndKillMaze() {
    size_t pos = uniform_int_distribution<>(0, size() - 1)(dre), lowerBound = pos;
    vector<bool> visited(size(), false);
    visited[pos] = true;
    auto notVisited = [&](size_t i) { return !visited[i]; };
    selectRandomDirRT randDir = selectRandomDir(pos, notVisited);
    while (randDir.next != NaN) {
        do {
            link(pos, randDir.next);
            pos = randDir.next;
            visited[pos] = true;
            if (pos < lowerBound) lowerBound = pos;
            randDir = selectRandomDir(pos, notVisited);
        } while (randDir.next != NaN);

        for (size_t i = lowerBound; i < size(); ++i) {
            if (visited[i] && (randDir = selectRandomDir(i, notVisited)).next != NaN) {
                lowerBound = pos = i;
                break;
            }
        }
    }
}
size_t Space::GrowingTreeMazeSettings::newestMiddleOldestRandom(const std::deque<size_t>& p,
                                                                unsigned int pNewest,
                                                                unsigned int pMiddle,
                                                                unsigned int pOldest,
                                                                unsigned int pRandom)
{
    if (pNewest + pMiddle + pOldest + pRandom == 0)
        return 0; // fallback to front
    std::uniform_int_distribution<> ud(1, pNewest + pMiddle + pOldest + pRandom);
    int result = ud(Space::dre);
    if (result <= pNewest) {
        return 0;
    } else if (result > pNewest && result <= pNewest + pMiddle) {
        return p.size() / 2;
    } else if (result > pNewest + pMiddle && result <= pNewest + pMiddle + pOldest) {
        return p.size() - 1;
    } else {
        return std::uniform_int_distribution<>(0, p.size() - 1)(Space::dre);
    }
}
void Space::growingTreeMaze() {
    uniform_int_distribution<> ud(0, size() - 1);

    size_t begin = ud(dre), cur;

    deque<size_t> points({begin});
    addFillStep({points.front(), 1}, SETCOLOR);

    vector<bool> visited(size(), false);
    while (!points.empty()) {
        size_t index = growingTreeMazeSets.selectPointAlgorithm(points);
        cur = points[index];
        visited[cur] = true;

        auto randDir = selectRandomDir(cur, [&](size_t i) { return !visited[i]; });
        if (randDir.dir != NaN) {
            points.push_front(randDir.next);
            visited[randDir.next] = true;
            link(cur, points.front(), false);
            addFillStep({points.front(), 1}, SETCOLOR);
        } else {
            addFillStep({cur, 2}, SETCOLOR);
            points.erase(points.begin() + index);
        }
    }
}
void Space::merge(const std::vector<std::unique_ptr<Node>>& grid, const std::vector<std::vector<bool>>& mask) {
    std::vector<std::pair<size_t, size_t>> restore;
    for (size_t i = 0; i < mask.size(); ++i) {
        for (size_t j = 0; j < mask[i].size(); ++j) {
            size_t pos = get1DCoordinates(Point2Du(i, j));
            if (!mask[i][j]) {
                for (auto& nextPos : this->grid[pos]->next) {
                    if (mask[nextPos % _width][nextPos / _width]) { restore.push_back({pos, nextPos}); }
                }
            }
        }
    }
    for (size_t i = 0; i < mask.size(); ++i) {
        for (size_t j = 0; j < mask[i].size(); ++j) {
            if (!mask[i][j]) {
                size_t pos = get1DCoordinates(Point2Du(i, j));
                disintegrate(pos);
                this->grid[pos].reset(new SquareNode(*(SquareNode*) &*grid[pos]));
            }
        }
    }
    for (auto [i, with] : restore) {
        link(i, with);
    }
}
list<size_t> Space::BFSFind(size_t from, size_t to) {
    if (from == to) return {};

    vector<size_t> parent(size(), -1);
    deque<size_t> d({NaN, from});
    while (!d.empty()) {
        size_t back = d.back();
        d.pop_back();
        if (back == NaN) {
            if (!d.empty()) {
                d.push_front(size_t(NaN));
                if (!pathStepList.empty()) pathStepList.back().endOfStep = true;
            }
            continue;
        }

        for (auto i : grid[back]->next) {
            if (parent[i] == -1) {
                d.push_front(i);
                addPathStep({i, back}, SETPARENT, false);
                parent[i] = back;
                addPathStep({i, 1}, SETCOLOR, false);
                if (i == to) { return constructPath(parent, from, to); }
            }
        }
    }
    return {};
}
std::list<size_t> Space::AStarFind(size_t from, size_t to) {
    if (from == to) return {};

    enum Costs { G = 1, H = 2, F = 0 };
    vector<size_t> parent(size(), -1), open; // TODO: rewrite using priority queue (heap)
    open.reserve(size());
    open.push_back(from);
    setValue(from, G, 0, false);
    vector<bool> closed(size(), false);
    while (1) {
        // Looking for an element from the open deque whose cost F is minimal or with an equal cost, H is minimal
        size_t min = NaN;
        for (auto it = open.begin(); it != open.end(); ++it) {
            if (!closed[*it]
                && ((min == NaN)
                    || ((grid[*it]->values[F] < grid[min]->values[F]
                         || (grid[*it]->values[F] == grid[min]->values[F] && grid[*it]->values[H] < grid[min]->values[H]))))) {
                min = *it;
            }
        }

        if (min == NaN) break;
        addPathStep({min, 2}, SETCOLOR);
        closed[min] = true;
        for (auto& i : grid[min]->next) {
            if (closed[i]) continue;

            if (auto t = grid[min]->values[G] + calcWeightFunc(min, i); grid[i]->values[G] == Node::defaultValue || grid[i]->values[G] > t) {
                parent[i] = min;
                addPathStep({i, min}, SETPARENT, false);
                setValue(i, G, t, false); // G cost = distance from starting node
                if (grid[i]->values[H] == Node::defaultValue)
                    setValue(i, H, calcWeightFunc(to, i), false);               // H cost (heuristic) = Euclidean distance (by default) from end node
                setValue(i, F, grid[i]->values[G] + grid[i]->values[H], false); // F cost = G cost + H cost
                addPathStep({i, 1}, SETCOLOR, false);
                if (i == to) {
                    auto t = constructPath(parent, from, to);
                    addPath(t);
                    // visualizePath(t);
                    return t;
                }
                open.push_back(i);
            }
        }
        addPath(constructPath(parent, from, min)); // unsuccessful attempts lightning-like visualisation
    }
    return {};
}
void Space::DFS(size_t from) {
    vector<bool> discovered(size(), false);
    discovered[from] = true;
    vector<size_t> entry(size()), leave(size());
    entry[from] = 1;
    deque<size_t> d({from});
    unsigned time = 1;
    while (!d.empty()) {
        size_t front = d.front();
        for (auto i : grid[front]->next) {
            if (!discovered[i]) {
                d.push_front(i);
                entry[i] = ++time;
                discovered[i] = true;
                break;
            }
        }
        if (front == d.front()) {
            d.pop_front();
            leave[front] = ++time;
        }
    }
}
// private
void Space::addFillStep(Point2Du stepValue, StepType stepType, bool endOfStep) {
    if (stepByStepFill) fillStepList.push_back({stepValue, stepType, endOfStep});
}
void Space::addPathStep(Point2Du stepValue, StepType stepType, bool endOfStep) {
    if (stepByStepPath) pathStepList.push_back({stepValue, stepType, endOfStep});
}
// Node
Space::Node::Node(size_t i) : i(i) { values.resize(Space::MAX_VALUES, defaultValue); }
vector<size_t> Space::Node::getAvailableDirs() const {
    vector<size_t> result(space->_tiling);
    iota(result.begin(), result.end(), 0);
    return result;
}
bool Space::Node::empty() const { return next.empty(); }
bool Space::Node::linked(size_t with) const { return next.find(with) != next.end(); }
bool Space::Node::link(size_t with) { return next.insert(with).second; }
set<size_t>::size_type Space::Node::unlink(size_t with) { return next.erase(with); }
const size_t Space::Node::defaultValue = Space::NaN;
thread_local Space* Space::Node::space = nullptr;
// SquareNode
size_t Space::SquareNode::offset(size_t dir) const {
    // (→ ↓ ← ↑)
    static constexpr array wallArr = {&Space::SquareNode::wallR, &Space::SquareNode::wallD, &Space::SquareNode::wallL, &Space::SquareNode::wallU};
    static constexpr array offsetArr = {&Space::SquareNode::offsetR, &Space::SquareNode::offsetD, &Space::SquareNode::offsetL, &Space::SquareNode::offsetU};
    return !(this->*wallArr[dir])() ? (this->*offsetArr[dir])() : NaN;
}
size_t Space::SquareNode::offsetR() const { return i + 1; }
size_t Space::SquareNode::offsetD() const { return i + space->width(); }
size_t Space::SquareNode::offsetL() const { return i - 1; }
size_t Space::SquareNode::offsetU() const { return i - space->width(); }
bool Space::SquareNode::wallR() const { return i % space->width() == space->width() - 1; }
bool Space::SquareNode::wallD() const { return i > space->size() - space->width() - 1 || space->height() == 1; }
bool Space::SquareNode::wallL() const { return i % space->width() == 0; }
bool Space::SquareNode::wallU() const { return i < space->width() || space->height() == 1; }
// HexagonNode
size_t Space::HexagonNode::offset(size_t dir) const {
    // (→ ↘ ↙ ← ↖ ↗)
    static constexpr array wallArr = {&Space::HexagonNode::wallR, &Space::HexagonNode::wallDR, &Space::HexagonNode::wallDL,
                                      &Space::HexagonNode::wallL, &Space::HexagonNode::wallUL, &Space::HexagonNode::wallUR};
    static constexpr array offsetArr = {&Space::HexagonNode::offsetR, &Space::HexagonNode::offsetDR, &Space::HexagonNode::offsetDL,
                                        &Space::HexagonNode::offsetL, &Space::HexagonNode::offsetUL, &Space::HexagonNode::offsetUR};
    return !(this->*wallArr[dir])() ? (this->*offsetArr[dir])() : NaN;
}
size_t Space::HexagonNode::offsetR() const { return i + 1; }
size_t Space::HexagonNode::offsetDR() const { return i + space->width() + ((i / space->width()) & 1); }
size_t Space::HexagonNode::offsetDL() const { return i + space->width() - !((i / space->width()) & 1); }
size_t Space::HexagonNode::offsetL() const { return i - 1; }
size_t Space::HexagonNode::offsetUL() const { return i - space->width() - !((i / space->width()) & 1); }
size_t Space::HexagonNode::offsetUR() const { return i - space->width() + ((i / space->width()) & 1); }
bool Space::HexagonNode::wallR() const { return i % space->width() == space->width() - 1; }
bool Space::HexagonNode::wallDR() const { return (i > space->size() - space->width() - 1 || space->height() == 1) || (wallR() && i / space->width() % 2); }
bool Space::HexagonNode::wallDL() const { return (i > space->size() - space->width() - 1 || space->height() == 1) || (wallL() && !(i / space->width() % 2)); }
bool Space::HexagonNode::wallL() const { return i % space->width() == 0; }
bool Space::HexagonNode::wallUL() const { return (i < space->width() || space->height() == 1) || (wallL() && !(i / space->width() % 2)); }
bool Space::HexagonNode::wallUR() const { return (i < space->width() || space->height() == 1) || (wallR() && i / space->width() % 2); }
// AmorphousNode
Space::AmorphousNode::AmorphousNode(size_t i, map<size_t, size_t> neighbours) : Node(i) {
    values.resize(neighbours.size(), defaultValue);
    this->neighbours = std::move(neighbours);
}
vector<size_t> Space::AmorphousNode::getAvailableDirs() const {
    auto t = std::views::keys(neighbours);
    return vector<size_t>{t.begin(), t.end()};
}
size_t Space::AmorphousNode::offset(size_t dir) const {
    if (auto t = neighbours.find(dir); t != neighbours.end()) return t->second;
    return NaN;
}
// helper functions
bool Space::moveFrom(size_t& pos, size_t dir) {
    if (size_t offset = grid[pos]->offset(dir); offset != Space::NaN && grid[offset]->empty()) return link(pos, offset), pos = offset, true;
    return false;
}
Space::selectRandomDirRT Space::selectRandomDir(size_t i, function<bool(size_t)> condition) {
    vector<size_t> dirs = grid[i]->getAvailableDirs();
    shuffle(dirs.begin(), dirs.end(), dre);
    for (size_t j = 0; j < dirs.size(); ++j)
        if (size_t result = grid[i]->offset(dirs[j]); result != NaN && condition(result)) return {result, dirs[j]};
    return {NaN, NaN};
}
optional<Point2Df> Space::lineSegmentsIntersection(Point2Df AV0, Point2Df AV1, Point2Df BV0, Point2Df BV1) {
    float s1_x = AV1.x - AV0.x, s1_y = AV1.y - AV0.y, s2_x = BV1.x - BV0.x, s2_y = BV1.y - BV0.y,
          s = (-s1_y * (AV0.x - BV0.x) + s1_x * (AV0.y - BV0.y)) / (-s2_x * s1_y + s1_x * s2_y),
          t = (s2_x * (AV0.y - BV0.y) - s2_y * (AV0.x - BV0.x)) / (-s2_x * s1_y + s1_x * s2_y);
    return (s >= 0 && s <= 1 && t >= 0 && t <= 1) ? Point2Df(AV0.x + (t * s1_x), AV0.y + (t * s1_y)) : optional<Point2Df>{};
}
Point2Df Space::lineWindowIntersection(Point2Df AV0, Point2Df AV1, Point2Df windowSize) {
    optional<Point2Df> result;
    return (result = lineSegmentsIntersection(AV0, AV1, Point2Df(0, 0), Point2Df(windowSize.x, 0)))                         ? *result
           : (result = lineSegmentsIntersection(AV0, AV1, Point2Df(windowSize.x, 0), Point2Df(windowSize.x, windowSize.y))) ? *result
           : (result = lineSegmentsIntersection(AV0, AV1, Point2Df(windowSize.x, windowSize.y), Point2Df(0, windowSize.y))) ? *result
           : (result = lineSegmentsIntersection(AV0, AV1, Point2Df(0, windowSize.y), Point2Df(0, 0)))                       ? *result
                                                                                                                            : AV1;
}
/*void Space::visualizePath(const std::list<size_t>& path) {
    if(stepByStepPath && !path.empty()) {
        pathStepList.push_front({{path.back(), 3}, SETCOLOR, false});
        size_t prev = *(path.rbegin());
        for(auto it = ++path.rbegin(); it != path.rend(); ++it) {
            addPathStep({prev, 2}, SETCOLOR, false);
            addPathStep({*it, 3}, SETCOLOR);
            prev = *it;
        }
        pathStepList.push_front({{path.front(), 4}, SETCOLOR});
    }
}*/
void Space::setValue(size_t i, size_t vi, size_t v, bool endOfStep) {
    addPathStep({i, v}, StepType(vi), endOfStep);
    grid[i]->values[vi] = v;
}
void Space::addPath(std::list<size_t> path) {
    if (stepByStepPath) {
        paths.push_back(std::move(path));
        addPathStep({}, SETNEXTPATH, false);
    }
}
std::list<size_t> Space::constructPath(const std::vector<size_t>& parent, size_t from, size_t to) {
    list<size_t> result;
    while (from != to) {
        if (parent[to] == -1) return {};
        result.push_front(to);
        to = parent[to];
    }
    result.push_front(from);
    return result;
}
/* *** Legacy algorithms that were implemented when the project was not about mazes, but about space-filling curves.
void Space::horizontally() {
    size_t pos = 0;
    bool d = 1;
    do {
        while (moveFrom(pos, d ? HexagonNode::RIGHT : HexagonNode::LEFT)) {}
        d = !d;
    } while (!d ? moveFrom(pos, HexagonNode::DOWNRIGHT) : moveFrom(pos, HexagonNode::DOWNLEFT));
}
void Space::mirrorX() {
    vector<Node> temp = move(field);
    field.resize(temp.size());
    for(size_t i = 0; i < temp.size(); ++i) {
        size_t mirrored = mirrorX(i);
        field[mirrored].value = temp[i].value;
        for(auto& j : temp[i].next)
            field[mirrored].next.insert(mirrorX(j));
    }
}
void Space::mirrorY() {
    vector<Node> temp = move(field);
    field.resize(temp.size());
    for(size_t i = 0; i < temp.size(); ++i) {
        size_t mirrored = mirrorY(i);
        field[mirrored].value = temp[i].value;
        for(auto& j : temp[i].next)
            field[mirrored].next.insert(mirrorY(j));
    }
}
void Space::spiral() {
    size_t pos = 0, dirs[] = {RIGHT, DOWN, LEFT, UP};
    for(size_t i = 0; i<max(width, height) * 2 - 1; ++i) {
        while(moveFrom(pos, dirs[i % 4])) { }
    }
}
void Space::zigzag() {
    size_t pos = 0, dirs[] = {RIGHT, DOWNLEFT, DOWN, UPRIGHT};
    for(size_t i = 0; i<8 * max(width, height); ++i) {
        size_t a = 0.25f * (3.0f + i + (1.0f - i) * pow(-1.0f, i));
        while(a-- && moveFrom(pos, dirs[i % 4])) { }
    }
}
size_t Space::mirrorX(size_t i) {
    return width * (i / width) + (width - (i % width) - 1);
}
size_t Space::mirrorY(size_t i) {
    return (height - (i / width) - 1) * width + i % width;
}
*** TODO: Adapt to new tilings */

