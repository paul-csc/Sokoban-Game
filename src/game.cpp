#include "game.h"
#include "raylib.h"
#include <algorithm>
#include <cassert>

namespace BabaIsYou {

Game::Game() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sokoban");
    SetTargetFPS(60);

    m_levelManager.LoadLevel(m_currentState);
    Reset();

    m_rules.Add(ObjectType::Baba, Property::You);
    m_rules.Add(ObjectType::Wall, Property::Stop);
    m_rules.Add(ObjectType::Flag, Property::Win);
    m_rules.Add(ObjectType::Rock, Property::Push);

    for (ObjectType i = ObjectType::TextBaba; i <= ObjectType::NumType; ++i) {
        m_rules.Add(i, Property::Push);
    }
}

Game::~Game() {
    CloseWindow();
}

void Game::Loop() {
    while (!WindowShouldClose()) {
        Update();
        Draw();
    }
}

void Game::Update() {
    if (!m_currentState.isWin) {
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
        m_levelManager.LoadLevel(m_currentState);
        Reset();
    } else if (IsKeyPressed(KEY_N)) {
        m_levelManager.NextLevel(m_currentState);
        Reset();
    } else if (IsKeyPressed(KEY_P)) {
        m_levelManager.PreviousLevel(m_currentState);
        Reset();
    } else if (IsKeyPressed(KEY_X)) {
        Undo();
    }
}

void Game::Draw() const {
    BeginDrawing();
    ClearBackground({ 20, 20, 20, 255 });

    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
        for (int x = 0; x < LEVEL_WIDTH; ++x) {
            const Tile& tile = m_currentState.tiles[y][x];
            Rectangle r = { x * TILE_PIXEL_SIZE, y * TILE_PIXEL_SIZE, TILE_PIXEL_SIZE,
                TILE_PIXEL_SIZE };

            DrawRectangleRec(r, { 50, 50, 50, 255 });
            DrawRectangleLines(
                (int)r.x, (int)r.y, (int)r.width, (int)r.height, { 30, 30, 30, 255 });

            for (const auto object : tile) {
                if (object == ObjectType::Wall) {
                    DrawRectangleRec(r, DARKGRAY);
                } else if (object == ObjectType::Rock) {
                    DrawRectangleRounded(
                        { r.x + 7.0f, r.y + 7.0f, TILE_PIXEL_SIZE - 14, TILE_PIXEL_SIZE - 14 },
                        0.3f, 6, { 150, 100, 60, 255 });
                } else if (object == ObjectType::Flag) {
                    DrawRectangle(
                        r.x + 15, r.y + 5, TILE_PIXEL_SIZE - 42, TILE_PIXEL_SIZE - 10, YELLOW);
                    DrawRectangle(r.x + 21, r.y + 5, 17, 16, YELLOW);
                } else if (object == ObjectType::Baba) {
                    DrawRectangleRec(
                        { r.x + 6, r.y + 6, TILE_PIXEL_SIZE - 12, TILE_PIXEL_SIZE - 12 }, BLUE);

                    // eyes
                    DrawRectangleRec({ r.x + 13, r.y + 15, 7, 7 }, BLACK);
                    DrawRectangleRec({ r.x + TILE_PIXEL_SIZE - 20, r.y + 15, 7, 7 }, BLACK);
                } else if (IsText(object)) {
                    DrawRectangleRounded(
                        { r.x + 6.0f, r.y + 6.0f, TILE_PIXEL_SIZE - 12, TILE_PIXEL_SIZE - 12 },
                        0.3f, 6, WHITE);
                    DrawText(TypeToStr(object).c_str(), r.x + 6.0f, r.y + 6.0f, 20, BLACK);
                }
            }
        }
    }

    if (m_currentState.isWin) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, { 50, 50, 50, 150 });
        DrawText("You Win!", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 25, 50, GREEN);
    }

    EndDrawing();
}

void Game::Reset() {
    m_historyStart = 0;
    m_historyCount = 0;
    SaveState();
}

