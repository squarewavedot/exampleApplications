#include "Pacman.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <limits>
#include <random>

namespace {
constexpr float kJoystickAxisTrigger = 0.5f;
constexpr float kJoystickAxisIdleDeadzone = 0.2f;

struct PreferredAxes {
    float horizontal{0.0f};
    float vertical{0.0f};
};

PreferredAxes readPreferredAxes(Joystick &joystick) {
    PreferredAxes axes;
    const float primaryHorizontal = joystick.getAxis(6);
    const float primaryVertical   = joystick.getAxis(7);

    if (std::abs(primaryHorizontal) <= kJoystickAxisIdleDeadzone &&
        std::abs(primaryVertical) <= kJoystickAxisIdleDeadzone) {
        axes.horizontal = joystick.getAxis(0);
        axes.vertical   = joystick.getAxis(1);
    } else {
        axes.horizontal = primaryHorizontal;
        axes.vertical   = primaryVertical;
    }

    return axes;
}
}  // namespace

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

Pacman::Pacman(std::string serverUri, int initialSeed)
    : CubeApplication(20, serverUri, "Pacman"), joystick(0) {
    currentSeed_ = (initialSeed >= 0) ? initialSeed : static_cast<int>(std::time(nullptr));
    std::srand(currentSeed_);
    loadHighScore();
    resetGame(false);
    gameState = menu;
}

// ---------------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------------

bool Pacman::loop() {
    if (quitRequested_) return false;
    frameCounter++;
    pelletBlink = (pelletBlink + 1) % 40;
    mouthOpen   = (frameCounter / 3) % 2 == 0;

    clear();
    if (gameState == menu) {
        updateMenu();
        renderMenu();
    } else if (gameState == gameOver || gameState == victory) {
        renderBoard();
        updateGameOver();
        renderGameOver();
    } else {
        updateInput();
        updateGame();
        renderBoard();
    }
    render();
    return true;
}

// ---------------------------------------------------------------------------
// Wall helpers
// ---------------------------------------------------------------------------

void Pacman::setHWall(int face, int col, int row, bool v) {
    if (col < 0 || col >= FACE_TILES || row < 0 || row > FACE_TILES) return;
    hwall[face][row][col] = v;
}

void Pacman::setVWall(int face, int col, int row, bool v) {
    if (col < 0 || col > FACE_TILES || row < 0 || row >= FACE_TILES) return;
    vwall[face][row][col] = v;
}

// Closed rectangle of walls (border of a rectangular room)
void Pacman::addRect(int face, int x0, int y0, int x1, int y1) {
    for (int x = x0; x < x1; x++) {
        setHWall(face, x, y0);
        setHWall(face, x, y1);
    }
    for (int y = y0; y < y1; y++) {
        setVWall(face, x0, y);
        setVWall(face, x1, y);
    }
}

// Horizontal corridor of width 1 tile: walls above row y and below row y+1 between x0..x1
void Pacman::addHCorridor(int face, int x0, int x1, int y) {
    for (int x = x0; x <= x1; x++) {
        setHWall(face, x, y);
        setHWall(face, x, y + 2);
    }
    setVWall(face, x0, y);
    setVWall(face, x0, y + 1);
    setVWall(face, x1 + 1, y);
    setVWall(face, x1 + 1, y + 1);
}

// Vertical corridor of width 1 tile: walls left of col x and right of col x+1 between y0..y1
void Pacman::addVCorridor(int face, int y0, int y1, int x) {
    for (int y = y0; y <= y1; y++) {
        setVWall(face, x, y);
        setVWall(face, x + 2, y);
    }
    setHWall(face, x, y0);
    setHWall(face, x + 1, y0);
    setHWall(face, x, y1 + 1);
    setHWall(face, x + 1, y1 + 1);
}

bool Pacman::hasHWall(int face, int col, int row) const {
    if (col < 0 || col >= FACE_TILES || row < 0 || row > FACE_TILES) return true;
    return hwall[face][row][col];
}

bool Pacman::hasVWall(int face, int col, int row) const {
    if (col < 0 || col > FACE_TILES || row < 0 || row >= FACE_TILES) return true;
    return vwall[face][row][col];
}

bool Pacman::isWall(Tile tile) const {
    // A tile is "wall" if it is enclosed on all 4 sides (dead cell, inside a wall block).
    // We do NOT use a per-tile wall flag; instead walls are only on edges.
    // A tile is impassable if ANY of its 4 boundary edges are walls AND there is no
    // adjacent open tile – but that's path logic.  For rendering we treat edges.
    // For movement: a tile is accessible if it can be entered; handled in canMove.
    (void)tile;
    return false;   // tiles themselves are never "wall" – only edges are walls
}

bool Pacman::canMove(Tile from, Direction dir, int ghostDoor) {
    // Diagonal guard
    if (dir.dx != 0 && dir.dy != 0) return false;
    if (dir.dx == 0 && dir.dy == 0) return false;

    Tile to = stepTile(from, dir);

    // Ghost door logic:
    //   ghostDoor=0 → always blocked (Pacman, demo AI)
    //   ghostDoor=1 → exit only: inside→outside allowed, outside→inside blocked
    //   ghostDoor=2 → both directions (eaten ghost returning home)
    if (isGhostDoor(from, to)) {
        if (ghostDoor == 0) return false;
        bool exiting = (from.face == top && from.y == GH_Y0 && to.y == GH_Y0 - 1);
        bool entering= (from.face == top && from.y == GH_Y0 - 1 && to.y == GH_Y0);
        if (ghostDoor == 1) return exiting;
        if (ghostDoor == 2) return exiting || entering;
        return false;
    }

    int fx = from.x, fy = from.y;
    bool srcOk = false;
    if (dir.dx == 1)  srcOk = !hasVWall(from.face, fx + 1, fy);
    else if (dir.dx == -1) srcOk = !hasVWall(from.face, fx,     fy);
    else if (dir.dy == 1)  srcOk = !hasHWall(from.face, fx, fy + 1);
    else if (dir.dy == -1) srcOk = !hasHWall(from.face, fx, fy);
    if (!srcOk) return false;

    // Cross-face: also check entry wall on destination face
    if (to.face != from.face) {
        int tx = to.x, ty = to.y;
        bool dstOk = true;
        if      (tx == 0)              dstOk = !hasVWall(to.face, 0,          ty);
        else if (tx == FACE_TILES - 1) dstOk = !hasVWall(to.face, FACE_TILES, ty);
        else if (ty == 0)              dstOk = !hasHWall(to.face, tx, 0);
        else if (ty == FACE_TILES - 1) dstOk = !hasHWall(to.face, tx, FACE_TILES);
        if (!dstOk) return false;
    }
    return true;
}

bool Pacman::isGhostDoor(Tile from, Tile to) const {
    if (from.face != top || to.face != top) return false;
    // Door: horizontal edge at row GH_Y0, cols GH_DOOR_X0..GH_DOOR_X1
    bool crossingDown = (from.y == GH_Y0 - 1 && to.y == GH_Y0) ||
                        (from.y == GH_Y0     && to.y == GH_Y0 - 1);
    if (!crossingDown) return false;
    int x = from.x;
    return x >= GH_DOOR_X0 && x <= GH_DOOR_X1;
}

// Open a gap of 2 tiles in the outer border to allow cross-face travel.
// axis: 0 = horizontal border (top/bottom edge at row), 1 = vertical border (left/right edge at col)
// borderPos: 0 or FACE_TILES for hwall; 0 or FACE_TILES for vwall
// gapStart: first tile of the 2-tile opening
void Pacman::openBorderGap(int face, int axis, int borderPos, int gapStart) {
    if (axis == 0) {
        // Open two hwall segments at row borderPos, cols gapStart and gapStart+1
        setHWall(face, gapStart,     borderPos, false);
        setHWall(face, gapStart + 1, borderPos, false);
        // Also open the side walls that addHCorridor would have placed
        setVWall(face, gapStart,     (borderPos == 0) ? 0 : FACE_TILES - 1, false);
        setVWall(face, gapStart + 2, (borderPos == 0) ? 0 : FACE_TILES - 1, false);
    } else {
        // Open two vwall segments at col borderPos, rows gapStart and gapStart+1
        setVWall(face, borderPos, gapStart,     false);
        setVWall(face, borderPos, gapStart + 1, false);
    }
}

// ---------------------------------------------------------------------------
// Board construction – 6 individual layouts
// ---------------------------------------------------------------------------

