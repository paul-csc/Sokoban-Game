#include "raylib.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

constexpr float TILE_SIZE = 48;
constexpr int LEVEL_WIDTH = 33;
constexpr int LEVEL_HEIGHT = 18;
constexpr int SCREEN_WIDTH = TILE_SIZE * LEVEL_WIDTH;
constexpr int SCREEN_HEIGHT = TILE_SIZE * LEVEL_HEIGHT;

constexpr size_t MAX_OBJECT_PER_TILE = 6;

constexpr int NUM_LEVEL = 2;

constexpr size_t MAX_HISTORY = 512;

struct Vec2i {
    int x;
    int y;
};

enum class TileType { Empty, Wall, Player, Flag, Rock };

class Tile {
  public:
    void Push(TileType type) {
        if (m_numObjects >= MAX_OBJECT_PER_TILE) {
            return;
        }

        m_objects[m_numObjects++] = type;
    }

    TileType Pop() {
        assert(m_numObjects >= 1);
        return m_objects[--m_numObjects];
    }

    bool Remove(TileType type) {
        for (int i = 0; i < m_numObjects; ++i) {
            if (m_objects[i] == type) {
                for (int j = i; j < m_numObjects - 1; j++) {
                    m_objects[j] = m_objects[j + 1];
                }

                m_numObjects--;
                return true;
            }
        }
        return false;
    }

    bool IsEmpty() const { return m_numObjects == 0; }

    bool Contains(TileType type) const {
        for (int i = 0; i < m_numObjects; ++i) {
            if (m_objects[i] == type) {
                return true;
            }
        }
        return false;
    }

    // iteration (non-const)
    auto begin() { return m_objects.begin(); }
    auto end() { return m_objects.begin() + m_numObjects; }

    // iteration (const)
    auto begin() const { return m_objects.begin(); }
    auto end() const { return m_objects.begin() + m_numObjects; }

  private:
    std::array<TileType, MAX_OBJECT_PER_TILE> m_objects;
    int m_numObjects = 0;
};

// ' ' = empty
// '#' = wall
// 'R' = rock
// 'F' = flag
// '@' = player start

// clang-format off
static const char Levels[NUM_LEVEL][LEVEL_HEIGHT][LEVEL_WIDTH + 1] = {
{
    "#################################",
    "#       @                       #",
    "#           R R                 #",
    "#                               #",
    "#      F          ##            #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#################################",
}, {
    "#################################",
    "#       @                       #",
    "#           RRRRRRRR            #",
    "#                               #",
    "#      F          ##            #",
    "#                               #",
    "#                               #",
    "#    RRRRRRRRRRRRRRRRRRR        #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#################################",
}
};
// clang-format on

class Game {
  public:
    Game() {
        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sokoban");
        SetTargetFPS(60);
        LoadLevel(m_levelNum);
    }

    ~Game() { CloseWindow(); }

    void Update() {
        if (!m_isWin) {
            if (IsKeyPressed(KEY_W)) {
                TryMove(0, -1);
            } else if (IsKeyPressed(KEY_S)) {
                TryMove(0, 1);
            } else if (IsKeyPressed(KEY_A)) {
                TryMove(-1, 0);
            } else if (IsKeyPressed(KEY_D)) {
                TryMove(1, 0);
            }
        }

        if (IsKeyPressed(KEY_R)) {
            LoadLevel(m_levelNum);
        } else if (IsKeyPressed(KEY_N) && m_levelNum < NUM_LEVEL) {
            LoadLevel(++m_levelNum);
        } else if (IsKeyPressed(KEY_P) && m_levelNum > 1) {
            LoadLevel(--m_levelNum);
        } else if (IsKeyPressed(KEY_X)) {
            Undo();
        }
    }

    void Draw() const {
        BeginDrawing();
        ClearBackground({ 20, 20, 20, 255 });

        for (int y = 0; y < LEVEL_HEIGHT; ++y) {
            for (int x = 0; x < LEVEL_WIDTH; ++x) {
                Rectangle r = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };

                DrawRectangleRec(r, { 50, 50, 50, 255 });
                DrawRectangleLines((int)r.x, (int)r.y, (int)r.width, (int)r.height, { 30, 30, 30, 255 });

                for (const auto object : m_tiles[y][x]) {
                    if (object == TileType::Wall) {
                        DrawRectangleRec(r, DARKGRAY);
                    } else if (object == TileType::Rock) {
                        DrawRectangleRounded({ r.x + 4.0f, r.y + 4.0f, TILE_SIZE - 8, TILE_SIZE - 8 }, 0.3f,
                            6, { 150, 100, 60, 255 });
                    } else if (object == TileType::Flag) {
                        DrawRectangle(r.x + 15, r.y + 5, TILE_SIZE - 42, TILE_SIZE - 10, YELLOW);
                        DrawRectangle(r.x + 21, r.y + 5, 17, 16, YELLOW);
                    } else if (object == TileType::Player) {
                        DrawRectangleRec({ r.x + 6, r.y + 6, TILE_SIZE - 12, TILE_SIZE - 12 }, BLUE);

                        // eyes
                        DrawRectangleRec({ r.x + 13, r.y + 15, 7, 7 }, BLACK);
                        DrawRectangleRec({ r.x + TILE_SIZE - 20, r.y + 15, 7, 7 }, BLACK);
                    }
                }
            }
        }

