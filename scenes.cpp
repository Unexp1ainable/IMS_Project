#include <vector>

using namespace std;

void sceneBox(vector<vector<bool>>& scene) {
    size_t height = scene.size();
    size_t width = scene[0].size();

    for (size_t i = 0; i < height; i++) {
        scene[i][1] = true;
        scene[i][2] = true;
        scene[i][width - 2] = true;
        scene[i][width - 1] = true;
    };
}

void sceneFloatingStick(vector<vector<bool>>& scene) {
    size_t height = scene.size();
    size_t width = scene[0].size();
    for (size_t i = 0; i < height / 2; i++) {
        scene[height - i - 1 - height / 20][width / 3] = true;
        scene[height - i - 1 - height / 20][width / 3 + 1] = true;
    }
}

// testing stuff
// lattice[50][25].wind[0] = true;
// lattice[50][25].wind[1] = true;
// lattice[50][25].wind[2] = true;
// lattice[50][25].wind[3] = true;
// lattice[50][25].wind[4] = true;
// lattice[50][25].wind[5] = true;

// downright
// scene[39][81] = true;
// scene[40][81] = true;
// scene[41][80] = true;

// downleft
// scene[39][16] = true;
// scene[40][16] = true;
// scene[40][17] = true;

// upright
// scene[7][80] = true;
// scene[7][81] = true;
// scene[8][81] = true;

// upleft
// scene[7][16] = true;
// scene[7][15] = true;
// scene[8][15] = true;

// up
// scene[9][49] = true;
// scene[9][48] = true;
// scene[9][47] = true;

// down
// scene[38][49] = true;
// scene[39][48] = true;
// scene[38][47] = true;
