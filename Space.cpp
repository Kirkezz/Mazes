#include "Space.h"
using namespace std;
// public
void Space::resize(size_t newWidth, size_t newHeight, TilingType newTiling) {
    grid.clear();
    _width = newWidth;
    _height = newHeight;
    grid.reserve(_width * _height);
    _tiling = newTiling;
    for(size_t i = 0; i < _width * _height; ++i) {
        switch(_tiling) {
        case TRIANGLE:
            /*grid.push_back(make_unique<TriangleNode>());*/ break;
        case SQUARE:
            grid.push_back(make_unique<SquareNode>(i));
            break;
        case HEXAGON:
            grid.push_back(make_unique<HexagonNode>(i));
            break;
        case AMORPHOUS:
            break;
        }
    }
}
typedef boost::polygon::segment_data<int> Segment;
void Space::resize(Point2Df windowSize, int smoothness) {
    auto isInsideWindow = [windowSize](const boost::polygon::voronoi_edge<double>::voronoi_vertex_type* v) {
        return (v && v->x() >= 0 && v->y() >= 0 && v->x() < windowSize.x && v->y() < windowSize.y);
    };
    VoronoiWindowSize = Point2Du(windowSize.x, windowSize.y);
    assert(!VoronoiPoints.empty());
    _width = NaN;
    _height = NaN;
    _tiling = AMORPHOUS;
    vector<Segment> segments;
    segments.push_back(Segment(VPoint(-windowSize.x * 3, -windowSize.y * 3), VPoint(windowSize.x * 3, -windowSize.y * 3)));
    segments.push_back(Segment(VPoint(windowSize.x * 3, -windowSize.y * 3), VPoint(windowSize.x * 3, windowSize.y * 3)));
    segments.push_back(Segment(VPoint(windowSize.x * 3, windowSize.y * 3), VPoint(-windowSize.x * 3, windowSize.y * 3)));
    segments.push_back(Segment(VPoint(-windowSize.x * 3, windowSize.y * 3), VPoint(-windowSize.x * 3, -windowSize.y * 3)));
    vd.clear();
    for(size_t i = 0; i < smoothness; ++i) { // Lloyd's algorithm
        sort(VoronoiPoints.begin(), VoronoiPoints.end());
        boost::polygon::construct_voronoi(VoronoiPoints.begin(), VoronoiPoints.end(), segments.begin(), segments.end(), &vd);
        vector<VPoint> nextPoints;
        nextPoints.reserve(VoronoiPoints.size());
        for(auto& cell : vd.cells()) {
            if(!cell.contains_point())
                continue;
            auto edge = cell.incident_edge();
            set<Point2Df> verticles;
            do {
                if(edge->is_primary()) {
                    auto v0 = edge->vertex0(), v1 = edge->vertex1();
                    bool insideV0 = isInsideWindow(v0), insideV1 = isInsideWindow(v1);
                    if(insideV0)
                        verticles.insert(Point2Df(v0->x(), v0->y()));
                    if(insideV1)
                        verticles.insert(Point2Df(v1->x(), v1->y()));
                    if(Point2Df t = lineWindowIntersection(Point2Df(v0->x(), v0->y()), Point2Df(v1->x(), v1->y()), windowSize); t && (insideV0 ^ insideV1))
                        verticles.insert(t);
                }
                edge = edge->next();
            } while(edge != cell.incident_edge());
            if(!verticles.empty()) {
                Point2Df result(0, 0);
                for(auto& i : verticles)
                    result += i;
                result /= verticles.size();
                nextPoints.push_back(VPoint(result.x, result.y));
            }
        }
        VoronoiPoints = std::move(nextPoints);
        vd.clear();
    }
    grid.clear();
    grid.reserve(VoronoiPoints.size());
    // "All the site objects are constructed and sorted at the algorithm
    // initialization step". I sort it before, because I want to get point by
    // cell->source_index
    sort(VoronoiPoints.begin(), VoronoiPoints.end());
    boost::polygon::construct_voronoi(VoronoiPoints.begin(), VoronoiPoints.end(), segments.begin(), segments.end(), &vd);
    for(auto& cell : vd.cells()) {
        if(!cell.contains_point())
            continue;
        map<size_t, size_t> neighbours;
        auto edge = cell.incident_edge();
        int n = 0;
        do {
            const auto& twin = edge->twin();
            if(twin->is_primary() && twin->cell()->contains_point() && (isInsideWindow(twin->vertex0()) || isInsideWindow(twin->vertex1())) &&
               Point2Df(twin->vertex0()->x(), twin->vertex0()->y()).distance(Point2Df(twin->vertex1()->x(), twin->vertex1()->y())) >= minDistForEdgeAdjacency)
                neighbours.insert({n, twin->cell()->source_index()});
            edge = edge->next();
            ++n;
        } while(edge != cell.incident_edge());
        if(!neighbours.empty() && n > 2)
            grid.push_back(make_unique<AmorphousNode>(grid.size(), neighbours));
    }
}
Space::Space(size_t width, size_t height, TilingType tiling) {
    Node::space = this;
    resize(width, height, tiling);
    dre.seed(rd());
}
size_t Space::size() const { return grid.size(); }
Space::TilingType Space::tiling() const { return _tiling; }
size_t Space::width() const { return _width; }
size_t Space::height() const { return _height; }
bool Space::link(size_t i, size_t with, bool endOfStep) {
    addFillStep({i, with}, LINK, endOfStep);
    return grid[i]->link(with) && grid[with]->link(i);
}
bool Space::unlink(size_t i, size_t with, bool endOfStep) {
    addFillStep({i, with}, UNLINK, endOfStep);
    return grid[i]->unlink(with) && grid[with]->unlink(i);
}
void Space::disintegrate(size_t i) {
    for(auto it = grid[i]->next.begin(); it != grid[i]->next.end(); it = grid[i]->next.begin()) {
        unlink(i, *it);
    }
    grid[i]->values.assign(grid[i]->values.size(), Node::defaultValue);
}
Space::Node& Space::operator[](size_t i) { return *grid[i]; }
Space::Node& Space::operator()(size_t x, size_t y) { return operator[](get1DCoordinates(Point2Du(x, y))); }
Space::Node& Space::operator()(Point2Du p) { return operator[](get1DCoordinates(p)); }
Point2Du Space::get2DCoordinates(size_t i) const {
    switch(_tiling) {
    case SQUARE:
    case HEXAGON:
        return Point2Du(i % _width, i / _width);
    default:
        return NaN;
    }
}
size_t Space::get1DCoordinates(Point2Du p) const {
    switch(_tiling) {
    case SQUARE:
    case HEXAGON:
        return p.x + p.y * _width;
    default:
        return NaN;
    }
}
optional<Space::Step> Space::getNextFillStep() {
    if(!fillStepList.empty()) {
        Step t = fillStepList.front();
        fillStepList.pop_front();
        return t;
    }
    return {};
}
std::optional<Space::Step> Space::getNextPathStep() {
    if(!pathStepList.empty()) {
        Step t = pathStepList.front();
        pathStepList.pop_front();
        return t;
    }
    return {};
}
std::list<size_t> Space::getNextPath() {
    std::list<size_t> t = paths.front();
    paths.pop_front();
    return t;
}
void Space::defaultAllValues() {
    for(auto& i : grid) {
        i->values.assign(MAX_VALUES, Node::defaultValue);
    }
}
void Space::prePathAlgInit() {
    pathStepList.clear();
    defaultAllValues();
    paths.clear();
}
void Space::clear() {
    for(size_t i = 0; i < size(); ++i)
        disintegrate(i);
}
void Space::floodFill() {
    for(auto& i : grid) {
        for(auto& j : i->getAvailableDirs()) {
            if(size_t next = i->offset(j); next != NaN) {
                link(i->i, next);
            }
        }
    }
}
void Space::horizontally() {
    size_t pos = 0;
    bool d = 1;
    do {
        while(moveFrom(pos, d ? HexagonNode::RIGHT : HexagonNode::LEFT)) {
        }
        d = !d;
    } while(!d ? moveFrom(pos, HexagonNode::DOWNRIGHT) : moveFrom(pos, HexagonNode::DOWNLEFT));
}

