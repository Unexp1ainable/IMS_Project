#pragma once

using SnowCell = bool;
using WindCell = bool;

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
};