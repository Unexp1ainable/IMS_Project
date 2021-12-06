#pragma once
#include <vector>

#include "LatticeCell.h"


using namespace std;

class Lattice
{
public:
    Lattice(const int height, const int width);
    LatticeCell& at(const int height, const int width);

    vector<LatticeCell>& operator[](int height){ return _cells[height]; }
    const vector<LatticeCell>& operator[](int height) const { return _cells[height]; }
private:
    int _width;
    int _height;
    vector<vector<LatticeCell>> _cells;
};