void Space::recursiveBacktrackerMaze() {
    uniform_int_distribution<> ud(0, size() - 1);
    size_t begin = 0, cur;
    deque<size_t> points({begin});
    vector<bool> visited(size(), false);
    addFillStep({points.front(), 1}, SETCOLOR);
    while(!points.empty()) {
        visited[cur = points.front()] = true;
        auto randDir = selectRandomDir(cur, [&](size_t i) { return !visited[i]; });
        if(randDir.dir != NaN) {
            points.push_front(randDir.next);
            link(cur, points.front(), false);
            addFillStep({points.front(), 1}, SETCOLOR);
        } else {
            addFillStep({points.front(), 2}, SETCOLOR);
            points.pop_front();
        }
    }
}
void Space::EllersMaze() {
    uniform_real_distribution<> ud(0.0, 1.0);
    vector<size_t> thisLine(_width, 0), nextLine(_width, 0);
    size_t n = 0;
    for(size_t i = 0; i < _height; ++i) {
        for(size_t j = 0; j < _width; ++j)
            if(!thisLine[j])
                thisLine[j] = ++n;
        for(size_t j = 0; j < _width - 1; ++j) {
            if((i == _height - 1 || ud(dre) < EllersMazeVerticalProbability) && thisLine[j] != thisLine[j + 1]) {
                link(i * _width + j, i * _width + j + 1);
                for(size_t c = 0, t = thisLine[j + 1]; c < _width; ++c)
                    if(thisLine[c] == t)
                        thisLine[c] = thisLine[j];
            }
        }
        auto calcWalls = [&](size_t v) {
            for(size_t c = 0; c < _width; ++c)
                if(thisLine[c] == v && thisLine[c] == nextLine[c])
                    return true;
            return false;
        };
        if(i != _height - 1) {
            for(size_t j = 0; j < _width; ++j) {
                if(ud(dre) < EllersMazeVerticalProbability || count(thisLine.begin(), thisLine.end(), thisLine[j]) == 1) {
                    link(i * _width + j, (i + 1) * _width + j);
                    nextLine[j] = thisLine[j];
                }
            }
            for(size_t j = 0; j < _width; ++j) {
                if(calcWalls(thisLine[j]) == 0) {
                    link(i * _width + j, (i + 1) * _width + j);
                    nextLine[j] = thisLine[j];
                }
            }
        }
        thisLine = std::move(nextLine);
        nextLine.resize(_width);
    }
}
void Space::KruskalsMaze() {
    vector<Point2Du> edges;
    for(auto& i : grid)
        for(size_t j : i->getAvailableDirs())
            if(size_t c = i->offset(j); c != NaN)
                edges.push_back(Point2Du(i->i, c));
    shuffle(edges.begin(), edges.end(), dre);
    vector<size_t> lkd(size()), lng(size(), 1); // weighted quick-union with path compression by halving
    iota(lkd.begin(), lkd.end(), 0);
    for(auto& [p, q] : edges) {
        size_t i, j;
        for(i = p; i != lkd[i]; i = lkd[i])
            lkd[i] = lkd[lkd[i]];
        for(j = q; j != lkd[j]; j = lkd[j])
            lkd[j] = lkd[lkd[j]];
        if(i != j) {
            if(lkd[i] > lkd[j])
                swap(i, j);
            lkd[i] = j;
            lng[j] += lng[i];
            link(p, q);
        }
    }
}
void Space::PrimsMaze() {
    uniform_int_distribution<> ud(0, size() - 1);
    size_t cur = ud(dre);
    set<size_t> pre;
    auto mark = [&](size_t index) {
        auto dirs = grid[index]->getAvailableDirs();
        shuffle(dirs.begin(), dirs.end(), dre);
        for(auto dir : dirs) {
            if(size_t t = grid[index]->offset(dir); t != NaN && grid[t]->empty()) {
                pre.insert(t);
                addFillStep({t, 1}, SETCOLOR, false);
            }
        }
    };
    mark(cur);
    addFillStep({cur, 2}, SETCOLOR);
    while(!pre.empty()) {
        uniform_int_distribution<> udpre(0, pre.size() - 1);
        auto it = pre.begin();
        size_t from = (advance(it, udpre(dre)), *it);
        pre.erase(it);
        auto randDir = selectRandomDir(from, [&](size_t i) { return (!grid[i]->empty() || i == cur); });
        if(randDir.next != NaN) {
            link(from, randDir.next, false);
            addFillStep({from, 2}, SETCOLOR);
        }
        mark(from);
        if(stepByStepFill)
            fillStepList.back().endOfStep = true;
    }
}
void Space::recursiveDivisionMaze() {
    for(size_t i = 0; i < size(); ++i) {
        if(size_t t = grid[i]->offset(SquareNode::RIGHT); t != NaN)
            link(i, t, false);
        if(size_t t = grid[i]->offset(SquareNode::DOWN); t != NaN)
            link(i, t, false);
    }
    uniform_int_distribution<> ud(0, 1);
    function<void(Point2Du)> divide = [&](Point2Du area) {
        auto [x1, y1] = get2DCoordinates(area.x);
        auto [x2, y2] = get2DCoordinates(area.y);
        size_t _width = x2 - x1, _height = y2 - y1;
        if(!_width || !_height)
            return;
        size_t dir = _width < _height ? SquareNode::RIGHT : _height < _width ? SquareNode::DOWN : ud(dre);
        size_t wallFrom, wallTo, wallGap, wallIt;
        if(dir == SquareNode::RIGHT) { // horizontal wall
            wallFrom = area.x + uniform_int_distribution<>(0, _width - 1)(dre) * this->_width;
            wallTo = wallFrom + _width;
            wallGap = wallFrom + uniform_int_distribution<>(0, _width)(dre);
        } else { // vertical wall
            wallFrom = area.x + uniform_int_distribution<>(0, _height - 1)(dre);
            wallTo = wallFrom + _height * this->_width;
            wallGap = wallFrom + uniform_int_distribution<>(0, _height)(dre) * this->_width;
        }
        wallIt = wallFrom;
        while(wallIt != wallTo) {
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
    while(n != size() - 1) {
        size_t t = pos;
        addFillStep({pos, 2}, SETCOLOR, false);
        auto randDir = selectRandomDir(pos);
        pos = randDir.next;
        if(grid[pos]->empty()) {
            link(t, pos, false);
            ++n;
        }
        addFillStep({pos, 1}, SETCOLOR);
    }
}
void Space::WilsonsMaze() {
    uniform_int_distribution<> ud(0, size() - 1);
    size_t firstMazePart = ud(dre), begin, n = 0;
    addFillStep({firstMazePart, 2}, SETCOLOR);
    while(n != size() - 1) {
        vector<uint8_t> vDir(size(), 0);
        do {
            begin = ud(dre);
        } while(!grid[begin]->empty() || begin == firstMazePart);
        size_t pos = begin;
        selectRandomDirRT randDir;
        addFillStep({pos, 3}, SETCOLOR);
        while(grid[pos]->empty() && pos != firstMazePart) {
            randDir = selectRandomDir(pos);
            vDir[pos] = randDir.dir;
            addFillStep({pos, 1}, SETCOLOR, false);
            addFillStep({randDir.next, 3}, SETCOLOR);
            pos = randDir.next;
        }
        pos = begin;
        addFillStep({pos, 3}, SETCOLOR);
        while(pos != randDir.next) {
            link(pos, grid[pos]->offset(vDir[pos]), false);
            addFillStep({pos, 2}, SETCOLOR, false);
            addFillStep({grid[pos]->offset(vDir[pos]), 3}, SETCOLOR);
            pos = grid[pos]->offset(vDir[pos]);
            ++n;
        }
        if(stepByStepFill) {
            fillStepList.back().stepValue.y = 2;
            for(size_t i = 0; i < size(); ++i) {
                if(grid[i]->empty()) {
                    addFillStep({i, 0}, SETCOLOR, false);
                }
            }
            fillStepList.back().endOfStep = true;
        }
    }
}
void Space::huntAndKillMaze() {
    size_t pos = uniform_int_distribution<>(0, size() - 1)(dre), lowerBound = pos;
    vector<bool> visited(size(), false);
    visited[pos] = true;
    auto notVisited = [&](size_t i) { return !visited[i]; };
    selectRandomDirRT randDir = selectRandomDir(pos, notVisited);
    while(randDir.next != NaN) {
        do {
            link(pos, randDir.next);
            pos = randDir.next;
            visited[pos] = true;
            if(pos < lowerBound)
                lowerBound = pos;
            randDir = selectRandomDir(pos, notVisited);
        } while(randDir.next != NaN);
        for(size_t i = lowerBound; i < size(); ++i) {
            if(visited[i] && (randDir = selectRandomDir(i, notVisited)).next != NaN) {
                lowerBound = pos = i;
                break;
            }
        }
    }
}
list<size_t> Space::BFSFind(size_t from, size_t to) {
    if(from == to)
        return {};
    vector<bool> discovered(size(), false);
    discovered[from] = true;
    vector<size_t> parent(size(), -1);
    deque<size_t> d({NaN, from});
    while(!d.empty()) {
        size_t back = d.back();
        d.pop_back();
        if(back == NaN) {
            if(!d.empty()) {
                d.push_front(size_t(NaN));
                pathStepList.back().endOfStep = true;
            }
            continue;
        }
        for(auto i : grid[back]->next) {
            if(!discovered[i]) {
                d.push_front(i);
                parent[i] = back;
                addPathStep({i, 1}, SETCOLOR, false);
                discovered[i] = true;
                if(i == to) {
                    list<size_t> result;
                    while(from != to) {
                        result.push_front(to);
                        if(parent[to] == -1)
                            return {};
                        to = parent[to];
                    }
                    result.push_front(from);
                    // visualizePath(result);
                    return result;
                }
            }
        }
    }
    return {};
}
std::list<size_t> Space::AStarFind(size_t from, size_t to) {
    if(from == to)
        return {};
    enum Costs { G = 1, H = 2, F = 0 };
    size_t current;
    vector<size_t> parent(size(), -1), open;
    open.reserve(size());
    open.push_back(from);
    setValue(from, G, 0, false);
    vector<bool> closed(size(), false);
    while(1) {
        // Looking for an element from the open deque whose cost F is minimal or with an equal cost, H is minimal
        size_t min = NaN;
        for(auto it = open.begin(); it != open.end(); ++it) {
            if(!closed[*it] && ((min == NaN) || ((grid[*it]->values[F] < grid[min]->values[F] ||
                                                  (grid[*it]->values[F] == grid[min]->values[F] && grid[*it]->values[H] < grid[min]->values[H]))))) {
                min = *it;
            }
        }
        if(min == NaN)
            break;
        addPathStep({min, 2}, SETCOLOR);
        closed[min] = true;
        for(auto& i : grid[min]->next) { // fix: G cost wrong calculated (first calc)
            if(closed[i])
                continue;
            if(auto t = grid[min]->values[G] + calcWeightFunc(min, i); grid[i]->values[G] == Node::defaultValue || grid[i]->values[G] > t) {
                parent[i] = min;
                setValue(i, G, t, false);                                       // G cost = distance from starting node
                setValue(i, H, calcWeightFunc(to, i), false);                   // H cost (heuristic) = Euclidean distance (by default) from end node
                setValue(i, F, grid[i]->values[G] + grid[i]->values[H], false); // F cost = G cost + H cost
                addPathStep({i, 1}, SETCOLOR, false);
                if(i == to) {
                    auto t = constructPath(parent, from, to);
                    addPath(t);
                    // visualizePath(t);
                    return t;
                }
                open.push_back(i);
            }
        }
        addPath(constructPath(parent, from, min));
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
    while(!d.empty()) {
        size_t front = d.front();
        for(auto i : grid[front]->next) {
            if(!discovered[i]) {
                d.push_front(i);
                entry[i] = ++time;
                discovered[i] = true;
                break;
            }
        }
        if(front == d.front()) {
            d.pop_front();
            leave[front] = ++time;
        }
    }
}
// private
void Space::addFillStep(Point2Du stepValue, StepType stepType, bool endOfStep) {
    if(stepByStepFill)
        fillStepList.push_back({stepValue, stepType, endOfStep});
}
void Space::addPathStep(Point2Du stepValue, StepType stepType, bool endOfStep) {
    if(stepByStepPath)
        pathStepList.push_back({stepValue, stepType, endOfStep});
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
Space* Space::Node::space = nullptr;
// SquareNode
size_t Space::SquareNode::offset(size_t dir) const {
    // (→ ↓ ← ↑)
    static constexpr array wallArr = {&Space::SquareNode::wallR, &Space::SquareNode::wallD, &Space::SquareNode::wallL, &Space::SquareNode::wallU};
    static constexpr array offsetArr = {&Space::SquareNode::offsetR, &Space::SquareNode::offsetD, &Space::SquareNode::offsetL, &Space::SquareNode::offsetU};
    return !(this->*wallArr[dir])() ? (this->*offsetArr[dir])() : NaN;
}
size_t Space::SquareNode::offsetR() const { return i + 1; }
size_t Space::SquareNode::offsetD() const { return i + space->_width; }
size_t Space::SquareNode::offsetL() const { return i - 1; }
size_t Space::SquareNode::offsetU() const { return i - space->_width; }
bool Space::SquareNode::wallR() const { return i % space->_width == space->_width - 1; }
bool Space::SquareNode::wallD() const { return i > space->size() - space->_width - 1 || space->_height == 1; }
bool Space::SquareNode::wallL() const { return i % space->_width == 0; }
bool Space::SquareNode::wallU() const { return i < space->_width || space->_height == 1; }
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
size_t Space::HexagonNode::offsetDR() const { return i + space->_width + ((i / space->_width) & 1); }
size_t Space::HexagonNode::offsetDL() const { return i + space->_width - !((i / space->_width) & 1); }
size_t Space::HexagonNode::offsetL() const { return i - 1; }
size_t Space::HexagonNode::offsetUL() const { return i - space->_width - !((i / space->_width) & 1); }
size_t Space::HexagonNode::offsetUR() const { return i - space->_width + ((i / space->_width) & 1); }
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
    if(auto t = neighbours.find(dir); t != neighbours.end())
        return t->second;
    return NaN;
}
// helper functions
bool Space::moveFrom(size_t& pos, size_t dir) {
    if(size_t offset = grid[pos]->offset(dir); offset != Space::NaN && grid[offset]->empty())
        return link(pos, offset), pos = offset, true;
    return false;
}
Space::selectRandomDirRT Space::selectRandomDir(size_t i, function<bool(size_t)> condition) {
    vector<size_t> dirs = grid[i]->getAvailableDirs();
    shuffle(dirs.begin(), dirs.end(), dre);
    for(size_t j = 0; j < dirs.size(); ++j)
        if(size_t result = grid[i]->offset(dirs[j]); result != NaN && condition(result))
            return {result, dirs[j]};
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
    if(stepByStepPath) {
        paths.push_back(std::move(path));
        addPathStep({}, SETNEXTPATH, false);
    }
}
std::list<size_t> Space::constructPath(const std::vector<size_t>& parent, size_t from, size_t to) {
    list<size_t> result;
    while(from != to) {
        result.push_front(to);
        if(parent[to] == -1)
            return {};
        to = parent[to];
    }
    result.push_front(from);
    return result;
}