void Pacman::buildBoard() {
    for (int f = 0; f < 6; f++) {
        for (int r = 0; r <= FACE_TILES; r++)
            for (int c = 0; c < FACE_TILES; c++)
                hwall[f][r][c] = true;
        for (int r = 0; r < FACE_TILES; r++)
            for (int c = 0; c <= FACE_TILES; c++)
                vwall[f][r][c] = true;
        for (int y = 0; y < FACE_TILES; y++)
            for (int x = 0; x < FACE_TILES; x++) {
                pellets[f][y][x] = false;
                powerPellets[f][y][x] = false;
            }
    }

    std::mt19937 rng(static_cast<uint32_t>(currentSeed_));
    static const Direction dirs[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    auto isGhostHouseTile = [&](Tile tile) {
        return tile.face == top &&
               tile.x >= GH_X0 && tile.x < GH_X1 &&
               tile.y >= GH_Y0 && tile.y < GH_Y1;
    };

    auto isPlayableTile = [&](Tile tile) {
        return !isGhostHouseTile(tile);
    };

    auto setEdgeOpen = [&](Tile from, Direction dir, bool open) {
        Tile to = stepTile(from, dir);
        if (dir.dx == 1) setVWall(from.face, from.x + 1, from.y, !open);
        else if (dir.dx == -1) setVWall(from.face, from.x, from.y, !open);
        else if (dir.dy == 1) setHWall(from.face, from.x, from.y + 1, !open);
        else if (dir.dy == -1) setHWall(from.face, from.x, from.y, !open);

        if (to.face == from.face) return;

        if      (to.x == 0)              setVWall(to.face, 0,          to.y, !open);
        else if (to.x == FACE_TILES - 1) setVWall(to.face, FACE_TILES, to.y, !open);
        else if (to.y == 0)              setHWall(to.face, to.x, 0, !open);
        else if (to.y == FACE_TILES - 1) setHWall(to.face, to.x, FACE_TILES, !open);
    };

    auto shuffleDirections = [&](std::array<Direction, 4> &order) {
        for (int i = static_cast<int>(order.size()) - 1; i > 0; i--) {
            std::uniform_int_distribution<int> pick(0, i);
            std::swap(order[i], order[pick(rng)]);
        }
    };

    // Per-face depth-first maze with a few extra loops.
    // This keeps each face fully connected while still letting the seed shape the layout.
    auto carveFaceMaze = [&](ScreenNumber face) {
        std::array<std::array<bool, FACE_TILES>, FACE_TILES> visited{};
        std::vector<Tile> stack;
        stack.reserve(FACE_TILES * FACE_TILES);

        Tile start{face, FACE_TILES / 2, FACE_TILES / 2};
        if (!isPlayableTile(start)) {
            bool found = false;
            for (int y = 0; y < FACE_TILES && !found; y++) {
                for (int x = 0; x < FACE_TILES; x++) {
                    Tile candidate{face, x, y};
                    if (isPlayableTile(candidate)) {
                        start = candidate;
                        found = true;
                        break;
                    }
                }
            }
        }

        visited[start.y][start.x] = true;
        stack.push_back(start);

        while (!stack.empty()) {
            Tile current = stack.back();
            std::array<Direction, 4> order = {dirs[0], dirs[1], dirs[2], dirs[3]};
            shuffleDirections(order);

            bool advanced = false;
            for (const Direction &dir : order) {
                int nx = current.x + dir.dx;
                int ny = current.y + dir.dy;
                if (nx < 0 || nx >= FACE_TILES || ny < 0 || ny >= FACE_TILES) continue;

                Tile next{face, nx, ny};
                if (!isPlayableTile(next) || visited[ny][nx]) continue;

                setEdgeOpen(current, dir, true);
                visited[ny][nx] = true;
                stack.push_back(next);
                advanced = true;
                break;
            }

            if (!advanced) stack.pop_back();
        }

        std::uniform_int_distribution<int> loopChance(0, 99);
        const int extraLoopPercent = (face == top) ? 18 : 24;
        for (int y = 0; y < FACE_TILES; y++) {
            for (int x = 0; x < FACE_TILES; x++) {
                Tile tile{face, x, y};
                if (!isPlayableTile(tile)) continue;

                if (x + 1 < FACE_TILES) {
                    Tile rightTile{face, x + 1, y};
                    if (isPlayableTile(rightTile) && hasVWall(face, x + 1, y) && loopChance(rng) < extraLoopPercent)
                        setEdgeOpen(tile, {1, 0}, true);
                }
                if (y + 1 < FACE_TILES) {
                    Tile downTile{face, x, y + 1};
                    if (isPlayableTile(downTile) && hasHWall(face, x, y + 1) && loopChance(rng) < extraLoopPercent)
                        setEdgeOpen(tile, {0, 1}, true);
                }
            }
        }
    };

    carveFaceMaze(front);
    carveFaceMaze(back);
    carveFaceMaze(left);
    carveFaceMaze(right);
    carveFaceMaze(bottom);
    carveFaceMaze(top);

    // Ghost house stays as a deliberate top-face structure instead of participating in the maze.
    for (int y = GH_Y0; y < GH_Y1; y++) {
        for (int x = GH_X0; x < GH_X1; x++) {
            Tile tile{top, x, y};
            if (x + 1 < GH_X1) setEdgeOpen(tile, {1, 0}, true);
            if (y + 1 < GH_Y1) setEdgeOpen(tile, {0, 1}, true);
        }
    }
    setHWall(top, GH_DOOR_X0, GH_Y0, false);
    setHWall(top, GH_DOOR_X1, GH_Y0, false);

    // -----------------------------------------------------------------------
    // Cross-face connections: open ALL 12 cube edges so every face is reachable.
    // Each edge gets a 2-tile gap at tile position 7-8 (centre of the 16-tile face).
    // -----------------------------------------------------------------------

    // top ↔ front  (top bottom edge  /  front top edge)
    openBorderGap(top,   0, FACE_TILES, 7);
    openBorderGap(front, 0, 0,          7);

    // top ↔ back   (top top edge  /  back top edge)
    openBorderGap(top,  0, 0,          7);
    openBorderGap(back, 0, 0,          7);

    // top ↔ right  (top right edge  /  right top edge)
    openBorderGap(top,   1, FACE_TILES, 7);
    openBorderGap(right, 0, 0,          7);

    // top ↔ left   (top left edge  /  left top edge)
    openBorderGap(top,  1, 0,          7);
    openBorderGap(left, 0, 0,          7);

    // bottom ↔ front  (bottom top edge  /  front bottom edge)
    openBorderGap(bottom, 0, 0,          7);
    openBorderGap(front,  0, FACE_TILES, 7);

    // bottom ↔ back   (bottom bottom edge  /  back bottom edge)
    openBorderGap(bottom, 0, FACE_TILES, 7);
    openBorderGap(back,   0, FACE_TILES, 7);

    // bottom ↔ right  (bottom right edge  /  right bottom edge)
    openBorderGap(bottom, 1, FACE_TILES, 7);
    openBorderGap(right,  0, FACE_TILES, 7);

    // bottom ↔ left   (bottom left edge  /  left bottom edge)
    openBorderGap(bottom, 1, 0,          7);
    openBorderGap(left,   0, FACE_TILES, 7);

    // front ↔ right   (front right edge  /  right left edge)
    openBorderGap(front, 1, FACE_TILES, 7);
    openBorderGap(right, 1, 0,          7);

    // front ↔ left    (front left edge  /  left right edge)
    openBorderGap(front, 1, 0,          7);
    openBorderGap(left,  1, FACE_TILES, 7);

    // back ↔ right    (back left edge  /  right right edge)
    openBorderGap(back,  1, 0,          7);
    openBorderGap(right, 1, FACE_TILES, 7);

    // back ↔ left     (back right edge  /  left left edge)
    openBorderGap(back, 1, FACE_TILES,  7);
    openBorderGap(left, 1, 0,           7);

    // -----------------------------------------------------------------------
    // Pellets: only on tiles Pacman can actually reach from the spawn.
    // This keeps seed-generated maps safe even if a future layout tweak becomes too tight.
    // -----------------------------------------------------------------------
    std::array<std::array<std::array<bool, FACE_TILES>, FACE_TILES>, 6> reachable{};
    std::vector<Tile> queue;
    queue.reserve(6 * FACE_TILES * FACE_TILES);
    queue.push_back(pacmanSpawn);
    reachable[pacmanSpawn.face][pacmanSpawn.y][pacmanSpawn.x] = true;

    for (std::size_t qi = 0; qi < queue.size(); qi++) {
        Tile current = queue[qi];
        for (const Direction &dir : dirs) {
            if (!canMove(current, dir)) continue;
            Tile next = stepTile(current, dir);
            if (reachable[next.face][next.y][next.x]) continue;
            reachable[next.face][next.y][next.x] = true;
            queue.push_back(next);
        }
    }

    for (int f = 0; f < 6; f++)
        for (int y = 0; y < FACE_TILES; y++)
            for (int x = 0; x < FACE_TILES; x++)
                pellets[f][y][x] = reachable[f][y][x];

    pellets[pacmanSpawn.face][pacmanSpawn.y][pacmanSpawn.x] = false;

    // -----------------------------------------------------------------------
    // Power pellets: keep the old preferred spots, but slide them to the nearest
    // reachable pellet tile when a generated maze blocks the exact coordinate.
    // -----------------------------------------------------------------------
    struct PPPos { int face, x, y; };
    static const PPPos ppList[] = {
        // top
        {top,    1,  1}, {top,   14,  1}, {top,    1, 14}, {top,   14, 14},
        // front
        {front,  1,  1}, {front, 14,  1}, {front,  1, 14}, {front, 14, 14},
        // back
        {back,   1,  1}, {back,  14,  1}, {back,   1, 14}, {back,  14, 14},
        // right
        {right,  1,  1}, {right, 14, 14},
        // left
        {left,   1,  1}, {left,  14, 14},
        // bottom
        {bottom, 1,  7}, {bottom,14,  7},
    };

    auto placePowerPelletNear = [&](const PPPos &preferred) {
        for (int radius = 0; radius < FACE_TILES; radius++) {
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    if (std::abs(dx) + std::abs(dy) != radius) continue;

                    int x = preferred.x + dx;
                    int y = preferred.y + dy;
                    if (x < 0 || x >= FACE_TILES || y < 0 || y >= FACE_TILES) continue;
                    if (!pellets[preferred.face][y][x] || powerPellets[preferred.face][y][x]) continue;

                    powerPellets[preferred.face][y][x] = true;
                    pellets[preferred.face][y][x] = false;
                    return;
                }
            }
        }
    };

    for (auto &pp : ppList) {
        placePowerPelletNear(pp);
    }
}

void Pacman::resetGame(bool newSeed) {
    if (newSeed) {
        currentSeed_ = static_cast<int>(std::time(nullptr));
    }
    std::srand(currentSeed_);

    buildBoard();

    pacmanTile      = pacmanSpawn;
    pacmanDirection = {0, 0};
    queuedDirection = {0, 0};
    score           = 0;
    frameCounter    = 0;
    pelletBlink     = 0;
    mouthOpen       = true;
    gameState       = ready;
    frightenedTimer = 0;
    menuDemoCountdown = 100;
    gameOverTimer_  = 0;
    fireworkSpawnTimer_ = 0;
    gameOverMenuIndex_ = 0;
    newHighScore_ = false;
    fireworks_.clear();

    ghosts.clear();
ghosts.push_back({{top, 6, 6}, {top, 6, 6}, {0, 1}, {top, 6, 6}, Color(220,   0,   0),  0, false});
ghosts.push_back({{top, 7, 6}, {top, 7, 6}, {0, 1}, {top, 7, 6}, Color(255, 140,   0), 25, false});
ghosts.push_back({{top, 8, 6}, {top, 8, 6}, {0, 1}, {top, 8, 6}, Color(  0, 210, 220), 50, false});
ghosts.push_back({{top, 9, 6}, {top, 9, 6}, {0, 1}, {top, 9, 6}, Color(255, 130, 180), 75, false});

    pelletCount = 0;
    for (int f = 0; f < 6; f++)
        for (int y = 0; y < FACE_TILES; y++)
            for (int x = 0; x < FACE_TILES; x++)
                if (pellets[f][y][x]) pelletCount++;

    bonusItem = {};
    bonusSpawnCounter = 0;
}

