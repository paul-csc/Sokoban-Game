#pragma once

#include "tile.h"
#include <array>

namespace BabaIsYou {

constexpr int LEVEL_WIDTH = 33;
constexpr int LEVEL_HEIGHT = 18;
constexpr int NUM_LEVEL = 3;

struct GameState {
    std::array<std::array<Tile, LEVEL_WIDTH>, LEVEL_HEIGHT> tiles;
    bool isWin;
};

using Level = std::array<std::array<char, LEVEL_WIDTH + 1>, LEVEL_HEIGHT>;

class LevelManager {
  public:
    LevelManager();

    void LoadLevel(GameState& gs) const;
    void NextLevel(GameState& gs);
    void PreviousLevel(GameState& gs);

  private:
    const Level& GetLevel(int index) const;

    int m_currentLevel = 0;
    static const std::array<Level, NUM_LEVEL> m_Levels;

    std::array<ObjectType, 256> m_charToTile{};
};

} // namespace BabaIsYou
