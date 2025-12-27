#ifndef GOMOKU_TYPES_H
#define GOMOKU_TYPES_H

const int BOARD_SIZE = 15;
const int SEARCH_DEPTH = 4;
const int SEARCH_RANGE = 3;
// K-Value = Defense / Attack
const double DEFENSE_WEIGHT = 1.2; 

namespace PredefinedScore {
    const int WIN = 1e7;
    const int LOSE = -1e7;

    const int FIVE = 1e6;
    const int LIVE_4 = 1e5;
    const int RUSH_4 = 1e4;
    const int LIVE_3 = 8e3;
    const int SLEEP_3 = 1e3;
    const int LIVE_2 = 5e2;
    const int SLEEP_2 = 5e1;
}

enum class Role { EMPTY, USER, BOT };

class Point {
private:
    int x, y;
public:
    Point(int x = 0, int y = 0) : x(x), y(y) { }
    int getX() const { return x; }
    int getY() const { return y; }
    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }

    bool operator == (const Point &p) const {
        return x == p.x && y == p.y;
    }
};

using Direction = Point;

#endif