        if (m_isWin) {
            DrawText("You Win!", 50, 50, 50, GREEN);
        }

        EndDrawing();
    }

  private:
    void LoadLevel(int number) {
        number--;
        assert(number >= 0 && number < NUM_LEVEL);

        m_historyStart = 0;
        m_historyCount = 0;

        m_isWin = false;
        bool hasPlayer = false;
        bool hasFlag = false;

        for (int y = 0; y < LEVEL_HEIGHT; ++y) {
            for (int x = 0; x < LEVEL_WIDTH; ++x) {
                m_tiles[y][x] = Tile{};

                switch (Levels[number][y][x]) {
                    case '#': m_tiles[y][x].Push(TileType::Wall); break;
                    case 'R': m_tiles[y][x].Push(TileType::Rock); break;
                    case 'F':
                        m_tiles[y][x].Push(TileType::Flag);
                        hasFlag = true;
                        break;
                    case '@':
                        m_tiles[y][x].Push(TileType::Player);
                        m_player = { x, y };
                        hasPlayer = true;
                        break;
                    case ' ': break;
                    default: assert(false);
                }
            }
        }

        assert(hasPlayer);
        assert(hasFlag);

        SaveState();
    }

    static bool InBounds(int x, int y) { return x >= 0 && x < LEVEL_WIDTH && y >= 0 && y < LEVEL_HEIGHT; }

    void TryMove(int dx, int dy) {
        const int nx = m_player.x + dx;
        const int ny = m_player.y + dy;

        if (!InBounds(nx, ny) || m_tiles[ny][nx].Contains(TileType::Wall)) {
            return;
        }

        // scan forward until non-rock
        int cx = nx;
        int cy = ny;
        while (InBounds(cx, cy) && m_tiles[cy][cx].Contains(TileType::Rock)) {
            cx += dx;
            cy += dy;
        }

        if (!InBounds(cx, cy) || m_tiles[cy][cx].Contains(TileType::Wall)) {
            return;
        }

        // if destination cell already has a rock (shouldn't happen since we scanned), block
        // but scanning should stop at first non-rock, so it's safe.

        SaveState();

        // shift rocks forward (back-to-front)
        while (cx != nx || cy != ny) {
            const int px = cx - dx;
            const int py = cy - dy;

            // if there is a rock at source, move it to destination
            if (m_tiles[py][px].Contains(TileType::Rock)) {
                m_tiles[cy][cx].Push(TileType::Rock);
                m_tiles[py][px].Remove(TileType::Rock);
            }

            cx = px;
            cy = py;
        }

        // move player
        m_tiles[m_player.y][m_player.x].Remove(TileType::Player);
        m_tiles[ny][nx].Push(TileType::Player);
        m_player.x = nx;
        m_player.y = ny;

        if (m_tiles[ny][nx].Contains(TileType::Flag)) {
            m_isWin = true;
        }
    }

    void SaveState() {
        size_t index = (m_historyStart + m_historyCount) % MAX_HISTORY;

        m_history[index] = GameState(m_tiles, m_player, m_isWin);

        if (m_historyCount < MAX_HISTORY) {
            m_historyCount++;
        } else {
            m_historyStart = (m_historyStart + 1) % MAX_HISTORY;
        }
    }

    void Undo() {
        if (m_historyCount == 0) {
            return;
        }

        size_t index = (m_historyStart + m_historyCount - 1) % MAX_HISTORY;

        m_tiles = m_history[index].tiles;
        m_player = m_history[index].player;
        m_isWin = m_history[index].isWin;

        m_historyCount--;
    }

    struct GameState {
        std::array<std::array<Tile, LEVEL_WIDTH>, LEVEL_HEIGHT> tiles;
        Vec2i player;
        bool isWin;
    };

    std::array<std::array<Tile, LEVEL_WIDTH>, LEVEL_HEIGHT> m_tiles;
    Vec2i m_player; // index into m_tiles

    int m_levelNum = 1;
    bool m_isWin = false;

    std::array<GameState, MAX_HISTORY> m_history;
    size_t m_historyStart = 0; // oldest saved
    size_t m_historyCount = 0; // how many valid snapshots
};

int main() {
    auto game = std::make_unique<Game>();

    while (!WindowShouldClose()) {
        game->Update();
        game->Draw();
    }

    return 0;
}
