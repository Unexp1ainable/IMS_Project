#pragma once

#include <iostream>

using SnowCell = bool;
using WindCell = bool;

enum class Direction {
    NONE = -1,
    UP,
    UPLEFT,
    DOWNLEFT,
    DOWN,
    DOWNRIGHT,
    UPRIGHT
};

class LatticeCell {
   public:
    LatticeCell(const bool solid_ = false)
        : solid(solid_) {}
    /**
          0
       1  |  5
         >|<
       2  |  4
          3     */
    WindCell wind[6]{};  // each bucket will take one cell, which will be determined by its direction
    SnowCell snow = false;
    bool solid;
    unsigned erosion = 0;

    /**
     * @brief Decrements erosion counter, unless it is 0.
     * If the counter reaches 0, solid property is set to false.
     *
     * @param direction Escape vector of the wind, to be set into the cell.
     */
    void decrementCounter(int direction) {
        if (!snow)
            return;
        if (erosion) {
            erosion--;
        }
        if (!erosion) {
            // for (int i = 0; i < 6; i++) {
            //     wind[i] = false;
            // }
            // wind[direction] = true;
            solid = false;
        }
    }

    int getVector() {
        // mean wind & gravity
        int windParticleCount = 0;
        int shift = 0;
        int ref = -1;
        for (int i = 0; i < 6; i++) {
            if (wind[i]) {
                // ignore vector if it has opposite vector
                if (wind[(i + 3) % 6])
                    continue;
                // set reference vector
                if (ref == -1) {
                    ref = i;
                } else {
                    int r1 = i - ref;
                    int r2 = -6 + r1;
                    int r = r1 <= -r2 ? r1 : r2;
                    if (r == 3) {
                        windParticleCount -= 2;
                    } else {
                        shift += r;
                    }
                }
                windParticleCount++;
            }
        }
        if (ref == -1 || windParticleCount == 0 || (shift == 0 && windParticleCount != 1)) {
            return -1;
        } else {
            int error = shift % windParticleCount;
            if (rand() % 2)
                return ref + shift / windParticleCount;
            else
                return ref + (error + shift) / windParticleCount;
        }
    }
};
