#include "Space.h"
using namespace std;
Space::Space(size_t width, size_t height) : width(width), height(height) {
	field.resize(width * height);
    srand(time(0));
}
size_t Space::size() { return field.size(); }
Node& Space::operator[](size_t i) { return field[i]; }
Node Space::operator()(size_t i) { return field[i]; }
Node Space::operator()(size_t x, size_t y) { return field[x + y * width]; }
Node Space::operator()(Point2Du p) { return field[p.x + p.y * width]; }
Point2Du Space::get2DCoordinates(size_t i) { return Point2Du(i % width, i / width); }
size_t Space::get1DCoordinates(Point2Du p) { return p.x + p.y * width; }
void Space::link(size_t i, size_t with) {
	field[i].next = with;
	field[with].prev = i;
    field[with].value = field[i].value + 1;
}
void Space::mirrorX() {
    vector<Node> temp = move(field);
	field.resize(temp.size());
	for (size_t i = 0; i < temp.size(); ++i) {
		size_t mirrored = mirrorX(i);
		field[mirrored].value = temp[i].value;
		field[mirrored].next = mirrorX(temp[i].next);
		field[mirrored].prev = mirrorX(temp[i].prev);
	}
	initBegin(mirrorX(begin));
}
void Space::mirrorY() {
    vector<Node> temp = move(field);
	field.resize(temp.size());
	for (size_t i = 0; i < temp.size(); ++i) {
		size_t mirrored = mirrorY(i);
		field[mirrored].value = temp[i].value;
		field[mirrored].next = mirrorY(temp[i].next);
		field[mirrored].prev = mirrorY(temp[i].prev);
	}
	initBegin(mirrorY(begin));
}
void Space::horizontally() {
	initBegin(0);
    size_t pos = begin;
	bool d = 0;
    do {
        while (movefrom(pos, d ? LEFT : RIGHT)) { }
		d = !d;
    } while(movefrom(pos, DOWN));
}
void Space::zigzag() {
    initBegin(0);
    size_t pos = begin, n = 0, dirs[] = {RIGHT, DOWNLEFT, DOWN, UPRIGHT};
    for(int i = 0; i<8 * max(width, height); ++i, ++n) {
        size_t a = 0.25f * (3.0f + n + (1.0f - n) * pow(-1.0f, n));
        while(a-- && movefrom(pos, dirs[n % 4])) { }
    }
}
void Space::spiral() {
    initBegin(0);
    size_t pos = begin, n = 0, dirs[] = {RIGHT, DOWN, LEFT, UP};
    for(int i = 0; i<max(width, height) * 2 - 1; ++i, ++n) {
        while(movefrom(pos, dirs[n % 4])) { }
    }
}

void Space::initBegin(size_t i) { begin = i; }
size_t Space::mirrorX(size_t i) {
	if (i == -1)
		return -1;
	return width * (i / width) + (width - (i % width) - 1);
}
size_t Space::mirrorY(size_t i) {
	if (i == -1)
		return -1;
	return (height - (i / width) - 1) * width + i % width;
}
// → ← ↑ ↓ ↖ ↗ ↙ ↘
bool Space::wallR(size_t i) { return i % width == width - 1; }
bool Space::wallL(size_t i) { return i % width == 0; }
bool Space::wallU(size_t i) { return i < width || height == 1; }
bool Space::wallD(size_t i) { return i > field.size() - width - 1 || height == 1; }
bool Space::wallUL(size_t i) { return wallU(i) || wallL(i); }
bool Space::wallUR(size_t i) { return wallU(i) || wallR(i); }
bool Space::wallDL(size_t i) { return wallD(i) || wallL(i); }
bool Space::wallDR(size_t i) { return wallD(i) || wallR(i); }
size_t Space::right(size_t pos) { return pos + 1; }
size_t Space::left(size_t pos) { return pos - 1; }
size_t Space::up(size_t pos) { return pos - width; }
size_t Space::down(size_t pos) { return pos + width; }
size_t Space::upleft(size_t pos) { return up(left(pos)); }
size_t Space::upright(size_t pos) { return up(right(pos)); }
size_t Space::downleft(size_t pos) { return down(left(pos)); }
size_t Space::downright(size_t pos) { return down(right(pos)); }
bool Space::movefrom(size_t& pos, size_t dir) {
    switch(dir) {
    case RIGHT:
        if(wallR(pos) || !(field[right(pos)].prev == NaN && field[right(pos)].next == NaN))
            return 0;
        link(pos, right(pos));
        pos = right(pos);
        break;
    case LEFT:
        if(wallL(pos) || !(field[left(pos)].prev == NaN && field[left(pos)].next == NaN))
            return 0;
        link(pos, left(pos));
        pos = left(pos);
        break;
    case UP:
        if(wallU(pos) || !(field[up(pos)].prev == NaN && field[up(pos)].next == NaN))
            return 0;
        link(pos, up(pos));
        pos = up(pos);
        break;
    case DOWN:
        if(wallD(pos) || !(field[down(pos)].prev == NaN && field[down(pos)].next == NaN))
            return 0;
        link(pos, down(pos));
        pos = down(pos);
        break;
    case UPLEFT:
        if(wallUL(pos) || !(field[upleft(pos)].prev == NaN && field[upleft(pos)].next == NaN))
            return 0;
        link(pos, upleft(pos));
        pos = upleft(pos);
        break;
    case UPRIGHT:
        if(wallUR(pos) || !(field[upright(pos)].prev == NaN && field[upright(pos)].next == NaN))
            return 0;
        link(pos, upright(pos));
        pos = upright(pos);
        break;
    case DOWNLEFT:
        if(wallDL(pos) || !(field[downleft(pos)].prev == NaN && field[downleft(pos)].next == NaN))
            return 0;
        link(pos, downleft(pos));
        pos = downleft(pos);
        break;
    case DOWNRIGHT:
        if(wallDR(pos) || !(field[downright(pos)].prev == NaN && field[downright(pos)].next == NaN))
            return 0;
        link(pos, downright(pos));
        pos = downright(pos);
        break;
    }
    return 1;
}
