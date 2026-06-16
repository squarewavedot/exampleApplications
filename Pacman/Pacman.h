#ifndef PACMAN_PACMAN_H
#define PACMAN_PACMAN_H

#include <CubeApplication.h>
#include <Joystick.h>

#include <array>
#include <string>
#include <vector>

class Pacman : public CubeApplication {
public:
    Pacman(std::string serverUri = DEFAULTSERVERURI, int initialSeed = -1);

    bool loop() override;

private:
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    // Each face is 64×64 pixels.
    // We divide each face into a 16×16 tile grid (4 px per tile).
    // Walls are 1-px or 2-px lines drawn on pixel coordinates.
    // Sprites are 5×5 pixels (centred in a tile).
    static const int FACE_PX    = 64;   // pixels per face side
    static const int TILE_PX    = 4;    // pixels per tile cell
    static const int FACE_TILES = 16;   // tiles per face side  (64/4)

    // Wall grid: between tiles we store horizontal and vertical wall edges.
    // hwall[face][row][col] == true  →  wall between tile (col,row-1) and (col,row)  (horizontal line at y = row*TILE_PX)
    // vwall[face][row][col] == true  →  wall between tile (col-1,row) and (col,row)  (vertical line at x = col*TILE_PX)
    // Dimensions:  [6][FACE_TILES+1][FACE_TILES]  and  [6][FACE_TILES][FACE_TILES+1]
    static const int EDGE = FACE_TILES + 1;

    using WallGrid = std::array<std::array<std::array<bool, FACE_TILES>, EDGE>, 6>;   // hwall
    using VWallGrid = std::array<std::array<std::array<bool, EDGE>, FACE_TILES>, 6>;  // vwall

    // -----------------------------------------------------------------------
    // Data types
    // -----------------------------------------------------------------------

    struct Direction {
        int dx, dy;
    };

    struct Tile {
        ScreenNumber face;
        int x, y;   // tile coordinates [0..FACE_TILES-1]
    };

    struct StepResult {
        Tile tile;
        Direction dir;
    };

    enum GameState { menu, ready, playing, gameOver, victory };

    enum BonusType { Cherry = 0, Strawberry, Orange, Banana };

    struct BonusItem {
        Tile      tile;
        BonusType type;
        int       timer;   // frames remaining until disappears (0 = inactive)
        bool      active;
    };

    struct Ghost {
        Tile tile;
        Tile lastTile;
        Direction direction;
        Tile home;
        Color color;
        int releaseDelay;
        bool eaten;   // true = nur Augen sichtbar, läuft zum Spawn zurück
    };

    enum class FireworkPhase { rocket, burst };

    struct Firework {
        float x, y;
        float vx, vy;
        int life;
        int maxLife;
        Color color;
        ScreenNumber face;
        FireworkPhase phase;
    };

    struct FaceBasis {
        ScreenNumber face;
        Vector3i origin;   // world coord of tile (0,0) top-left pixel
        Vector3i axisX;    // world delta per 1 pixel in face-X direction
        Vector3i axisY;    // world delta per 1 pixel in face-Y direction
    };

    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    void buildBoard();
    void resetGame(bool newSeed = true);
    void saveHighScore();
    void loadHighScore();
    void updateMenu();
    void updateInput();
    void updateGame();
    void updateGameOver();
    void updatePacman();
    void updateGhosts();
    void updateGhost(Ghost &ghost, int ghostIndex);

    void renderBoard();
    void renderGameOver();
    void renderMenu();
    void renderWalls(float brightness = 1.0f);
    void renderWallsFace(int face, float brightness = 1.0f);
    void renderGhostHouse();
    void renderPellets();
    void renderBonusItem();
    void renderPacman();
    void renderGhost(const Ghost &ghost);
    void drawPixel(ScreenNumber face, int px, int py, Color color);
    void drawHLine(ScreenNumber face, int x0, int x1, int y, Color color, int thickness = 1);
    void drawVLine(ScreenNumber face, int x, int y0, int y1, Color color, int thickness = 1);

