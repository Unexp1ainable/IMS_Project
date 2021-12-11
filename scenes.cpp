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
        scene[height - i - 1 - height / 15][width / 3] = true;
        scene[height - i - 1 - height / 15][width / 3 + 1] = true;
    }
}

void sceneStick(vector<vector<bool>>& scene) {
    size_t height = scene.size();
    size_t width = scene[0].size();
    for (size_t i = 0; i < height / 2; i++) {
        scene[height - i - 1][width / 3] = true;
        scene[height - i - 1][width / 3 + 1] = true;
    }
}

void sceneHouse(vector<vector<bool>>& scene) {
    size_t height = scene.size();
    size_t width = scene[0].size();
    for (size_t i = 0; i < height / 6 + 2; i++) {
        scene[height - i - 1][width / 6 * 2] = true;
        scene[height - i - 1][width / 6 * 2 + 1] = true;

        scene[height - i - 1][width / 6 * 3] = true;
        scene[height - i - 1][width / 6 * 3 + 1] = true;
    }

    for (size_t i = 0; i < width / 6; i++) {
        scene[height / 6 * 5 + 1][width / 6 * 2 + i] = true;
    }

    for (size_t i = 0; i < width / 12; i++) {
        scene[height / 6 * 5 + 1 - width / 24 + i / 2][width / 3 + width / 12 + i] = true;
        scene[height / 6 * 5 + 1 - width / 24 + i / 2][width / 3 + width / 12 - i] = true;
        scene[height / 6 * 5 + 1 - width / 24 + i / 2 - 1][width / 3 + width / 12 + i] = true;
        scene[height / 6 * 5 + 1 - width / 24 + i / 2 - 1][width / 3 + width / 12 - i] = true;
    }
}

void sceneOpenHouse(vector<vector<bool>>& scene) {
    size_t height = scene.size();
    size_t width = scene[0].size();
    for (size_t i = 0; i < height / 8; i++) {
        scene[height - i - 1][width / 6 * 2] = true;
        scene[height - i - 1][width / 6 * 2 + 1] = true;

        scene[height - i - 1][width / 6 * 3] = true;
        scene[height - i - 1][width / 6 * 3 + 1] = true;
    }

    for (size_t i = 0; i < width / 6; i++) {
        scene[height / 6 * 5 + 1][width / 6 * 2 + i] = true;
    }

    for (size_t i = 0; i < width / 12; i++) {
        scene[height / 6 * 5 + 1 - width / 24 + i / 2][width / 3 + width / 12 + i] = true;
        scene[height / 6 * 5 + 1 - width / 24 + i / 2][width / 3 + width / 12 - i] = true;
        scene[height / 6 * 5 + 1 - width / 24 + i / 2 - 1][width / 3 + width / 12 + i] = true;
        scene[height / 6 * 5 + 1 - width / 24 + i / 2 - 1][width / 3 + width / 12 - i] = true;
    }
}

void sceneOpenWindow(vector<vector<bool>>& scene) {
    size_t height = scene.size();
    size_t width = scene[0].size();
    for (size_t i = 0; i < height / 8; i++) {
        scene[height - i - 1][width / 6 * 2] = true;
        scene[height - i - 1][width / 6 * 2 + 1] = true;
    }

    for (size_t i = 0; i < height / 6 + 2; i++) {
        scene[height - i - 1][width / 6 * 3] = true;
        scene[height - i - 1][width / 6 * 3 + 1] = true;
    }

    for (size_t i = 0; i < width / 6; i++) {
        scene[height / 6 * 5 + 1][width / 6 * 2 + i] = true;
    }

    for (size_t i = 0; i < width / 12; i++) {
        scene[height / 6 * 5 + 1 - width / 24 + i / 2][width / 3 + width / 12 + i] = true;
        scene[height / 6 * 5 + 1 - width / 24 + i / 2][width / 3 + width / 12 - i] = true;
        scene[height / 6 * 5 + 1 - width / 24 + i / 2 - 1][width / 3 + width / 12 + i] = true;
        scene[height / 6 * 5 + 1 - width / 24 + i / 2 - 1][width / 3 + width / 12 - i] = true;
    }
}

void sceneWindBreakers1(vector<vector<bool>>& scene) {
    size_t height = scene.size();
    size_t width = scene[0].size();

    for (size_t i = width / 8; i < width / 8 * 7; i += width / 8) {
        size_t f = 0;
        for (size_t j = height - 1; j > height / 6 * 4; j--, f++) {
            if (j % 7 < 4) {
                scene[j][i + f] = true;
                scene[j][i + f + 1] = true;
            }
        }
    }
}

void sceneWindBreakers2(vector<vector<bool>>& scene) {
    size_t height = scene.size();
    size_t width = scene[0].size();

    for (size_t i = width / 8; i < width / 8 * 7; i += width / 8) {
        size_t f = 0;
        for (size_t j = height - 1; j > height / 6 * 4; j--, f++) {
            if (j % 7 < 4) {
                scene[j][i] = true;
                scene[j][i + 1] = true;
            }
        }
    }
}

void sceneWindBreaker(vector<vector<bool>>& scene) {
    size_t height = scene.size();
    size_t width = scene[0].size();

    size_t f = 0;
    for (size_t j = height - 1; j > height / 6 * 4; j--, f++) {
        if (j % 7 < 4) {
            scene[j][width / 2] = true;
            scene[j][width / 2 + 1] = true;
        }
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
