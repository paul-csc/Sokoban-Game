#include "level.h"
#include <cassert>

namespace BabaIsYou {

// clang-format off
const std::array<Level, NUM_LEVEL> LevelManager::m_Levels ={{
{
    "#################################",
    "#       @                       #",
    "#           000                 #",
    "#       0                       #",
    "#       0        ###            #",
    "#       0        #              #",
    "#       0        #       #      #",
    "#       0        #       #      #",
    "#       0        #       #      #",
    "#       0                #      #",
    "#                        #      #",
    "#                        #      #",
    "#       $    ABC         #      #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#                               #",
    "#################################",
},
{
    "#################################",
    "#       @                       #",
    "#           00000000            #",
    "#                               #",
    "#      $                        #",
    "#                               #",
    "#                               #",
    "#          00000000000          #",
    "#                    0          #",
    "#                    0          #",
    "#                    0          #",
    "#          00000000000          #",
    "#          0                    #",
    "#          0                    #",
    "#          0                    #",
    "#          00000000000          #",
    "#                               #",
    "#################################",
},
{    
    "#################################",
    "#       @                       #",
    "#           00000000            #",
    "#                               #",
    "#      $                        #",
    "#                               #",
    "#                               #",
    "#            00000000000        #",
    "#                      0        #",
    "#                      0        #",
    "#                      0        #",
    "#                0000000        #",
    "#                      0        #",
    "#                      0        #",
    "#                      0        #",
    "#            00000000000        #",
    "#                               #",
    "#################################",
}
}};
// clang-format on

LevelManager::LevelManager() {
    m_charToTile.fill(ObjectType::Empty);
    m_charToTile['#'] = ObjectType::Wall;
    m_charToTile['0'] = ObjectType::Rock;
    m_charToTile['@'] = ObjectType::Baba;
    m_charToTile['$'] = ObjectType::Flag;

    assert(int(ObjectType::NumType) - int(ObjectType::TextBaba) <= 26);

    char c = 'A';
    for (ObjectType i = ObjectType::TextBaba; i <= ObjectType::NumType; ++i) {
        m_charToTile[c++] = i;
    }
}

void LevelManager::LoadLevel(GameState& gs) const {
    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
        for (int x = 0; x < LEVEL_WIDTH; ++x) {
            gs.tiles[y][x] = Tile{};
            char c = GetLevel(m_currentLevel)[y][x];
            if (c == ' ') {
                continue;
            }

            if (m_charToTile[c] != ObjectType::Empty) {
                gs.tiles[y][x].Push(m_charToTile[c]);
            } else {
                assert(false);
            }
        }
    }

    gs.isWin = false;
}

void LevelManager::NextLevel(GameState& gs) {
    if (m_currentLevel + 1 < NUM_LEVEL) {
        m_currentLevel++;
        LoadLevel(gs);
    }
}

void LevelManager::PreviousLevel(GameState& gs) {
    if (m_currentLevel > 0) {
        m_currentLevel--;
        LoadLevel(gs);
    }
}

const Level& LevelManager::GetLevel(int index) const {
    assert(index < NUM_LEVEL && index >= 0);
    return m_Levels[index];
}

} // namespace BabaIsYou
