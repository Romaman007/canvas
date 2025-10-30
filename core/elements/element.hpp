#pragma once
#include "../types.hpp"

struct Element {
    // базовая «тупая» сущность: позиция/размер в world, без знания масштаба
    Vec2 pos{0,0};
    Vec2 size{0,0};
};
