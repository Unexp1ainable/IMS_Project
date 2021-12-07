#include "Lattice.h"

Lattice::Lattice(const int height, const int width, vector<vector<bool>> scene) : _width(width),
                                                                                  _height(height),
                                                                                  _scene(scene)
{
    _cells = vector<vector<LatticeCell>>(height * 2 + 4, vector<LatticeCell>(width / 2 + 2, false));
    for (int y = 2; y < _height * 2 + 2; y++)
    {
        for (int x = 1; x < _width / 2 + 1; x++)
        {
            _cells[y][x].solid = _scene[(y - 2) / 2][(x - 1) * 2 + y % 2];
        }
    }
}