    bool isWall(Tile tile) const;
    // ghostDoor: 0=blocked(Pacman), 1=exit-only(normal ghost), 2=enter+exit(eaten ghost)
    bool canMove(Tile from, Direction dir, int ghostDoor = 0);
    bool isGhostDoor(Tile from, Tile to) const;
    bool hasHWall(int face, int col, int row) const;   // wall above row
    bool hasVWall(int face, int col, int row) const;   // wall left of col

    Tile stepTile(Tile from, Direction dir);
    StepResult stepResult(Tile from, Direction dir) const;
    Tile tileFromWorld(Vector3i world);
    Vector3i tileToWorld(Tile tile);
    Direction directionAfterStep(Tile from, Direction dir, Tile to) const;
    Direction rotateDirection(ScreenNumber oldFace, ScreenNumber newFace, Direction dir) const;
    Direction chooseGhostDirection(const Ghost &ghost, int ghostIndex);
    int manhattanDistance(Tile a, Tile b);
    FaceBasis faceBasis(ScreenNumber face) const;
    bool sameTile(Tile a, Tile b) const;
    Tile offsetTarget(Tile tile, Direction dir, int steps);
    void demoChooseDirection();

    void setHWall(int face, int col, int row, bool v = true);
    void setVWall(int face, int col, int row, bool v = true);
    void addRect(int face, int x0, int y0, int x1, int y1);
    void addHCorridor(int face, int x0, int x1, int y);
    void addVCorridor(int face, int y0, int y1, int x);
    void openBorderGap(int face, int axis, int borderPos, int gapStart);
    void spawnBonusItem();
    void updateBonusItem();

    // -----------------------------------------------------------------------
    // State
    // -----------------------------------------------------------------------

    WallGrid  hwall{};   // horizontal walls (row-boundaries)
    VWallGrid vwall{};   // vertical walls (col-boundaries)

    // Pellet: one per tile that is not a wall and not the ghost house
    std::array<std::array<std::array<bool, FACE_TILES>, FACE_TILES>, 6> pellets{};
    // Power pellets (2x2 px, frightens ghosts)
    std::array<std::array<std::array<bool, FACE_TILES>, FACE_TILES>, 6> powerPellets{};
    int frightenedTimer{0};   // counts down when ghosts are blue

    std::vector<Ghost> ghosts;
    Joystick joystick;

    Tile pacmanTile;
    Direction pacmanDirection{0, 0};
    Direction queuedDirection{0, 0};

    // Ghost house geometry (tile coords on top face)
    static const int GH_X0 = 5, GH_Y0 = 5, GH_X1 = 10, GH_Y1 = 8;
    // Ghost door: horizontal segment at top of ghost house (y = GH_Y0)
    // Door spans cols GH_DOOR_X0 .. GH_DOOR_X1
    static const int GH_DOOR_X0 = 7, GH_DOOR_X1 = 8;

    Tile pacmanSpawn{top, 7, 10};   // just below ghost house

    GameState gameState{ready};
    int menuIndex{0};
    int menuDemoCountdown{100};
    int highScore{0};
    int currentSeed_{0};
    int score{0};
    int pelletCount{0};
    int frameCounter{0};
    int moveDivider{3};       // Pacman: moves every 3rd frame (~6.7 tiles/s at 20fps)
    int ghostMoveDivider{5};  // Ghosts: moves every 5th frame (~4 tiles/s)
    int pelletBlink{0};
    int gameOverTimer_{0};
    int fireworkSpawnTimer_{0};
    int gameOverMenuIndex_{0};
    bool mouthOpen{true};
    bool newHighScore_{false};

    BonusItem bonusItem{};
    int bonusSpawnCounter{0};
    std::vector<Firework> fireworks_;
    bool demoActive_{false};
    bool seedMenuActive_{false};
    int seedStub_{1234};
    bool quitRequested_{false};

    struct MenuSprite {
        float x, y;
        float vx, vy;
        Color color;
        bool isPacman;
    };
    std::vector<MenuSprite> menuSprites_;
    bool menuSpritesInit_{false};
};

#endif // PACMAN_PACMAN_H