bool Game::InBounds(int x, int y) {
    return x >= 0 && x < LEVEL_WIDTH && y >= 0 && y < LEVEL_HEIGHT;
}

bool Game::VecContains(const std::vector<ObjectType>& v, ObjectType type) {
    return std::find(v.begin(), v.end(), type) != v.end();
}

bool Game::AllPushable(const Tile& tile, const std::vector<ObjectType>& pushObjects) const {
    for (const auto obj : tile) {
        if (VecContains(pushObjects, obj)) {
            return true;
        }
    }
    return false;
}

void Game::TryMove(int dx, int dy) {
    const auto& youObjects = m_rules.Get(Property::You);
    const auto& pushObjects = m_rules.Get(Property::Push);
    const auto& stopObjects = m_rules.Get(Property::Stop);
    const auto& winObjects = m_rules.Get(Property::Win);

    std::vector<std::pair<Vec2i, ObjectType>> yous;
    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
        for (int x = 0; x < LEVEL_WIDTH; ++x) {
            for (const auto obj : m_currentState.tiles[y][x]) {
                if (VecContains(youObjects, obj)) {
                    yous.emplace_back(Vec2i{ x, y }, obj);
                }
            }
        }
    }

    if (yous.empty()) {
        return;
    }

    auto proj = [dx, dy](const Vec2i& pos) { return pos.x * dx + pos.y * dy; };
    std::sort(yous.begin(), yous.end(),
        [&proj](const auto& a, const auto& b) { return proj(a.first) > proj(b.first); });

    SaveState();

    for (const auto& [pos, type] : yous) {
        const int nx = pos.x + dx;
        const int ny = pos.y + dy;

        if (!InBounds(nx, ny) || m_currentState.tiles[ny][nx].Contains(stopObjects)) {
            continue;
        }

        int cx = nx;
        int cy = ny;

        while (InBounds(cx, cy) && !m_currentState.tiles[cy][cx].IsEmpty() &&
            AllPushable(m_currentState.tiles[cy][cx], pushObjects)) {
            cx += dx;
            cy += dy;
        }

        if (!InBounds(cx, cy) || m_currentState.tiles[cy][cx].Contains(stopObjects)) {
            continue;
        }

        // perform shift
        while (cx != nx || cy != ny) {
            const int prevX = cx - dx;
            const int prevY = cy - dy;

            auto& source = m_currentState.tiles[prevY][prevX];
            auto& dest = m_currentState.tiles[cy][cx];

            for (const auto obj : source) {
                if (VecContains(pushObjects, obj)) {
                    source.Remove(ObjectType::Rock);
                    dest.Push(obj);
                }
            }

            cx = prevX;
            cy = prevY;
        }

        // move the You object
        auto& source_you = m_currentState.tiles[pos.y][pos.x];
        if (source_you.Remove(type)) {
            m_currentState.tiles[ny][nx].Push(type);
        }
    }

    // check for win
    m_currentState.isWin = false;
    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
        for (int x = 0; x < LEVEL_WIDTH; ++x) {
            if (m_currentState.tiles[y][x].Contains(youObjects) &&
                m_currentState.tiles[y][x].Contains(winObjects)) {
                m_currentState.isWin = true;
                return;
            }
        }
    }
}

void Game::SaveState() {
    size_t index = (m_historyStart + m_historyCount) % MAX_HISTORY;

    m_history[index] = GameState(m_currentState.tiles, m_currentState.isWin);

    if (m_historyCount < MAX_HISTORY) {
        m_historyCount++;
    } else {
        m_historyStart = (m_historyStart + 1) % MAX_HISTORY;
    }
}

void Game::LoadState(const GameState& gs) {
    m_currentState.tiles = gs.tiles;
    m_currentState.isWin = gs.isWin;
}

void Game::Undo() {
    if (m_historyCount == 0) {
        return;
    }

    size_t index = (m_historyStart + m_historyCount - 1) % MAX_HISTORY;

    LoadState(m_history[index]);

    m_historyCount--;
}

} // namespace BabaIsYou