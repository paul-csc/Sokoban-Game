#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace BabaIsYou {

constexpr float TILE_PIXEL_SIZE = 48.0f;
constexpr size_t MAX_OBJECT_PER_TILE = 5;

enum class ObjectType {
    Empty,
    Wall,
    Baba,
    Flag,
    Rock,

    TextBaba,
    TextRock,
    TextWall,
    TextFlag,
    TextIs,
    TextYou,
    TextWin,
    TextPush,
    TextStop,

    NumType
};
enum class Property { You, Stop, Win, Push };

constexpr ObjectType& operator++(ObjectType& type) {
    return type = ObjectType(int(type) + 1);
}

constexpr bool IsText(ObjectType type) {
    return (type >= ObjectType::TextBaba && type < ObjectType::NumType);
}

inline std::string TypeToStr(ObjectType type) {
    std::string str;
    if (type == ObjectType::TextBaba) {
        str = "baba";
    } else if (type == ObjectType::TextRock) {
        str = "rock";
    } else if (type == ObjectType::TextWall) {
        str = "wall";
    } else if (type == ObjectType::TextFlag) {
        str = "flag";
    } else if (type == ObjectType::TextIs) {
        str = "is";
    } else if (type == ObjectType::TextYou) {
        str = "you";
    } else if (type == ObjectType::TextWin) {
        str = "win";
    } else if (type == ObjectType::TextPush) {
        str = "push";
    } else if (type == ObjectType::TextStop) {
        str = "stop";
    }
    return str;
}

class Tile {
  public:
    void Push(ObjectType type);
    ObjectType Pop();
    bool Remove(ObjectType type);
    void Clear();
    bool IsEmpty() const;
    bool Contains(ObjectType type) const;
    bool Contains(const std::vector<ObjectType>& types) const;

    auto begin() const { return m_objects.begin(); }
    auto end() const { return m_objects.begin() + m_numObjects; }

  private:
    std::array<ObjectType, MAX_OBJECT_PER_TILE> m_objects;
    int m_numObjects = 0;
};

} // namespace BabaIsYou
