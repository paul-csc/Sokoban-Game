#pragma once

#include "bimap.h"
#include "level.h"
#include "tile.h"
#include <string>
#include <vector>

namespace BabaIsYou {

constexpr int SCREEN_WIDTH = TILE_PIXEL_SIZE * LEVEL_WIDTH;
constexpr int SCREEN_HEIGHT = TILE_PIXEL_SIZE * LEVEL_HEIGHT;
constexpr size_t MAX_HISTORY = 512;

class Game {
  public:
    Game();
    ~Game();

    void Loop();

  private:
    void Update();
    void Draw() const;

    void Reset();

    static bool InBounds(int x, int y);
    static bool VecContains(const std::vector<ObjectType>& v, ObjectType type);
    bool AllPushable(const Tile& tile, const std::vector<ObjectType>& pushObjects) const;
    void TryMove(int dx, int dy);

    void SaveState();
    void LoadState(const GameState& gs);
    void Undo();

    struct Vec2i {
        int x;
        int y;
    };

    GameState m_currentState;
    LevelManager m_levelManager;

    BiMap<ObjectType, Property> m_rules;

    std::array<GameState, MAX_HISTORY> m_history;
    size_t m_historyStart = 0; // oldest saved
    size_t m_historyCount = 0; // how many valid snapshots
};

} // namespace BabaIsYou