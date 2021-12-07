#include "Lattice.h"

Lattice::Lattice(const int height, const int width) : _width(width),
                                                      _height(height)
{
    _cells = vector<vector<LatticeCell>>(height * 2 + 4, vector<LatticeCell>(width / 2 + 2));
}

void Lattice::mask()
{
}