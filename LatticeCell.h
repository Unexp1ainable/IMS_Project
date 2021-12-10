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
          0
       1  |  5
         >|<
       2  |  4
          3     */
    WindCell wind[6]{}; // each bucket will take one cell, which will be determined by its direction
    SnowCell snow = false;
    bool solid;
    unsigned erosion = 0;

    /**
     * @brief Decrements erosion counter, unless it is 0.
     * If the counter reaches 0, solid property is set to false.
     *
     * @param direction Escape vector of the wind, to be set into the cell.
     */
    void decrementCounter(int direction)
    {
        if (!snow)
            return;
        if (erosion)
        {
            erosion--;
        }
        if (!erosion)
        {
            for (int i = 0; i < 6; i++)
            {
                wind[i] = false;
            }
            wind[direction] = true;
            solid = false;
        }
    }
};
