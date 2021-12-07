#pragma once
#include <vector>

#include "LatticeCell.h"

using namespace std;

class Lattice
{
public:
    Lattice(const int height, const int width, vector<vector<bool>> scene);
    LatticeCell &at(const int height, const int width);

    vector<LatticeCell> &operator[](int height) { return _cells[height]; }
    const vector<LatticeCell> &operator[](int height) const { return _cells[height]; }

private:
    int _width;
    int _height;

    // mask of the same size as the lattice,
    // true cell means, that the cell in the lattice is immovable solid cell
    vector<vector<bool>> _scene{};

    vector<vector<LatticeCell>> _cells{};
};