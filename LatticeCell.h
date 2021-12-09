#pragma once

#include <iostream>

using SnowCell = bool;
using WindCell = bool;

enum class Direction
{
    UP,
    UPLEFT,
    DOWNLEFT,
    DOWN,
    DOWNRIGHT,
    UPRIGHT
};

class LatticeCell
{
public:
    LatticeCell(const bool solid_ = false) : solid(solid_) {}
    /**
        2   1
         \ /
      3 --x-- 0
         / \
        4   5   */
    WindCell wind[6]{}; // each bucket will take one cell, which will be determined by its direction
    SnowCell snow = false;
    bool solid;
    unsigned erosion = 0;

    /**
     * @brief Decrements erosion counter, unless it is 0. 
     * If the counter reaches 0, solid property is set to false.
     */
    void decrementCounter()
    {
        if (!snow)
            return;
        if (erosion)
        {
            erosion--;
        }
        if (!erosion)
        {
            solid = false;
        }
    }
};