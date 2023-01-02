#include "Space.h"
#include <algorithm>
#include <ranges>
#include <deque>
#include <map>
using namespace std;
Space::Space(size_t width, size_t height) : width(width), height(height) {
    field.resize(width * height);
    dre.seed(rd());
}
size_t Space::size() { return field.size(); }
void Space::resize(size_t new_width, size_t new_height) {
    field.clear();
    field.resize(new_width * new_height);
    width = new_width;
    height = new_height;
    stepList.clear();
}
bool Space::isValid(size_t i) {
    return i < field.size();
}
bool Space::isValid(Point2Du p) {
    return p.x < width && p.y < height;
}
Node& Space::operator[](size_t i) { return field[i]; }
Node Space::operator()(size_t i) { return field[i]; }
Node Space::operator()(size_t x, size_t y) { return field[x + y * width]; }
Node Space::operator()(Point2Du p) { return field[p.x + p.y * width]; }
std::vector<Node>& Space::getField() { return field; }
Point2Du Space::get2DCoordinates(size_t i) { return Point2Du(i % width, i / width); }
size_t Space::get1DCoordinates(Point2Du p) { return p.x + p.y * width; }
bool Space::link(size_t i, size_t with, bool endOfStep) {
    addStep({i, with}, LINK, endOfStep);
    return (field[i].next.insert(with).second && field[with].next.insert(i).second);
}
bool Space::unlink(size_t i, size_t with, bool endOfStep) {
    addStep({i, with}, UNLINK, endOfStep);
    return (field[i].next.erase(with) && field[with].next.erase(i));
}
void Space::disintegrate(size_t i) {
    for(auto it = field[i].next.begin(); it != field[i].next.end(); it = field[i].next.begin()) {
        unlink(i, *it);
    }
    field[i].value = 0;
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
void Space::clear() {
    field.clear();
    field.resize(width * height);
    stepList.clear();
}
void Space::horizontally() {
    size_t pos = 0;
    bool d = 0;
    do {
        while(moveFrom(pos, d ? LEFT : RIGHT)) { }
        d = !d;
    } while(moveFrom(pos, DOWN));
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
Space::selectRandomDirRT Space::selectRandomDir(size_t i, function<bool(size_t)> condition, bool shuffle) {
    static array dirs = {RIGHT, DOWN, LEFT, UP};
    if(shuffle) std::shuffle(dirs.begin(), dirs.end(), dre);
    for(size_t j = 0, result; j < dirs.size(); ++j) {
        if(!wall(i, dirs[j]) && condition(result = offset(i, dirs[j]))) {
            return { result, dirs[j] };
        }
    }
    return { NaN, NaN };
}
void Space::recursiveBacktrackerMaze() {
    uniform_int_distribution<> ud(0, size() - 1);
    size_t begin = ud(dre), cur;
    deque<size_t> points({begin});
    vector<bool> visited(size(), false);
    addStep({points.front(), 1}, SETVALUE);
    while(!points.empty()) {
        visited[cur = points.front()] = true;
        auto randDir = selectRandomDir(cur, [&](size_t i){return !visited[i];});
        if(randDir.dir != NaN) {
            points.push_front(randDir.next);
            link(cur, points.front());
            addStep({points.front(), 1}, SETVALUE);
        } else {
            addStep({points.front(), 2}, SETVALUE);
            points.pop_front();
        }
    }
}
void Space::EllersMaze() {
    uniform_real_distribution<> ud(0.0, 1.0);
    vector<size_t> thisLine(width, 0), nextLine(width, 0);
    size_t n = 0;
    for(size_t i = 0; i < height; ++i) {
        for(size_t j = 0; j < width; ++j)
            if(!thisLine[j])
                thisLine[j] = ++n;
        for(size_t j = 0; j < width - 1; ++j) {
            if((i == height - 1 || ud(dre) < EllersMazeVerticalProbability) && thisLine[j] != thisLine[j + 1]) {
                link(i * width + j, i * width + j + 1);
                for(size_t c = 0, t = thisLine[j + 1]; c < width; ++c)
                    if(thisLine[c] == t)
                        thisLine[c] = thisLine[j];
            }
        }
        auto calcWalls = [&](size_t v) { for(size_t c = 0; c < width; ++c) if(thisLine[c] == v && thisLine[c] == nextLine[c]) return true; return false; };
        if(i != height - 1) {
            for(size_t j = 0; j < width; ++j) {
                if(ud(dre) < EllersMazeVerticalProbability || count(thisLine.begin(), thisLine.end(), thisLine[j]) == 1) {
                    link(i * width + j, (i + 1) * width + j);
                    nextLine[j] = thisLine[j];
                }
            }
            for(size_t j = 0; j < width; ++j) {
                if(calcWalls(thisLine[j]) == 0) {
                    link(i * width + j, (i + 1) * width + j);
                    nextLine[j] = thisLine[j];
                }
            }
        }
        thisLine = move(nextLine);
        nextLine.resize(width);
    }
}
void Space::KruskalsMaze() {
    vector<Point2Du> edges;
    edges.reserve(2 * width * height - width - height);
    for(size_t i = 0; i < size(); ++i) {
        if(!wall(i, RIGHT))
            edges.push_back({i, offset(i, RIGHT)});
        if(!wall(i, DOWN))
            edges.push_back({i, offset(i, DOWN)});
    }
    shuffle(edges.begin(), edges.end(), dre);
    vector<size_t> lkd(size()), lng(size(), 1); // "weighted quick-union with path compression by halving"
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
            lkd[i] = j; lng[j] += lng[i];
            link(p, q);
        }
    }
}
void Space::PrimsMaze() {
    uniform_int_distribution<> ud(0, size() - 1);
    size_t cur = ud(dre);
    set<size_t> pre;
    auto mark = [&](size_t index) {
        static array dirs = {RIGHT, DOWN, LEFT, UP};
        shuffle(dirs.begin(), dirs.end(), dre);
        for(auto dir : dirs) {
            size_t t = offset(index, dir);
            if(!wall(index, dir) && field[t].empty()) {
                pre.insert(t);
                addStep({t, 1}, SETVALUE, false);
            }
        }
    };
    mark(cur);
    addStep({cur, 2}, SETVALUE);
    while(!pre.empty()) {
        uniform_int_distribution<> udpre(0, pre.size() - 1);
        auto it = pre.begin();
        size_t from = (advance(it, udpre(dre)), *it);
        pre.erase(it);
        auto randDir = selectRandomDir(from, [&](size_t i){return (!field[i].empty() || i == cur);});
        if(randDir.next != NaN) {
            link(from, randDir.next, false);
            addStep({from, 2}, SETVALUE);
        }
        mark(from);
        if(stepByStepFilling) stepList.back().endOfStep = true;
    }
}
void Space::recursiveDivisionMaze() {
    for(size_t i = 0; i < size(); ++i) {
        if(!wall(i, RIGHT))
            link(i, offset(i, RIGHT), false);
        if(!wall(i, DOWN))
            link(i, offset(i, DOWN), false);
    }
    uniform_int_distribution<> ud(0, 1);
    function<void(Point2Du)> divide = [&](Point2Du area) {
        auto [x1, y1] = get2DCoordinates(area.x);
        auto [x2, y2] = get2DCoordinates(area.y);
        size_t width = x2 - x1, height = y2 - y1;
        if(!width || !height)
            return;
        size_t dir = width < height ? RIGHT : height < width ? DOWN : ud(dre) * 3;
        size_t wallFrom, wallTo, wallGap, wallIt;
        if(dir == RIGHT) { // horizontal wall
            wallFrom = area.x + uniform_int_distribution<>(0, width - 1)(dre) * this->width;
            wallTo = wallFrom + width;
            wallGap = wallFrom + uniform_int_distribution<>(0, width)(dre);
        } else { // vertical wall
            wallFrom = area.x + uniform_int_distribution<>(0, height - 1)(dre);
            wallTo = wallFrom + height * this->width;
            wallGap = wallFrom + uniform_int_distribution<>(0, height)(dre) * this->width;
        }
        wallIt = wallFrom;
        while(wallIt != wallTo) {
            unlink(wallIt, offset(wallIt, !dir * 3), false);
            wallIt = offset(wallIt, dir);
        }
        unlink(wallIt, offset(wallIt, !dir * 3), true);
        link(wallGap, offset(wallGap, !dir * 3));
        divide(Point2Du(area.x, wallTo));
        divide(Point2Du(offset(wallFrom, !dir * 3), area.y));
    };
    divide({0, size() - 1});
}
void Space::AldousBroderMaze() {
    size_t pos = uniform_int_distribution<>(0, size() - 1)(dre), n = 0;
    addStep({pos, 1}, SETVALUE);
    while(n != size() - 1) {
        size_t t = pos;
        addStep({pos, 2}, SETVALUE, false);
        auto randDir = selectRandomDir(pos);
        pos = randDir.next;
        if(field[pos].empty()) {
            link(t, pos, false);
            ++n;
        }
        addStep({pos, 1}, SETVALUE);
    }
}
void Space::WilsonsMaze() {
    uniform_int_distribution<> ud(0, size() - 1);
    size_t firstMazePart = ud(dre), begin, n = 0;
    addStep({firstMazePart, 2}, SETVALUE);
    while(n != size() - 1) {
        vector<uint8_t> vDir(size(), 0);
        do {
            begin = ud(dre);
        } while(!field[begin].empty() || begin == firstMazePart);
        size_t pos = begin;
        selectRandomDirRT randDir;
        addStep({pos, 3}, SETVALUE);
        while(field[pos].empty() && pos != firstMazePart) {
            randDir = selectRandomDir(pos);
            vDir[pos] = randDir.dir;
            addStep({pos, 1}, SETVALUE, false);
            addStep({randDir.next, 3}, SETVALUE);
            pos = randDir.next;
        }
        pos = begin;
        addStep({pos, 3}, SETVALUE);
        while(pos != randDir.next) {
            link(pos, offset(pos, vDir[pos]), false);
            addStep({pos, 2}, SETVALUE, false);
            addStep({offset(pos, vDir[pos]), 3}, SETVALUE);
            pos = offset(pos, vDir[pos]);
            ++n;
        }
        if(stepByStepFilling) {
            stepList.back().stepValue.y = 2;
            for(size_t i = 0; i < size(); ++i) {
                if(field[i].empty()) { addStep({i, 0}, SETVALUE, false); }
            }
            stepList.back().endOfStep = true;
        }
    }
}
void Space::huntAndKillMaze() {
    size_t pos = uniform_int_distribution<>(0, size() - 1)(dre), lowerBound = pos;
    vector<bool> visited(size(), false);
    visited[pos] = true;
    auto notVisited = [&](size_t i){return !visited[i];};
    selectRandomDirRT randDir = selectRandomDir(pos, notVisited);
    while(randDir.next != NaN) {
        do {
            link(pos, randDir.next);
            pos = randDir.next;
            visited[pos] = true;
            if(pos < lowerBound) lowerBound = pos;
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
bool Space::wall(size_t i, size_t dir) {
    static constexpr std::array wallArr =
        {&Space::wallR, &Space::wallL, &Space::wallU, &Space::wallD, &Space::wallUL, &Space::wallDR, &Space::wallUR, &Space::wallDL}; // (→ ← ↑ ↓ ↖ ↘ ↗ ↙)
    return (this->*(wallArr[dir]))(i);
}
size_t Space::offset(size_t i, size_t dir) {
    static constexpr std::array offsetArr =
        {&Space::right, &Space::left, &Space::up, &Space::down, &Space::upleft, &Space::downright, &Space::upright, &Space::downleft}; // (→ ← ↑ ↓ ↖ ↘ ↗ ↙)
    return (this->*(offsetArr[dir]))(i);
}
vector<size_t> Space::BFS(size_t from) {
    vector<bool> discovered(size(), false);
    vector<size_t> parent(size(), -1);
    deque<size_t> d({from});
    while(!d.empty()) {
        size_t front = d.front();
        d.pop_front();
        discovered[front] = true;
        for(auto i : field[front].next) {
            if(!discovered[i]) {
                d.push_front(i);
                parent[i] = front;
                discovered[i] = true;
            }
        }
    }
    return parent;
}
list<size_t> Space::BFSfind(size_t from, size_t to) {
    auto parent = BFS(from);
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
void Space::DFS(size_t from) {
    vector<bool> discovered(size(), false);
    discovered[from] = true;
    vector<size_t> entry(size()), leave(size());
    entry[from] = 1;
    deque<size_t> d({from});
    unsigned time = 1;
    while(!d.empty()) {
        size_t front = d.front();
        for(auto i : field[front].next) {
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
// sandbox
double Space::radiansToDegrees(double r) {
    return r * (180 / M_PI);
}
double Space::degreesToRadians(double d) {
    return d * M_PI / 180;
}
void Space::completeGraph(size_t verticles, float offset) {
    vector<Point2Df> v = {Point2Df(width / 4.5f, height / 4.5f)};
    v.reserve(verticles);
    for(int i = 1; i < verticles; ++i) {
        v.push_back(Point2Df(v.back().x * cos(degreesToRadians(360.0f / verticles + offset)) - v.back().y * sin(degreesToRadians(360.0f / verticles + offset)),
                             v.back().x * sin(degreesToRadians(360.0f / verticles + offset)) + v.back().y * cos(degreesToRadians(360.0f / verticles + offset))));
    }
    vector<size_t> p;
    p.reserve(verticles);
    for(int i = 0; i < verticles; ++i) {
        p[i] = get1DCoordinates(Point2Du(v[i].x + width / 2.0f, v[i].y + height / 2.0f));
    }
    for(int i = 0; i < verticles; ++i) {
        for(int j = i; j < verticles; ++j) {
            link(p[i], p[j]);
        }
    }
}
void Space::graphTraversal(size_t from) {
    field[from].value = 1;
    for(auto i : field[from].next) {
        if(!field[i].value)
            graphTraversal(i);
    }
}
vector<Point2Du> Space::combineIntervals(const vector<Point2Du>& s) {
    map<size_t, int> sum;
    for(auto& i : s) {
        ++sum[i.x];
        --sum[i.y];
    }
    vector<Point2Du> result;
    int n = 0, b = -1;
    for(auto it = sum.begin(); it != sum.end(); ++it) {
        n += it->second;
        if(n == 1 && b == -1) {
            b = it->first;
        } else if(n == 0) {
            result.push_back(Point2Du(b, it->first));
            b = -1;
        }
    }
    return result;
}
void Space::connectEdges() {
    for(size_t i = 0; i < width; ++i) {
        vector<Point2Du> s;
        for(size_t j = 0; j < height; ++j) {
            size_t n = get1DCoordinates(Point2Du(i, j));
            if(field[n].next.size() > 2)
                continue;
            for(auto c : field[n].next) {
                if(n < c && sameVertical(n, c) && field[c].next.size() < 3) {
                    s.push_back({n / width, c / width});
                }
            }
        }
        for(auto j : s) {
            unlink(get1DCoordinates(Point2Du(i, j.x)), get1DCoordinates(Point2Du(i, j.y)));
        }
        s = combineIntervals(s);
        for(auto j : s) {
            link(get1DCoordinates(Point2Du(i, j.x)), get1DCoordinates(Point2Du(i, j.y)));
        }
    }
    for(size_t i = 0; i < height; ++i) {
        vector<Point2Du> s;
        for(size_t j = 0; j < width; ++j) {
            size_t n = get1DCoordinates(Point2Du(j, i));
            if(field[n].next.size() > 2)
                continue;
            for(auto c : field[n].next) {
                if(n < c && sameHorizontal(n, c) && field[c].next.size() < 3) {
                    s.push_back({n % width, c % width});
                }
            }
        }
        for(auto j : s) {
            unlink(get1DCoordinates(Point2Du(j.x, i)), get1DCoordinates(Point2Du(j.y, i)));
        }
        s = combineIntervals(s);
        for(auto j : s) {
            link(get1DCoordinates(Point2Du(j.x, i)), get1DCoordinates(Point2Du(j.y, i)));
        }
    }
}
void Space::splitEdges() {

}
void Space::addStep(Point2Du stepValue, size_t stepType, bool endOfStep) {
    if(stepByStepFilling)
        stepList.push_back({stepValue, stepType, endOfStep});
}

size_t Space::mirrorX(size_t i) {
    return width * (i / width) + (width - (i % width) - 1);
}
size_t Space::mirrorY(size_t i) {
    return (height - (i / width) - 1) * width + i % width;
}
// (→ ← ↑ ↓ ↖ ↘ ↗ ↙)
bool Space::wallR(size_t i) { return i % width == width - 1; }
bool Space::wallL(size_t i) { return i % width == 0; }
bool Space::wallU(size_t i) { return i < width || height == 1; }
bool Space::wallD(size_t i) { return i > field.size() - width - 1 || height == 1; }
bool Space::wallUL(size_t i) { return wallU(i) || wallL(i); }
bool Space::wallUR(size_t i) { return wallU(i) || wallR(i); }
bool Space::wallDL(size_t i) { return wallD(i) || wallL(i); }
bool Space::wallDR(size_t i) { return wallD(i) || wallR(i); }
size_t Space::right(size_t i) { return i + 1; }
size_t Space::left(size_t i) { return i - 1; }
size_t Space::up(size_t i) { return i - width; }
size_t Space::down(size_t i) { return i + width; }
size_t Space::upleft(size_t i) { return up(left(i)); }
size_t Space::upright(size_t i) { return up(right(i)); }
size_t Space::downleft(size_t i) { return down(left(i)); }
size_t Space::downright(size_t i) { return down(right(i)); }
bool Space::sameHorizontal(size_t a, size_t b) { return a / width == b / width; }
bool Space::sameVertical(size_t a, size_t b) { return a % width == b % width; }
bool Space::sameDiagonal(size_t a, size_t b) { return (max(a, b) - min(a, b)) % (width + 1) == 0; }
bool Space::sameAntiDiagonal(size_t a, size_t b) { return (max(a, b) - min(a, b)) % (width - 1) == 0; }
bool Space::moveFrom(size_t& pos, size_t dir) {
    if(wall(pos, dir) || !(field[offset(pos, dir)].empty() && field[offset(pos, dir)].empty())) {
        return 0;
    }
    link(pos, offset(pos, dir));
    pos = offset(pos, dir);
    return 1;
}
size_t Space::reverseDir(size_t dir) { // (→ <-> ← etc)
    if(dir % 2 == 0) {
        return dir + 1;
    } else {
        return dir - 1;
    }
}