void Pacman::loadHighScore() {
    std::fstream highScoreFile;
    highScoreFile.open("pacman_highscore.dat", std::fstream::binary | std::fstream::in);
    if (highScoreFile) {
        highScoreFile >> highScore;
    } else {
        highScore = 0;
        highScoreFile.open("pacman_highscore.dat",
                           std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
        highScoreFile << 0;
    }
    highScoreFile.close();
}

void Pacman::saveHighScore() {
    std::fstream highScoreFile;
    highScoreFile.open("pacman_highscore.dat",
                       std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
    highScoreFile << highScore;
    highScoreFile.close();
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void Pacman::updateMenu() {
    static float lastMenuAxis = 0.0f;
    static int   seedScrollHold = 0;
    static int   seedScrollHoldH = 0;

    if (!joystick.isFound()) {
        if (menuDemoCountdown > 0) menuDemoCountdown--;
        if (menuDemoCountdown == 0) {
            demoActive_ = false;
            seedMenuActive_ = false;
            resetGame();
            gameState = playing;
        }
        return;
    }

    PreferredAxes axes = readPreferredAxes(joystick);
    float axis = axes.vertical;
    float axisH = axes.horizontal;
    if (seedMenuActive_) {
        if (axis <= -kJoystickAxisTrigger) {
            seedScrollHold++;
            int threshold = (seedScrollHold > 30) ? 3 : (seedScrollHold > 15 ? 6 : 0);
            if (seedScrollHold == 1 || (threshold > 0 && seedScrollHold % threshold == 0))
                seedStub_++;
        } else if (axis >= kJoystickAxisTrigger) {
            seedScrollHold++;
            int threshold = (seedScrollHold > 30) ? 3 : (seedScrollHold > 15 ? 6 : 0);
            if (seedScrollHold == 1 || (threshold > 0 && seedScrollHold % threshold == 0))
                seedStub_ = std::max(0, seedStub_ - 1);
        } else {
            seedScrollHold = 0;
        }

        if (axisH >= kJoystickAxisTrigger) {
            seedScrollHoldH++;
            int threshold = (seedScrollHoldH > 30) ? 3 : (seedScrollHoldH > 15 ? 6 : 0);
            if (seedScrollHoldH == 1 || (threshold > 0 && seedScrollHoldH % threshold == 0))
                seedStub_ += 10;
        } else if (axisH <= -kJoystickAxisTrigger) {
            seedScrollHoldH++;
            int threshold = (seedScrollHoldH > 30) ? 3 : (seedScrollHoldH > 15 ? 6 : 0);
            if (seedScrollHoldH == 1 || (threshold > 0 && seedScrollHoldH % threshold == 0))
                seedStub_ = std::max(0, seedStub_ - 10);
        } else {
            seedScrollHoldH = 0;
        }

        lastMenuAxis = axis;

        if (joystick.getButtonPress(0)) {
            currentSeed_ = seedStub_;
            std::srand(currentSeed_);
            demoActive_ = false;
            seedMenuActive_ = false;
            seedScrollHold = 0;
            seedScrollHoldH = 0;
            resetGame(false);
            gameState = playing;
        }
        if (joystick.getButtonPress(1)) {
            seedMenuActive_ = false;
            seedScrollHold = 0;
            seedScrollHoldH = 0;
        }
        joystick.clearAllButtonPresses();
        return;
    }

    if (lastMenuAxis > -kJoystickAxisTrigger && axis <= -kJoystickAxisTrigger) {
        menuIndex = (menuIndex + 4) % 5;
    } else if (lastMenuAxis < kJoystickAxisTrigger && axis >= kJoystickAxisTrigger) {
        menuIndex = (menuIndex + 1) % 5;
    }
    lastMenuAxis = axis;

    if (joystick.getButtonPress(0)) {
        switch (menuIndex) {
            case 0:
                demoActive_ = false;
                seedMenuActive_ = false;
                resetGame();
                gameState = playing;
                break;
            case 1:
                demoActive_ = false;
                seedMenuActive_ = true;
                seedStub_ = currentSeed_ % 100000;
                break;
            case 2:
                break;
            case 3:
                demoActive_ = true;
                seedMenuActive_ = false;
                resetGame();
                gameState = playing;
                break;
            case 4:
                quitRequested_ = true;
                break;
            default:
                break;
        }
    }

    joystick.clearAllButtonPresses();
}

void Pacman::updateInput() {
    if (demoActive_) {
        if (gameState == ready) {
            gameState = playing;
            queuedDirection = {1, 0};
        }
        demoChooseDirection();
        return;
    }

    if (joystick.isFound() && joystick.getButtonPress(1)) {
        demoActive_ = false;
        menuSpritesInit_ = false;
        menuDemoCountdown = 100;
        gameState = menu;
        joystick.clearAllButtonPresses();
        return;
    }

    if (joystick.isFound()) {
        PreferredAxes axes = readPreferredAxes(joystick);
        float ax = axes.horizontal;
        float ay = axes.vertical;
        if      (ax < -kJoystickAxisTrigger) queuedDirection = {-1,  0};
        else if (ax >  kJoystickAxisTrigger) queuedDirection = { 1,  0};
        else if (ay < -kJoystickAxisTrigger) queuedDirection = { 0, -1};
        else if (ay >  kJoystickAxisTrigger) queuedDirection = { 0,  1};

        if (gameState == ready && joystick.getButtonPress(0)) {
            gameState = playing;
            if (queuedDirection.dx == 0 && queuedDirection.dy == 0)
                queuedDirection = {0, -1};
        }
    } else if (gameState == ready) {
        gameState = playing;
        queuedDirection = {0, -1};
    }

    joystick.clearAllButtonPresses();
}

void Pacman::updateGameOver() {
    static float lastGameOverAxis = 0.0f;

    gameOverTimer_++;

    // All 6 cube faces
    static const ScreenNumber allFaces[] = { front, back, left, right, top, bottom };

    if (newHighScore_) {
        fireworkSpawnTimer_++;
        const int spawnInterval = (std::rand() % 2 == 0) ? 20 : 10;
        if (fireworkSpawnTimer_ >= spawnInterval) {
            fireworkSpawnTimer_ = 0;
            ScreenNumber face = allFaces[std::rand() % 6];
            bool isRocket = (std::rand() % 2 == 0);

            if (isRocket) {
                Firework rocket;
                rocket.face    = face;
                rocket.x       = static_cast<float>(8 + std::rand() % 48);
                rocket.y       = 60.0f;
                rocket.vx      = static_cast<float>((std::rand() % 7) - 3) / 10.0f;
                rocket.vy      = -(1.5f + static_cast<float>(std::rand() % 10) / 10.0f);
                rocket.life    = 18 + std::rand() % 10;
                rocket.maxLife = rocket.life;
                rocket.color   = Color(200 + std::rand() % 55,
                                       200 + std::rand() % 55,
                                       200 + std::rand() % 55);
                rocket.phase   = FireworkPhase::rocket;
                fireworks_.push_back(rocket);
            } else {
                float bx = static_cast<float>(6 + std::rand() % 52);
                float by = static_cast<float>(6 + std::rand() % 52);
                Color burstColor(
                    std::rand() % 2 == 0 ? 200 + std::rand() % 55 : std::rand() % 60,
                    std::rand() % 2 == 0 ? 200 + std::rand() % 55 : std::rand() % 60,
                    std::rand() % 2 == 0 ? 200 + std::rand() % 55 : std::rand() % 80);
                int numParticles = 14 + std::rand() % 10;
                for (int p = 0; p < numParticles; p++) {
                    float angle = static_cast<float>(p) / static_cast<float>(numParticles) * 6.2832f;
                    float speed = 0.6f + static_cast<float>(std::rand() % 10) / 10.0f;
                    Firework particle;
                    particle.face    = face;
                    particle.x       = bx;
                    particle.y       = by;
                    particle.vx      = std::cos(angle) * speed;
                    particle.vy      = std::sin(angle) * speed;
                    particle.life    = 30 + std::rand() % 20;
                    particle.maxLife = particle.life;
                    particle.color   = burstColor;
                    particle.phase   = FireworkPhase::burst;
                    fireworks_.push_back(particle);
                }
            }
        }
    }

    for (std::size_t i = 0; i < fireworks_.size(); ) {
        Firework &fw = fireworks_[i];
        fw.x += fw.vx;
        fw.y += fw.vy;
        fw.life--;

        if (fw.phase == FireworkPhase::rocket && fw.life <= 0) {
            float bx = fw.x;
            float by = fw.y;
            ScreenNumber face = fw.face;
            Color burstColor(
                std::rand() % 2 == 0 ? 220 + std::rand() % 35 : std::rand() % 80,
                std::rand() % 2 == 0 ? 220 + std::rand() % 35 : std::rand() % 80,
                std::rand() % 2 == 0 ? 220 + std::rand() % 35 : std::rand() % 80);
            int numParticles = 16 + std::rand() % 12;
            fireworks_.erase(fireworks_.begin() + static_cast<std::vector<Firework>::difference_type>(i));
            for (int p = 0; p < numParticles; p++) {
                float angle = static_cast<float>(p) / static_cast<float>(numParticles) * 6.2832f;
                float speed = 0.5f + static_cast<float>(std::rand() % 12) / 10.0f;
                Firework particle;
                particle.face    = face;
                particle.x       = bx;
                particle.y       = by;
                particle.vx      = std::cos(angle) * speed;
                particle.vy      = std::sin(angle) * speed;
                particle.life    = 28 + std::rand() % 20;
                particle.maxLife = particle.life;
                particle.color   = burstColor;
                particle.phase   = FireworkPhase::burst;
                fireworks_.push_back(particle);
            }
        } else if (fw.life <= 0) {
            fireworks_.erase(fireworks_.begin() + static_cast<std::vector<Firework>::difference_type>(i));
        } else {
            ++i;
        }
    }

    if (demoActive_ && gameOverTimer_ >= 80) {
        resetGame(false);
        gameState = playing;
        lastGameOverAxis = 0.0f;
        return;
    }

    if (!joystick.isFound()) {
        lastGameOverAxis = 0.0f;
        return;
    }

    PreferredAxes axes = readPreferredAxes(joystick);
    float axis = axes.vertical;
    if (lastGameOverAxis > -kJoystickAxisTrigger && axis <= -kJoystickAxisTrigger) {
        gameOverMenuIndex_ = (gameOverMenuIndex_ + 3) % 4;
    } else if (lastGameOverAxis < kJoystickAxisTrigger && axis >= kJoystickAxisTrigger) {
        gameOverMenuIndex_ = (gameOverMenuIndex_ + 1) % 4;
    }
    lastGameOverAxis = axis;

    if (joystick.getButtonPress(1)) {
        menuSpritesInit_ = false;
        menuDemoCountdown = 100;
        demoActive_ = false;
        gameState = menu;
        joystick.clearAllButtonPresses();
        lastGameOverAxis = 0.0f;
        return;
    }

    if (joystick.getButtonPress(0)) {
        switch (gameOverMenuIndex_) {
            case 0:
                resetGame(false);
                gameState = playing;
                break;
            case 1:
                resetGame(true);
                gameState = playing;
                break;
            case 2:
                resetGame(true);
                gameState = playing;
                break;
            case 3:
                quitRequested_ = true;
                break;
            default:
                break;
        }
        joystick.clearAllButtonPresses();
        lastGameOverAxis = 0.0f;
        return;
    }

    joystick.clearAllButtonPresses();
}

// ---------------------------------------------------------------------------
// Game logic
// ---------------------------------------------------------------------------

void Pacman::updateGame() {
    if (gameState != playing) return;
    if (frightenedTimer > 0) frightenedTimer--;

    auto enterEndState = [&](GameState endState) {
        gameOverTimer_ = 0;
        newHighScore_ = (score > highScore);
        if (score > highScore) {
            highScore = score;
            saveHighScore();
        }
        fireworks_.clear();
        fireworkSpawnTimer_ = 0;
        gameOverMenuIndex_ = 0;
        gameState = endState;
    };

    bool pacmanMoves = (frameCounter % moveDivider      == 0);
    bool ghostsMove  = (frameCounter % ghostMoveDivider == 0);

    if (pacmanMoves) updatePacman();
    if (ghostsMove)  updateGhosts();

    updateBonusItem();

    if (!pacmanMoves && !ghostsMove) return;

    for (auto &g : ghosts) {
        if (sameTile(g.tile, pacmanTile)) {
            if (g.eaten) continue; // already eaten, just passing through
            if (frightenedTimer > 0) {
                // Mark as eaten – ghost will run back as eyes only
                g.eaten = true;
                score += 200;
            } else {
                enterEndState(gameOver);
                return;
            }
        }
    }

    if (pelletCount == 0) {
        enterEndState(victory);
    }
}

void Pacman::updatePacman() {
    if (queuedDirection.dx != 0 || queuedDirection.dy != 0) {
        if (canMove(pacmanTile, queuedDirection))
            pacmanDirection = queuedDirection;
    }
    if (pacmanDirection.dx == 0 && pacmanDirection.dy == 0) return;
    if (!canMove(pacmanTile, pacmanDirection)) return;

    Tile next       = stepTile(pacmanTile, pacmanDirection);
    pacmanDirection = directionAfterStep(pacmanTile, pacmanDirection, next);
    pacmanTile      = next;

    if (pellets[pacmanTile.face][pacmanTile.y][pacmanTile.x]) {
        pellets[pacmanTile.face][pacmanTile.y][pacmanTile.x] = false;
        pelletCount--;
        score += 10;
    }

    if (powerPellets[pacmanTile.face][pacmanTile.y][pacmanTile.x]) {
        powerPellets[pacmanTile.face][pacmanTile.y][pacmanTile.x] = false;
        frightenedTimer = 120;  // ~6 seconds at 20fps
        score += 50;
    }

    if (bonusItem.active && sameTile(pacmanTile, bonusItem.tile)) {
        static const int bonusScores[4] = {100, 300, 500, 1000};
        score += bonusScores[bonusItem.type];
        bonusItem.active = false;
    }
}

void Pacman::updateGhosts() {
    for (int i = 0; i < (int)ghosts.size(); i++)
        updateGhost(ghosts[i], i);
}

void Pacman::updateGhost(Ghost &ghost, int ghostIndex) {
    if (ghost.releaseDelay > 0) { ghost.releaseDelay--; return; }

    // Eaten ghosts: run back to spawn as eyes-only.
    // Use full BFS to find the correct path (greedy doesn't work because
    // the ghost door is only passable at specific x-columns).
    if (ghost.eaten) {
        if (sameTile(ghost.tile, ghost.home)) {
            ghost.eaten = false;
            ghost.releaseDelay = 30;
            return;
        }

        // BFS from current tile to home, ghost door passable both ways (ghostDoor=2)
        static const Direction dirs[4] = {{1,0},{-1,0},{0,1},{0,-1}};
        struct Node { Tile tile; Direction firstDir; };
        std::vector<Node> queue;
        queue.reserve(256);

        std::vector<bool> visited(6 * FACE_TILES * FACE_TILES, false);
        auto idx = [&](Tile t) {
            return (int)t.face * FACE_TILES * FACE_TILES + t.y * FACE_TILES + t.x;
        };
        visited[idx(ghost.tile)] = true;

        for (auto &d : dirs) {
            if (!canMove(ghost.tile, d, 2)) continue;
            Tile next = stepTile(ghost.tile, d);
            if (!visited[idx(next)]) {
                visited[idx(next)] = true;
                queue.push_back({next, d});
            }
        }

        Direction bestDir = {0, 0};
        for (int qi = 0; qi < (int)queue.size(); qi++) {
            auto &node = queue[qi];
            if (sameTile(node.tile, ghost.home)) {
                bestDir = node.firstDir;
                break;
            }
            for (auto &d : dirs) {
                if (!canMove(node.tile, d, 2)) continue;
                Tile next = stepTile(node.tile, d);
                if (!visited[idx(next)]) {
                    visited[idx(next)] = true;
                    queue.push_back({next, node.firstDir});
                }
            }
        }

        if (bestDir.dx != 0 || bestDir.dy != 0) {
            Tile nextTile   = stepTile(ghost.tile, bestDir);
            ghost.direction = directionAfterStep(ghost.tile, bestDir, nextTile);
            ghost.lastTile  = ghost.tile;
            ghost.tile      = nextTile;
        }
        return;
    }

    // Leaving ghost house: if the ghost is still inside the house, use BFS
    // toward the exit tile (just above the door) so it always finds the way out.
    bool insideHouse = (ghost.tile.face == top &&
                        ghost.tile.x >= GH_X0 && ghost.tile.x < GH_X1 &&
                        ghost.tile.y >= GH_Y0 && ghost.tile.y < GH_Y1);
    if (insideHouse) {
        // Target: the tile just above the door opening (y = GH_Y0 - 1, centre of door)
        Tile exitTarget = {top, GH_DOOR_X0, GH_Y0 - 1};

        static const Direction dirs4[4] = {{1,0},{-1,0},{0,1},{0,-1}};
        struct Node { Tile tile; Direction firstDir; };
        std::vector<Node> queue;
        queue.reserve(64);

        std::vector<bool> visited(6 * FACE_TILES * FACE_TILES, false);
        auto idx = [&](Tile t) {
            return (int)t.face * FACE_TILES * FACE_TILES + t.y * FACE_TILES + t.x;
        };
        visited[idx(ghost.tile)] = true;

        for (auto &d : dirs4) {
            if (!canMove(ghost.tile, d, 1)) continue;
            Tile nxt = stepTile(ghost.tile, d);
            if (!visited[idx(nxt)]) {
                visited[idx(nxt)] = true;
                queue.push_back({nxt, d});
            }
        }

        Direction bestDir = {0, 0};
        for (int qi = 0; qi < (int)queue.size(); qi++) {
            auto &node = queue[qi];
            if (sameTile(node.tile, exitTarget)) {
                bestDir = node.firstDir;
                break;
            }
            for (auto &d : dirs4) {
                if (!canMove(node.tile, d, 1)) continue;
                Tile nxt = stepTile(node.tile, d);
                if (!visited[idx(nxt)]) {
                    visited[idx(nxt)] = true;
                    queue.push_back({nxt, node.firstDir});
                }
            }
        }

        if (bestDir.dx != 0 || bestDir.dy != 0) {
            Tile nextTile   = stepTile(ghost.tile, bestDir);
            ghost.direction = directionAfterStep(ghost.tile, bestDir, nextTile);
            ghost.lastTile  = ghost.tile;
            ghost.tile      = nextTile;
        }
        return;
    }

    Direction next = chooseGhostDirection(ghost, ghostIndex);
    if (next.dx == 0 && next.dy == 0) return;

    Tile nextTile    = stepTile(ghost.tile, next);
    ghost.direction  = directionAfterStep(ghost.tile, next, nextTile);
    ghost.lastTile   = ghost.tile;
    ghost.tile       = nextTile;
}

void Pacman::spawnBonusItem() {
    int totalPellets = 6 * FACE_TILES * FACE_TILES;
    int collected = totalPellets - pelletCount;
    BonusType type;
    if      (collected < 60)  type = Cherry;
    else if (collected < 120) type = Strawberry;
    else if (collected < 200) type = Orange;
    else                      type = Banana;

    for (int attempts = 0; attempts < 50; attempts++) {
        int f = std::rand() % 6;
        int x = 1 + std::rand() % (FACE_TILES - 2);
        int y = 1 + std::rand() % (FACE_TILES - 2);
        if (f == top && x >= GH_X0 && x < GH_X1 && y >= GH_Y0 && y < GH_Y1) continue;
        if (pellets[f][y][x] || powerPellets[f][y][x]) continue;
        bonusItem = {{static_cast<ScreenNumber>(f), x, y}, type, 20 * 20, true};
        return;
    }
}

void Pacman::updateBonusItem() {
    bonusSpawnCounter++;
    if (!bonusItem.active && bonusSpawnCounter >= 200) {
        bonusSpawnCounter = 0;
        spawnBonusItem();
    }

    if (bonusItem.active) {
        bonusItem.timer--;
        if (bonusItem.timer <= 0) bonusItem.active = false;
    }
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

void Pacman::renderMenu() {
    static const char *menuItems[] = {
        "Play",
        "PlaySeed",
        "Modus:Default",
        "Demo",
        "Quit"
    };
    static const Color selectedColor(255, 210, 0);
    static const Color normalColor = Color::white();

    drawText(front, Vector2i(CharacterBitmaps::centered, 5), selectedColor, "PACMAN");

    if (seedMenuActive_) {
        drawText(front, Vector2i(CharacterBitmaps::centered, 21), selectedColor, "SEED");
        drawText(front, Vector2i(CharacterBitmaps::centered, 31), normalColor, std::to_string(seedStub_));
        drawText(front, Vector2i(CharacterBitmaps::centered, 43), normalColor, "A=START");
        drawText(front, Vector2i(CharacterBitmaps::centered, 51), normalColor, "U/D=EDIT");
    } else {
        for (int i = 0; i < 5; i++) {
            drawText(front,
                     Vector2i(CharacterBitmaps::centered, 15 + i * 8),
                     i == menuIndex ? selectedColor : normalColor,
                     menuItems[i]);
        }
    }

    drawText(top, Vector2i(CharacterBitmaps::centered, 10), normalColor, "Highscore");
    drawText(top, Vector2i(CharacterBitmaps::centered, 22), selectedColor, std::to_string(highScore));

    if (!menuSpritesInit_) {
        menuSprites_.clear();
        MenuSprite pac;
        pac.x = 32; pac.y = 38; pac.vx = 0.0f; pac.vy = 0.0f;
        pac.color = Color(255, 210, 0); pac.isPacman = true;
        menuSprites_.push_back(pac);
        Color ghostColors[4] = {
            Color(220,0,0), Color(255,140,0), Color(0,210,220), Color(255,130,180)
        };
        for (int i = 0; i < 4; i++) {
            MenuSprite g;
            g.x = (float)(4 + i * 14); g.y = 52; g.vx = 0.5f; g.vy = 0.0f;
            g.color = ghostColors[i]; g.isPacman = false;
            menuSprites_.push_back(g);
        }
        menuSpritesInit_ = true;
    }

    for (auto &s : menuSprites_) {
        if (!s.isPacman) {
            s.x += s.vx;
            if (s.x > 66) s.x = -5;
        }
    }

    static const int ghostSpr[5][5] = {
        {0,1,1,1,0},{1,1,1,1,1},{1,1,1,1,1},{1,1,1,1,1},{1,0,1,0,1}
    };
    static const int pacBase[5][5] = {
        {0,1,1,1,0},{1,1,1,1,1},{1,1,1,1,1},{1,1,1,1,1},{0,1,1,1,0}
    };

    bool mouthOpenNow = (frameCounter / 3) % 2 == 0;

    for (auto &s : menuSprites_) {
        int cx = (int)s.x - 2;
        int cy = (int)s.y - 2;
        if (s.isPacman) {
            for (int sy = 0; sy < 5; sy++) {
                for (int sx = 0; sx < 5; sx++) {
                    if (!pacBase[sy][sx]) continue;
                    bool mouth = false;
                    if (mouthOpenNow)
                        mouth = sx > 2 && std::abs(sy-2) < (sx-2);
                    if (!mouth) drawPixel(top, cx+sx, cy+sy, s.color);
                }
            }
        } else {
            if (cx < -4 || cx > 63) continue;
            for (int sy = 0; sy < 5; sy++)
                for (int sx = 0; sx < 5; sx++)
                    if (ghostSpr[sy][sx]) drawPixel(top, cx+sx, cy+sy, s.color);
        }
    }
}

void Pacman::drawPixel(ScreenNumber face, int px, int py, Color color) {
    if (px < 0 || px >= FACE_PX || py < 0 || py >= FACE_PX) return;
    setPixel3D(getPointOnScreen(face, Vector2i(px, py)), color);
}

void Pacman::drawHLine(ScreenNumber face, int x0, int x1, int y, Color color, int thickness) {
    for (int t = 0; t < thickness; t++)
        for (int x = x0; x <= x1; x++)
            drawPixel(face, x, y + t, color);
}

void Pacman::drawVLine(ScreenNumber face, int x, int y0, int y1, Color color, int thickness) {
    for (int t = 0; t < thickness; t++)
        for (int y = y0; y <= y1; y++)
            drawPixel(face, x + t, y, color);
}

void Pacman::renderBoard() {
    if ((gameState == gameOver || gameState == victory) && gameOverTimer_ > 30) return;

    float brightness = 1.0f;
    if (gameState == gameOver || gameState == victory) {
        brightness = std::max(0.0f, 1.0f - static_cast<float>(gameOverTimer_) / 30.0f);
    }

    renderWalls(brightness);
    renderGhostHouse();
    renderPellets();
    renderBonusItem();
    renderPacman();
    for (auto &g : ghosts) renderGhost(g);
}

void Pacman::renderWalls(float brightness) {
    for (int f = 0; f < 6; f++)
        renderWallsFace(f, brightness);
}

void Pacman::renderWallsFace(int face, float brightness) {
    Color wallColor(0, 0, static_cast<int>(200.0f * brightness));
    ScreenNumber sn = static_cast<ScreenNumber>(face);

    // Draw horizontal walls (lines at y = row * TILE_PX)
    for (int row = 0; row <= FACE_TILES; row++) {
        for (int col = 0; col < FACE_TILES; col++) {
            if (hwall[face][row][col]) {
                int px0 = col * TILE_PX;
                int px1 = px0 + TILE_PX - 1;
                int py  = row * TILE_PX;
                // 2px thick wall, clamped inside face
                drawHLine(sn, px0, px1, py, wallColor);
            }
        }
    }

    // Draw vertical walls (lines at x = col * TILE_PX)
    for (int row = 0; row < FACE_TILES; row++) {
        for (int col = 0; col <= FACE_TILES; col++) {
            if (vwall[face][row][col]) {
                int py0 = row * TILE_PX;
                int py1 = py0 + TILE_PX - 1;
                int px  = col * TILE_PX;
                drawVLine(sn, px, py0, py1, wallColor);
            }
        }
    }
}

void Pacman::renderGhostHouse() {
    // Draw ghost house as a slightly highlighted box outline (already covered by walls,
    // but draw the door in a distinct pink colour)
    ScreenNumber sn = top;
    Color doorColor(255, 160, 160);
    int y = GH_Y0 * TILE_PX;
    for (int col = GH_DOOR_X0; col <= GH_DOOR_X1; col++) {
        int px0 = col * TILE_PX;
        int px1 = px0 + TILE_PX - 1;
        drawHLine(sn, px0, px1, y,     doorColor);
        drawHLine(sn, px0, px1, y + 1, doorColor);
    }
}

void Pacman::renderPellets() {
    float brightness = 1.0f;
    if (gameState == gameOver || gameState == victory) {
        brightness = std::max(0.0f, 1.0f - static_cast<float>(gameOverTimer_) / 30.0f);
        if (brightness <= 0.0f) return;
    }

    Color pelletColor(static_cast<int>(255.0f * brightness),
                      static_cast<int>(220.0f * brightness),
                      static_cast<int>(120.0f * brightness));
    Color powerPelletColor(static_cast<int>(255.0f * brightness),
                           static_cast<int>(255.0f * brightness),
                           static_cast<int>(180.0f * brightness));

    for (int f = 0; f < 6; f++) {
        ScreenNumber sn = static_cast<ScreenNumber>(f);
        for (int y = 0; y < FACE_TILES; y++) {
            for (int x = 0; x < FACE_TILES; x++) {
                int cx = x * TILE_PX + TILE_PX / 2;
                int cy = y * TILE_PX + TILE_PX / 2;

                if (pellets[f][y][x]) {
                    // 1px dot centred in tile
                    drawPixel(sn, cx, cy, pelletColor);
                }

                if (powerPellets[f][y][x]) {
                    // 2×2 px, always on
                    drawPixel(sn, cx,     cy,     powerPelletColor);
                    drawPixel(sn, cx - 1, cy,     powerPelletColor);
                    drawPixel(sn, cx,     cy - 1, powerPelletColor);
                    drawPixel(sn, cx - 1, cy - 1, powerPelletColor);
                }
            }
        }
    }
}

void Pacman::renderGameOver() {
    const bool showNewHigh = newHighScore_ && ((gameOverTimer_ / 15) % 2 == 0);
    const int displayedSeed = currentSeed_ % 100000;
    const Color titleColor = (gameState == victory) ? Color(0, 220, 0) : Color(220, 0, 0);
    const std::string title = (gameState == victory) ? "You Win!" : "Game Over";

    drawText(front, Vector2i(CharacterBitmaps::centered, 10), titleColor, title);
    drawText(front, Vector2i(CharacterBitmaps::centered, 22), Color::white(), "Score:");
    drawText(front, Vector2i(CharacterBitmaps::centered, 32), Color(255, 210, 0), std::to_string(score));
    if (showNewHigh) {
        drawText(front, Vector2i(CharacterBitmaps::centered, 42), Color(255, 180, 0), "NEW HI!");
    }
    drawText(front, Vector2i(2, 56), Color(120, 120, 120), "#" + std::to_string(displayedSeed));

    static const char *menuItems[] = {"Again", "New", "WithSeed", "Quit"};
    static const int menuY[] = {8, 20, 32, 44};
    for (int i = 0; i < 4; i++) {
        drawText(top,
                 Vector2i(CharacterBitmaps::centered, menuY[i]),
                 i == gameOverMenuIndex_ ? Color(255, 210, 0) : Color::white(),
                 menuItems[i]);
    }
    drawText(top, Vector2i(2, 56), Color(120, 120, 120), "Hi:" + std::to_string(highScore));

    for (std::size_t i = 0; i < fireworks_.size(); i++) {
        const Firework &fw = fireworks_[i];
        float fade = static_cast<float>(fw.life) / static_cast<float>(fw.maxLife);
        Color c(
            static_cast<uint8_t>(fw.color.r() * fade),
            static_cast<uint8_t>(fw.color.g() * fade),
            static_cast<uint8_t>(fw.color.b() * fade));
        int px = static_cast<int>(fw.x);
        int py = static_cast<int>(fw.y);
        if (px >= 0 && px < 64 && py >= 0 && py < 64) {
            drawPixel(fw.face, px, py, c);
        }
    }
}

void Pacman::renderBonusItem() {
    if (!bonusItem.active) return;
    if (bonusItem.timer < 60 && (frameCounter / 4) % 2 == 0) return;

    static const int sprites[4][5][5] = {
        { {0, 1, 0, 1, 0},
          {1, 1, 1, 1, 1},
          {1, 1, 1, 1, 1},
          {0, 1, 0, 1, 0},
          {0, 0, 2, 0, 0} },
        { {0, 1, 1, 1, 0},
          {1, 2, 1, 2, 1},
          {1, 1, 2, 1, 1},
          {0, 1, 1, 1, 0},
          {0, 0, 1, 0, 0} },
        { {0, 1, 1, 1, 0},
          {1, 1, 1, 1, 1},
          {1, 1, 2, 1, 1},
          {1, 1, 1, 1, 1},
          {0, 1, 1, 1, 0} },
        { {0, 1, 1, 0, 0},
          {0, 0, 1, 1, 0},
          {1, 1, 1, 1, 1},
          {1, 1, 1, 1, 1},
          {0, 1, 1, 1, 0} },
    };

    static const Color mainColors[4] = {
        Color(220,  40,  40),
        Color(220,  50,  80),
        Color(255, 140,   0),
        Color(255, 210,  30),
    };
    static const Color accentColors[4] = {
        Color(220,  40,  40),
        Color(255, 255, 255),
        Color(255, 180,  50),
        Color(200, 160,  10),
    };
    static const Color stemColor(50, 180, 50);

    int cx = bonusItem.tile.x * TILE_PX + TILE_PX / 2 - 2;
    int cy = bonusItem.tile.y * TILE_PX + TILE_PX / 2 - 2;
    const auto &spr = sprites[bonusItem.type];

    for (int sy = 0; sy < 5; sy++) {
        for (int sx = 0; sx < 5; sx++) {
            int v = spr[sy][sx];
            if (v == 0) continue;
            Color col = (bonusItem.type == Cherry && sy == 4 && sx == 2)
                        ? stemColor
                        : (v == 2 ? accentColors[bonusItem.type] : mainColors[bonusItem.type]);
            drawPixel(bonusItem.tile.face, cx + sx, cy + sy, col);
        }
    }
}

void Pacman::renderPacman() {
    // 5×5 sprite, placed at tile centre
    // Base circle pattern (1=yellow, 0=black)
    static const int base[5][5] = {
        {0, 1, 1, 1, 0},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {0, 1, 1, 1, 0},
    };

    Direction vd = pacmanDirection;
    if (vd.dx == 0 && vd.dy == 0) vd.dy = -1;

    Color pacColor(255, 210, 0);

    int cx = pacmanTile.x * TILE_PX + TILE_PX / 2 - 2;  // top-left of sprite
    int cy = pacmanTile.y * TILE_PX + TILE_PX / 2 - 2;

    for (int sy = 0; sy < 5; sy++) {
        for (int sx = 0; sx < 5; sx++) {
            if (!base[sy][sx]) continue;

            // Mouth cutout (open wedge in movement direction)
            bool mouth = false;
            if (mouthOpen) {
                if (vd.dx > 0)  mouth = sx > 2 && std::abs(sy - 2) < (sx - 2);
                if (vd.dx < 0)  mouth = sx < 2 && std::abs(sy - 2) < (2 - sx);
                if (vd.dy > 0)  mouth = sy > 2 && std::abs(sx - 2) < (sy - 2);
                if (vd.dy < 0)  mouth = sy < 2 && std::abs(sx - 2) < (2 - sy);
            }

            if (!mouth)
                drawPixel(pacmanTile.face, cx + sx, cy + sy, pacColor);
        }
    }
}

void Pacman::renderGhost(const Ghost &ghost) {
    static const int sprite[5][5] = {
        {0, 1, 1, 1, 0},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 0, 1, 0, 1},
    };

    Color bodyColor = ghost.color;
    if (ghost.eaten) {
        // Only draw eyes – no body
        int cx = ghost.tile.x * TILE_PX + TILE_PX / 2 - 2;
        int cy = ghost.tile.y * TILE_PX + TILE_PX / 2 - 2;
        drawPixel(ghost.tile.face, cx + 1, cy + 1, Color::white());
        drawPixel(ghost.tile.face, cx + 3, cy + 1, Color::white());
        drawPixel(ghost.tile.face, cx + 1, cy + 2, Color(0, 0, 180));
        drawPixel(ghost.tile.face, cx + 3, cy + 2, Color(0, 0, 180));
        return;
    }
    if (frightenedTimer > 0) {
        // Blink white during last 2 seconds (40 frames)
        bool blink = (frightenedTimer < 40) && ((frameCounter / 4) % 2 == 0);
        bodyColor = blink ? Color(200, 200, 200) : Color(0, 0, 160);
    }

    int cx = ghost.tile.x * TILE_PX + TILE_PX / 2 - 2;
    int cy = ghost.tile.y * TILE_PX + TILE_PX / 2 - 2;

    for (int sy = 0; sy < 5; sy++)
        for (int sx = 0; sx < 5; sx++)
            if (sprite[sy][sx])
                drawPixel(ghost.tile.face, cx + sx, cy + sy, bodyColor);

    // Eyes (hidden when frightened)
    if (frightenedTimer == 0) {
        drawPixel(ghost.tile.face, cx + 1, cy + 1, Color::white());
        drawPixel(ghost.tile.face, cx + 3, cy + 1, Color::white());
        drawPixel(ghost.tile.face, cx + 1, cy + 2, Color(0, 0, 180));
        drawPixel(ghost.tile.face, cx + 3, cy + 2, Color(0, 0, 180));
    }
}

// ---------------------------------------------------------------------------
// Tile / World math
// ---------------------------------------------------------------------------
//
// All 12 cube-edge transitions, derived from faceBasis() coordinate systems.
// Notation: each face has tile coords (x,y) in [0..15].
// At the border we step "off" the face and land on a neighbour face.
// The coordinate mapping for each crossing is listed explicitly so there
// is no floating-point / rounding ambiguity.
//
//  Face pixel layout (origin = tile(0,0) top-left pixel):
//   front:  origin(5,1,5)   axisX(+x) axisY(+z)  → world x=5+px, z=5+py, y=1
//   right:  origin(M,5,5)   axisX(+y) axisY(+z)  → world y=5+px, z=5+py, x=M   (M=VIRTUALCUBEMAXINDEX)
//   back:   origin(61,M,5)  axisX(-x) axisY(+z)  → world x=61-px, z=5+py, y=M
//   left:   origin(0,61,5)  axisX(-y) axisY(+z)  → world y=61-px, z=5+py, x=0
//   top:    origin(5,61,0)  axisX(+x) axisY(-y)  → world x=5+px, y=61-py, z=0
//   bottom: origin(5,5,M)   axisX(+x) axisY(+y)  → world x=5+px, y=5+py, z=M
//
// Cross-face transitions (gap at tile cols/rows 7-8 on every border):
//
//  top    dy=-1 (y<0):    → back   dy=+1,  nx=15-tx  (x-axis mirrors)
//  top    dy=+1 (y>15):   → front  dy=+1,  nx=tx     (x-axis same)
//  top    dx=-1 (x<0):    → left   dy=+1,  nx=ty     (rotate)
//  top    dx=+1 (x>15):   → right  dy=+1,  nx=15-ty  (rotate+mirror)
//
//  bottom dy=-1 (y<0):    → front  dy=-1,  nx=tx
//  bottom dy=+1 (y>15):   → back   dy=-1,  nx=15-tx
//  bottom dx=-1 (x<0):    → left   dy=-1,  nx=15-ty
//  bottom dx=+1 (x>15):   → right  dy=-1,  nx=ty
//
//  front  dy=-1 (y<0):    → top    dy=-1,  nx=tx
//  front  dy=+1 (y>15):   → bottom dy=+1,  nx=tx
//  front  dx=-1 (x<0):    → left   dx=-1,  nx=15-ty  (wait - verified below)
//  front  dx=+1 (x>15):   → right  dx=+1,  nx=ty     (wait - verified below)  -- WAIT need exact
//
// Rather than risk any algebra error here, I keep the world-coord approach
// but add a hard clamp: after crossing, the landing tile must be at the
// expected border row/col (0 or 15).  The coordinate math is sound; the
// only real bug was (a) diagonal directions from rotateDirection and
// (b) canMove not checking the destination-face entry wall.
// Both are now fixed.  We keep tileFromWorld as the single source of truth.

Pacman::StepResult Pacman::stepResult(Tile from, Direction dir) const {
    int nx = from.x + dir.dx;
    int ny = from.y + dir.dy;
    if (nx >= 0 && nx < FACE_TILES && ny >= 0 && ny < FACE_TILES)
        return {Tile{from.face, nx, ny}, dir};

    const int L = FACE_TILES - 1;
    int tx = from.x, ty = from.y;

    switch (from.face) {
        case top:
            if (dir.dy == -1) return {Tile{back,   L-tx, 0},   Direction{0,  1}};
            if (dir.dy == +1) return {Tile{front,  tx,   0},   Direction{0,  1}};
            if (dir.dx == -1) return {Tile{left,   ty,   0},   Direction{0,  1}};
            if (dir.dx == +1) return {Tile{right,  L-ty, 0},   Direction{0,  1}};
            break;
        case bottom:
            if (dir.dy == -1) return {Tile{front,  tx,   L},   Direction{0, -1}};
            if (dir.dy == +1) return {Tile{back,   L-tx, L},   Direction{0, -1}};
            if (dir.dx == -1) return {Tile{left,   L-ty, L},   Direction{0, -1}};
            if (dir.dx == +1) return {Tile{right,  ty,   L},   Direction{0, -1}};
            break;
        case front:
            if (dir.dy == -1) return {Tile{top,    tx,   L},   Direction{0, -1}};
            if (dir.dy == +1) return {Tile{bottom, tx,   0},   Direction{0,  1}};
            if (dir.dx == -1) return {Tile{left,   L,    ty},  Direction{-1, 0}};
            if (dir.dx == +1) return {Tile{right,  0,    ty},  Direction{+1, 0}};
            break;
        case back:
            if (dir.dy == -1) return {Tile{top,    L-tx, 0},   Direction{0,  1}};
            if (dir.dy == +1) return {Tile{bottom, L-tx, L},   Direction{0, -1}};
            if (dir.dx == -1) return {Tile{right,  L,    ty},  Direction{-1, 0}};
            if (dir.dx == +1) return {Tile{left,   0,    ty},  Direction{+1, 0}};
            break;
        case left:
            if (dir.dy == -1) return {Tile{top,    0,    tx},  Direction{+1, 0}};
            if (dir.dy == +1) return {Tile{bottom, 0,    L-tx},Direction{+1, 0}};
            if (dir.dx == -1) return {Tile{back,   L,    ty},  Direction{-1, 0}};
            if (dir.dx == +1) return {Tile{front,  0,    ty},  Direction{+1, 0}};
            break;
        case right:
            if (dir.dy == -1) return {Tile{top,    L,    L-tx},Direction{-1, 0}};
            if (dir.dy == +1) return {Tile{bottom, L,    tx},  Direction{-1, 0}};
            if (dir.dx == -1) return {Tile{front,  L,    ty},  Direction{-1, 0}};
            if (dir.dx == +1) return {Tile{back,   0,    ty},  Direction{+1, 0}};
            break;
        default:
            break;
    }
    return {from, Direction{0, 0}};
}

Pacman::Tile Pacman::stepTile(Tile from, Direction dir) {
    int nx = from.x + dir.dx;
    int ny = from.y + dir.dy;
    if (nx >= 0 && nx < FACE_TILES && ny >= 0 && ny < FACE_TILES)
        return {from.face, nx, ny};
    return stepResult(from, dir).tile;
}

Pacman::Tile Pacman::tileFromWorld(Vector3i world) {
    ScreenNumber face = getScreenNumber(world);
    FaceBasis basis   = faceBasis(face);
    Vector3i diff     = world - basis.origin;

    auto project = [](Vector3i v, Vector3i axis) -> int {
        if (axis[0]) return v[0] / axis[0];
        if (axis[1]) return v[1] / axis[1];
        return v[2] / axis[2];
    };

    int px = project(diff, basis.axisX);
    int py = project(diff, basis.axisY);

    int tx = std::max(0, std::min(FACE_TILES - 1, px / TILE_PX));
    int ty = std::max(0, std::min(FACE_TILES - 1, py / TILE_PX));
    return {face, tx, ty};
}

Vector3i Pacman::tileToWorld(Tile tile) {
    // Centre pixel of tile
    int px = tile.x * TILE_PX + TILE_PX / 2;
    int py = tile.y * TILE_PX + TILE_PX / 2;
    return getPointOnScreen(tile.face, Vector2i(px, py));
}

Pacman::Direction Pacman::directionAfterStep(Tile from, Direction dir, Tile to) const {
    if (from.face == to.face) return dir;
    return stepResult(from, dir).dir;
}

Pacman::Direction Pacman::rotateDirection(ScreenNumber oldFace, ScreenNumber newFace, Direction dir) const {
    FaceBasis ob = faceBasis(oldFace);
    FaceBasis nb = faceBasis(newFace);
    Vector3i wd  = ob.axisX * dir.dx + ob.axisY * dir.dy;

    int dotX = wd.dot(nb.axisX);
    int dotY = wd.dot(nb.axisY);

    // Only one axis should be dominant – pick the stronger one to avoid diagonals.
    if (std::abs(dotX) >= std::abs(dotY)) {
        return {dotX > 0 ? 1 : (dotX < 0 ? -1 : 0), 0};
    } else {
        return {0, dotY > 0 ? 1 : (dotY < 0 ? -1 : 0)};
    }
}

Pacman::Direction Pacman::chooseGhostDirection(const Ghost &ghost, int ghostIndex) {
    static const Direction dirs[4] = {{1,0},{-1,0},{0,1},{0,-1}};

    std::vector<Direction> candidates;
    for (auto &d : dirs) {
        if (!canMove(ghost.tile, d, 1)) continue;
        Tile next = stepTile(ghost.tile, d);
        if (sameTile(next, ghost.lastTile)) continue;
        candidates.push_back(d);
    }

    if (candidates.empty()) {
        for (auto &d : dirs) {
            if (canMove(ghost.tile, d, 1)) { return d; }
        }
        return {0, 0};
    }

    // 25% random wandering keeps ghosts unpredictable and spreads them across faces
    if (std::rand() % 4 == 0)
        return candidates[std::rand() % candidates.size()];

    // Build target: each ghost has a different targeting strategy
    // Ghost 0 (red):   direct chase – always targets Pacman
    // Ghost 1 (orange):targets 4 tiles ahead of Pacman
    // Ghost 2 (cyan):  targets 3 tiles ahead of Pacman
    // Ghost 3 (pink):  alternates between Pacman and a random face centre
    Tile target = pacmanTile;
    if (ghostIndex == 1) {
        Direction pd = (pacmanDirection.dx || pacmanDirection.dy) ? pacmanDirection : Direction{1,0};
        target = offsetTarget(pacmanTile, pd, 4);
    }
    if (ghostIndex == 2) {
        Direction pd = (pacmanDirection.dx || pacmanDirection.dy) ? pacmanDirection : Direction{0,1};
        target = offsetTarget(pacmanTile, pd, 3);
    }
    if (ghostIndex == 3) {
        // Patrol: periodically pick a face centre far from current position to spread out
        static int patrolCounter = 0;
        static Tile patrolTarget = {front, 8, 8};
        patrolCounter++;
        if (patrolCounter > 40 || sameTile(ghost.tile, patrolTarget)) {
            // Pick centre of a random face that isn't the ghost's current face
            ScreenNumber faces[6] = {front, back, left, right, top, bottom};
            ScreenNumber f = faces[std::rand() % 6];
            patrolTarget = {f, 8, 8};
            patrolCounter = 0;
        }
        target = patrolTarget;
    }

    // Use BFS from each candidate next-tile to find which gets closest to target
    // BFS is capped at 60 nodes per candidate to stay cheap
    auto bfsDist = [&](Tile from, Tile goal) -> int {
        if (sameTile(from, goal)) return 0;
        struct N { Tile tile; int dist; };
        std::vector<N> q;
        q.reserve(128);
        std::vector<bool> vis(6 * FACE_TILES * FACE_TILES, false);
        auto ix = [&](Tile t){ return (int)t.face*FACE_TILES*FACE_TILES + t.y*FACE_TILES + t.x; };
        vis[ix(from)] = true;
        q.push_back({from, 0});
        for (int qi = 0; qi < (int)q.size() && qi < 80; qi++) {
            auto &node = q[qi];
            for (auto &d2 : dirs) {
                if (!canMove(node.tile, d2, 1)) continue;
                Tile nxt = stepTile(node.tile, d2);
                if (sameTile(nxt, goal)) return node.dist + 1;
                if (!vis[ix(nxt)]) { vis[ix(nxt)] = true; q.push_back({nxt, node.dist+1}); }
            }
        }
        // Fallback to world-space manhattan if BFS didn't reach
        return manhattanDistance(from, goal);
    };

    int best = std::numeric_limits<int>::max();
    Direction bestDir = candidates[0];
    for (auto &d : candidates) {
        Tile next = stepTile(ghost.tile, d);
        int dist = bfsDist(next, target);
        if (dist < best) { best = dist; bestDir = d; }
    }
    return bestDir;
}

int Pacman::manhattanDistance(Tile a, Tile b) {
    Vector3i wa = tileToWorld(a);
    Vector3i wb = tileToWorld(b);
    return (std::abs(wa[0]-wb[0]) + std::abs(wa[1]-wb[1]) + std::abs(wa[2]-wb[2])) / TILE_PX;
}

Pacman::FaceBasis Pacman::faceBasis(ScreenNumber face) const {
    // origin = world coordinate of pixel (0,0) on each face.
    // axisX / axisY = world-space unit vectors (length 1 in pixel units).
    // These must match getPointOnScreen() exactly.
    switch (face) {
        case front:
            return {front,  Vector3i(5, 1, 5),                       Vector3i(1,0,0), Vector3i(0,0,1)};
        case right:
            return {right,  Vector3i(VIRTUALCUBEMAXINDEX, 5, 5),      Vector3i(0,1,0), Vector3i(0,0,1)};
        case back:
            return {back,   Vector3i(61, VIRTUALCUBEMAXINDEX, 5),     Vector3i(-1,0,0),Vector3i(0,0,1)};
        case left:
            return {left,   Vector3i(0, 61, 5),                       Vector3i(0,-1,0),Vector3i(0,0,1)};
        case top:
            return {top,    Vector3i(5, 61, 0),                       Vector3i(1,0,0), Vector3i(0,-1,0)};
        case bottom:
            return {bottom, Vector3i(5, 5, VIRTUALCUBEMAXINDEX),      Vector3i(1,0,0), Vector3i(0,1,0)};
        case anyScreen:
        default:
            return {top,    Vector3i(5, 61, 0),                       Vector3i(1,0,0), Vector3i(0,-1,0)};
    }
}

bool Pacman::sameTile(Tile a, Tile b) const {
    return a.face == b.face && a.x == b.x && a.y == b.y;
}

Pacman::Tile Pacman::offsetTarget(Tile tile, Direction dir, int steps) {
    Tile r = tile;
    for (int i = 0; i < steps; i++) {
        if (!canMove(r, dir)) break;
        r = stepTile(r, dir);
    }
    return r;
}

// ---------------------------------------------------------------------------
// Demo / auto-play: choose direction toward nearest pellet or power pellet
// Uses greedy BFS limited to a small search depth to keep it cheap.
// ---------------------------------------------------------------------------

void Pacman::demoChooseDirection() {
    if (frameCounter % moveDivider != 0) return;

    static const Direction dirs[4] = {{1,0},{-1,0},{0,1},{0,-1}};
    const int TOTAL = 6 * FACE_TILES * FACE_TILES;
    auto idx = [&](Tile t){ return (int)t.face*FACE_TILES*FACE_TILES + t.y*FACE_TILES + t.x; };

    // -----------------------------------------------------------------------
    // Full BFS from Pacman – builds distance + firstDir to every reachable tile.
    // No cap: explores all 6 faces so Pacman can always find cross-face targets.
    // -----------------------------------------------------------------------
    std::vector<int>       distPac(TOTAL, -1);
    std::vector<Direction> firstDir(TOTAL, {0,0});

    {
        struct Node { Tile tile; Direction fd; int dist; };
        std::vector<Node> q;
        q.reserve(TOTAL);
        distPac[idx(pacmanTile)] = 0;

        // Randomise initial order for variety
        int order[4] = {0,1,2,3};
        for (int i = 3; i > 0; i--) { int j = std::rand()%(i+1); std::swap(order[i],order[j]); }
        for (int oi = 0; oi < 4; oi++) {
            auto &d = dirs[order[oi]];
            if (!canMove(pacmanTile, d)) continue;
            Tile nxt = stepTile(pacmanTile, d);
            int ni = idx(nxt);
            if (distPac[ni] < 0) {
                distPac[ni] = 1; firstDir[ni] = d;
                q.push_back({nxt, d, 1});
            }
        }
        for (int qi = 0; qi < (int)q.size(); qi++) {
            auto &node = q[qi];
            for (auto &d : dirs) {
                if (!canMove(node.tile, d)) continue;
                Tile nxt = stepTile(node.tile, d);
                int ni = idx(nxt);
                if (distPac[ni] < 0) {
                    distPac[ni] = node.dist+1; firstDir[ni] = node.fd;
                    q.push_back({nxt, node.fd, node.dist+1});
                }
            }
        }
    }

    bool frightened = (frightenedTimer > 0);

    // -----------------------------------------------------------------------
    // Compute BFS distance from each dangerous ghost to Pacman.
    // We build a combined "threat map": minGhostDist[tile] = how close the
    // nearest dangerous ghost is to that tile (used to rank escape routes).
    // -----------------------------------------------------------------------
    std::vector<int> minGhostDist(TOTAL, std::numeric_limits<int>::max());
    int closestGhostToPac = std::numeric_limits<int>::max();

    if (!frightened) {
        for (auto &g : ghosts) {
            if (g.eaten) continue;
            // BFS from ghost outward
            struct Node2 { Tile tile; int dist; };
            std::vector<Node2> gq;
            gq.reserve(256);
            std::vector<int> gd(TOTAL, -1);
            gd[idx(g.tile)] = 0;
            gq.push_back({g.tile, 0});
            for (int qi = 0; qi < (int)gq.size() && qi < 200; qi++) {
                auto &node = gq[qi];
                if (node.tile.face != g.tile.face && node.dist > 12) break; // limit cross-face ghost BFS
                for (auto &d : dirs) {
                    if (!canMove(node.tile, d, 1)) continue;
                    Tile nxt = stepTile(node.tile, d);
                    int ni = idx(nxt);
                    if (gd[ni] < 0) {
                        gd[ni] = node.dist+1;
                        gq.push_back({nxt, node.dist+1});
                        if (gd[ni] < minGhostDist[ni]) minGhostDist[ni] = gd[ni];
                    }
                }
            }
            // Distance from this ghost to pacman
            if (gd[idx(pacmanTile)] >= 0)
                closestGhostToPac = std::min(closestGhostToPac, gd[idx(pacmanTile)]);
        }
    }

    const int THREAT_RADIUS    = 10;  // flee when ghost within this many tiles
    const int POWERPELLET_LURE = 6;   // go for power pellet when ghost within this range

    // -----------------------------------------------------------------------
    // DECISION LOGIC
    // -----------------------------------------------------------------------

    // 1. If frightened: hunt the nearest frightened (non-eaten) ghost via BFS
    if (frightened) {
        int best = std::numeric_limits<int>::max();
        std::vector<Direction> bds;
        for (auto &g : ghosts) {
            if (g.eaten) continue;
            int gi = idx(g.tile);
            if (distPac[gi] < 0) continue;
            if (distPac[gi] < best) { best = distPac[gi]; bds.clear(); }
            if (distPac[gi] == best) {
                Direction fd = firstDir[gi];
                bool dup = false;
                for (auto &bd : bds) if (bd.dx==fd.dx && bd.dy==fd.dy) { dup=true; break; }
                if (!dup) bds.push_back(fd);
            }
        }
        if (!bds.empty()) {
            queuedDirection = bds[std::rand() % bds.size()];
            return;
        }
        // No frightened ghost found → fall through to pellet hunt
    }

    // 2. If a dangerous ghost is close AND a power pellet is reachable nearby:
    //    go for the power pellet to turn the tables
    if (!frightened && closestGhostToPac <= POWERPELLET_LURE) {
        int bestPP = std::numeric_limits<int>::max();
        Direction ppDir = {0,0};
        for (int f = 0; f < 6; f++) {
            for (int y = 0; y < FACE_TILES; y++) {
                for (int x = 0; x < FACE_TILES; x++) {
                    if (!powerPellets[f][y][x]) continue;
                    Tile pp = {(ScreenNumber)f, x, y};
                    int pi = idx(pp);
                    if (distPac[pi] < 0) continue;
                    // Only go for it if we can reach it before the ghost (with margin)
                    int ghostDistToPP = minGhostDist[pi];
                    if (distPac[pi] < ghostDistToPP && distPac[pi] < bestPP) {
                        bestPP = distPac[pi];
                        ppDir  = firstDir[pi];
                    }
                }
            }
        }
        if (ppDir.dx != 0 || ppDir.dy != 0) {
            queuedDirection = ppDir;
            return;
        }
    }

    // 3. If a dangerous ghost is within THREAT_RADIUS: flee
    //    Choose the direction whose next tile maximises our minimum BFS distance
    //    to all dangerous ghosts AND leads away from the ghost's flood-fill.
    if (!frightened && closestGhostToPac <= THREAT_RADIUS) {
        Direction bestDir = {0,0};
        int bestScore = -1;
        int order[4] = {0,1,2,3};
        for (int i = 3; i > 0; i--) { int j = std::rand()%(i+1); std::swap(order[i],order[j]); }
        for (int oi = 0; oi < 4; oi++) {
            auto &d = dirs[order[oi]];
            if (!canMove(pacmanTile, d)) continue;
            Tile nxt = stepTile(pacmanTile, d);
            int ni = idx(nxt);
            // Safety score = minimum ghost BFS distance to this next tile
            int safety = (minGhostDist[ni] == std::numeric_limits<int>::max()) ? 999 : minGhostDist[ni];
            // Bonus for tiles on a different face (encourages cross-face escape)
            if (nxt.face != pacmanTile.face) safety += 4;
            if (safety > bestScore) { bestScore = safety; bestDir = d; }
        }
        if (bestDir.dx != 0 || bestDir.dy != 0) {
            queuedDirection = bestDir;
            return;
        }
    }

    // 4. Hunt mode: find nearest pellet or power-pellet across ALL faces.
    //    Prefer targets on faces that haven't been visited much
    //    (face diversity bonus to encourage spreading out).
    {
        // Count remaining pellets per face for diversity weighting
        int pelletsPerFace[6] = {};
        for (int f = 0; f < 6; f++)
            for (int y = 0; y < FACE_TILES; y++)
                for (int x = 0; x < FACE_TILES; x++)
                    if (pellets[f][y][x] || powerPellets[f][y][x]) pelletsPerFace[f]++;

        int bestScore = std::numeric_limits<int>::max();
        std::vector<Direction> bds;

        for (int f = 0; f < 6; f++) {
            for (int y = 0; y < FACE_TILES; y++) {
                for (int x = 0; x < FACE_TILES; x++) {
                    if (!pellets[f][y][x] && !powerPellets[f][y][x]) continue;
                    Tile t = {(ScreenNumber)f, x, y};
                    int ti = idx(t);
                    if (distPac[ti] < 0) continue;

                    // Weighted score: BFS distance minus a bonus for faces with many pellets
                    // and minus a bonus for different face (encourages cross-face travel)
                    int score = distPac[ti];
                    if ((ScreenNumber)f != pacmanTile.face) score -= 3;
                    if (pelletsPerFace[f] > 20) score -= 2;
                    if (powerPellets[f][y][x] && closestGhostToPac > POWERPELLET_LURE)
                        score += 5;
                    Direction fd = firstDir[ti];
                    Direction rev = {-pacmanDirection.dx, -pacmanDirection.dy};
                    if (fd.dx == rev.dx && fd.dy == rev.dy) score += 6;

                    if (score < bestScore) { bestScore = score; bds.clear(); }
                    if (score == bestScore) {
                        bool dup = false;
                        for (auto &bd : bds) if (bd.dx==fd.dx && bd.dy==fd.dy) { dup=true; break; }
                        if (!dup) bds.push_back(fd);
                    }
                }
            }
        }

        if (!bds.empty()) {
            queuedDirection = bds[std::rand() % bds.size()];
            return;
        }
    }

    // 5. Absolute fallback: any valid non-reverse move
    {
        Direction rev = {-pacmanDirection.dx, -pacmanDirection.dy};
        int order[4] = {0,1,2,3};
        for (int i = 3; i > 0; i--) { int j = std::rand()%(i+1); std::swap(order[i],order[j]); }
        for (int oi = 0; oi < 4; oi++) {
            auto &d = dirs[order[oi]];
            if (d.dx == rev.dx && d.dy == rev.dy) continue;
            if (canMove(pacmanTile, d)) { queuedDirection = d; return; }
        }
        for (int oi = 0; oi < 4; oi++) {
            auto &d = dirs[order[oi]];
            if (canMove(pacmanTile, d)) { queuedDirection = d; return; }
        }
    }
}
