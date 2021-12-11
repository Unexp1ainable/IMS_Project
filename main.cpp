#include <iostream>
#include <vector>

#include "Lattice.h"
#include "opencv2/opencv.hpp"
#include "scenes.cpp"

using namespace cv;
using namespace std;

#define LATTICE_WIDTH 900
#define LATTICE_HEIGHT 100
#define COHESION 5
#define GRAVITY 8

int wind_strength;  // must be between 0-LATTICE_HEIGHT
int snow_strength;  // must be between 0-LATTICE_WIDTH
vector<vector<bool>> scene{};
bool snowing = false;

void toCartesian(const Mat& hex, Mat& dst) {
    auto stamp = [&](int x, int y, uint8_t state) {
        dst.at<uint8_t>(y + 1, x) = state;
        dst.at<uint8_t>(y + 2, x) = state;

        dst.at<uint8_t>(y, x + 1) = state;
        dst.at<uint8_t>(y + 1, x + 1) = state;
        dst.at<uint8_t>(y + 2, x + 1) = state;
        dst.at<uint8_t>(y + 3, x + 1) = state;

        dst.at<uint8_t>(y, x + 2) = state;
        dst.at<uint8_t>(y + 1, x + 2) = state;
        dst.at<uint8_t>(y + 2, x + 2) = state;
        dst.at<uint8_t>(y + 3, x + 2) = state;

        dst.at<uint8_t>(y + 1, x + 3) = state;
        dst.at<uint8_t>(y + 2, x + 3) = state;
    };

    for (int y = 0; y < dst.size().height - 4; y += 2) {
        for (int x = ((y % 4) == 2) ? 3 : 0; x < dst.size().width - 2; x += 6) {
            auto state = hex.at<uint8_t>(y / 2, x / 6);
            stamp(x, y, state);
        }
    }
}

void toMat(const Lattice& lattice, Mat& latticeMat) {
    for (int y = 2; y < LATTICE_HEIGHT * 2 + 2; y++) {
        for (int x = 1; x < LATTICE_WIDTH / 2 + 1; x++) {
            auto wind = lattice[y][x].wind;
            if (lattice[y][x].snow) {
                if (lattice[y][x].solid)
                    latticeMat.at<uint8_t>(y - 2, x - 1) = 255;
                else
                    latticeMat.at<uint8_t>(y - 2, x - 1) = 210;

            } else if (wind[0] || wind[1] || wind[2] || wind[3] || wind[4] || wind[5]) {
                latticeMat.at<uint8_t>(y - 2, x - 1) = 25;
            } else if (lattice[y][x].solid) {
                latticeMat.at<uint8_t>(y - 2, x - 1) = 150;
            } else {
                latticeMat.at<uint8_t>(y - 2, x - 1) = 0;
            }
        }
    }
}

size_t countParticles(Lattice& lattice) {
    size_t count = 0;
    for (int y = 2; y < LATTICE_HEIGHT * 2 + 2; y++) {
        // cout << "============Y " << y << " =============" << endl;
        for (int x = 1; x < LATTICE_WIDTH / 2 + 1; x++) {
            for (int i = 0; i < 6; i++) {
                count += lattice[y][x].wind[i];
            }
        }
    }
    return count;
}

/**
 * @brief Shift vector one place to the right or to the left.
 *
 * @param x X coordinate of the place, where snow wants to go
 * @param y Y coordinate of the place, where snow wants to go
 * @param v Vector that pointed to that place
 * @param left Directionn. left if true, right if false
 * @return pair<int, int> (x,y) of the new place
 */
pair<int, int> shiftVector(int x, int y, Direction v, bool left) {
    int xcoord, ycoord;
    bool even = y % 2;
    switch (v) {
        case Direction::UP:
            if (left) {
                xcoord = x - even;
                ycoord = y + 1;
            } else {
                xcoord = x + !even;
                ycoord = y + 1;
            }
            break;

        case Direction::UPLEFT:
            if (left) {
                xcoord = x + !even;
                ycoord = y - 1;
            } else {
                xcoord = x;
                ycoord = y + 2;
            }
            break;

        case Direction::DOWNLEFT:
            if (left) {
                xcoord = x + !even;
                ycoord = y + 1;
            } else {
                xcoord = x;
                ycoord = y - 2;
            }
            break;

        case Direction::DOWN:
            if (left) {
                xcoord = x - !even;
                ycoord = y - 1;
            } else {
                xcoord = x + even;
                ycoord = y - 1;
            }
            break;

        case Direction::DOWNRIGHT:
            if (left) {
                xcoord = x;
                ycoord = y - 2;
            } else {
                xcoord = x - even;
                ycoord = y + 1;
            }
            break;

        case Direction::UPRIGHT:
            if (left) {
                xcoord = x - even;
                ycoord = y - 1;
            } else {
                xcoord = x;
                ycoord = y + 2;
            }
            break;

        default:
            xcoord = x;
            ycoord = y;
            break;
    }

    return make_pair(xcoord, ycoord);
}

void latticeStepPropagation(Lattice& lattice) {
    // flip vertical direction of the wind particles that are about to collide with the sky
    for (int x = 1; x < LATTICE_WIDTH / 2 + 1; x++) {
        auto wind = lattice[2][x].wind;

        if (wind[0]) {
            lattice[2][x].wind[0] = false;
            lattice[2][x].wind[3] = true;
        }
        if (wind[1]) {
            lattice[2][x].wind[1] = false;
            lattice[2][x].wind[2] = true;
        }
        if (wind[5]) {
            lattice[2][x].wind[5] = false;
            lattice[2][x].wind[4] = true;
        }

        wind = lattice[3][x].wind;

        if (wind[0]) {
            lattice[3][x].wind[0] = false;
            lattice[3][x].wind[3] = true;
        }
        if (wind[1]) {
            lattice[3][x].wind[1] = false;
            lattice[3][x].wind[2] = true;
        }
        if (wind[5]) {
            lattice[3][x].wind[5] = false;
            lattice[3][x].wind[4] = true;
        }
    }

    Lattice tmpLattice(LATTICE_HEIGHT, LATTICE_WIDTH, scene);

    for (int y = 2; y < LATTICE_HEIGHT * 2 + 2; y++) {
        for (int x = 1; x < LATTICE_WIDTH / 2 + 1; x++) {
            auto wind = lattice[y][x].wind;

            bool even = y % 2 == 0;  // compensation for matrix representation

            int escapeVector = 0;
            // it should not blow inside
            if (!lattice[y][x].solid) {
                // wind propagation
                if (wind[0]) {
                    // check if solid
                    if (lattice[y - 2][x].solid) {
                        if (lattice[y - 1][x + !even].solid && !lattice[y - 1][x - even].solid) {
                            tmpLattice[y + 1][x - even].wind[2] = true;
                            // escapeVector = 2;
                        } else if (!lattice[y - 1][x + !even].solid && lattice[y - 1][x - even].solid) {
                            tmpLattice[y + 1][x + !even].wind[4] = true;
                            // escapeVector = 4;
                        } else {
                            tmpLattice[y + 2][x].wind[3] = true;
                            // escapeVector = 3;
                        }

                        // decrement counter of the hit particle if it was snow and assign escape vector
                        tmpLattice[y - 2][x].decrementCounter(3);
                    }
                    // aint solid
                    else {
                        tmpLattice[y - 2][x].wind[0] = true;
                    }
                }
                if (wind[1]) {
                    // check if solid
                    if (lattice[y - 1][x - even].solid) {
                        if (lattice[y + 1][x - even].solid && !lattice[y - 2][x].solid) {
                            tmpLattice[y - 1][x + !even].wind[5] = true;
                            // escapeVector = 0;
                        } else if (!lattice[y + 1][x - even].solid && lattice[y - 2][x].solid) {
                            tmpLattice[y + 1][x - even].wind[2] = true;
                            // escapeVector = 2;
                        } else {
                            tmpLattice[y + 1][x + !even].wind[4] = true;
                            // escapeVector = 4;
                        }

                        // decrement counter of the hit particle if it was snow and assign escape vector
                        tmpLattice[y - 1][x - even].decrementCounter(4);
                    }
                    // aint solid
                    else {
                        tmpLattice[y - 1][x - even].wind[1] = true;
                    }
                }
                if (wind[5]) {
                    // check if solid
                    if (lattice[y - 1][x + !even].solid) {
                        if (lattice[y + 1][x + !even].solid && !lattice[y - 2][x].solid) {
                            tmpLattice[y - 1][x - even].wind[1] = true;
                            // escapeVector = 2;
                        } else if (!lattice[y + 1][x + !even].solid && lattice[y - 2][x].solid) {
                            tmpLattice[y + 1][x + !even].wind[4] = true;
                            // escapeVector = 3;
                        } else {
                            tmpLattice[y + 1][x - even].wind[2] = true;
                            // escapeVector = 2;
                        }
                        // decrement counter of the hit particle if it was snow and assign escape vector
                        tmpLattice[y - 1][x + !even].decrementCounter(2);
                    }
                    // aint solid
                    else {
                        tmpLattice[y - 1][x + !even].wind[5] = true;
                    }
                }
                if (wind[2]) {
                    // check if solid
                    if (lattice[y + 1][x - even].solid) {
                        if (lattice[y - 1][x - even].solid && !lattice[y + 2][x].solid) {
                            tmpLattice[y + 1][x + !even].wind[4] = true;
                            // escapeVector = 4;
                        } else if (!lattice[y - 1][x - even].solid && lattice[y + 2][x].solid) {
                            tmpLattice[y - 1][x - even].wind[1] = true;
                            // escapeVector = 1;
                        } else {
                            tmpLattice[y - 1][x + !even].wind[5] = true;
                            // escapeVector = 5;
                        }
                        // decrement counter of the hit particle if it was snow and assign escape vector
                        lattice[y + 1][x - even].decrementCounter(5);
                    }
                    // aint solid
                    else {
                        tmpLattice[y + 1][x - even].wind[2] = true;
                    }
                }
                if (wind[3]) {
                    // check if solid
                    if (lattice[y + 2][x].solid) {
                        if (lattice[y + 1][x + !even].solid && !lattice[y + 1][x - even].solid) {
                            tmpLattice[y - 1][x - even].wind[1] = true;
                            // escapeVector = 1;
                        } else if (!lattice[y + 1][x + !even].solid && lattice[y + 1][x - even].solid) {
                            tmpLattice[y - 1][x + !even].wind[5] = true;
                            // escapeVector = 5;
                        } else {
                            tmpLattice[y - 2][x].wind[0] = true;
                            // escapeVector = 0;
                        }
                        // decrement counter of the hit particle if it was snow and assign escape vector
                        lattice[y + 2][x].decrementCounter(0);
                    }
                    // aint solid
                    else {
                        tmpLattice[y + 2][x].wind[3] = true;
                    }
                }
                if (wind[4]) {
                    // check if solid
                    if (lattice[y + 1][x + !even].solid) {
                        if (lattice[y - 1][x + !even].solid && !lattice[y + 2][x].solid) {
                            tmpLattice[y + 1][x - even].wind[2] = true;
                            // escapeVector = 2;
                        } else if (!lattice[y - 1][x + !even].solid && lattice[y + 2][x].solid) {
                            tmpLattice[y - 1][x + !even].wind[5] = true;
                            // escapeVector = 5;
                        } else {
                            tmpLattice[y - 1][x - even].wind[1] = true;
                            // escapeVector = 1;
                        }
                        // decrement counter of the hit particle if it was snow and assign escape vector
                        lattice[y + 1][x + !even].decrementCounter(1);
                    }
                    // aint solid
                    else {
                        tmpLattice[y + 1][x + !even].wind[4] = true;
                    }
                }
            }

            // snow propagation
            if (lattice[y][x].snow) {
                // if solid, transfer it to tmpLattice
                if (lattice[y][x].solid) {
                    tmpLattice[y][x].solid = true;
                    tmpLattice[y][x].snow = true;
                    tmpLattice[y][x].erosion = lattice[y][x].erosion;
                }
                // snow is not solid, calculate movement
                else {
                    int snowVector = lattice[y][x].getVector();

                    // combine gravity and wind
                    if (rand() % GRAVITY == 0) {
                        if (snowVector == -1)
                            snowVector = 3;
                        else
                            snowVector = (snowVector + 3) / 2;
                    }

                    auto v = static_cast<Direction>(snowVector);
                    int xcoord, ycoord;
                    // get target cell
                    // if target cell is already a snow, dont change position
                    switch (v) {
                        case Direction::UP:
                            xcoord = x;
                            ycoord = y - 2;
                            break;

                        case Direction::UPLEFT:
                            xcoord = x - even;
                            ycoord = y - 1;
                            break;

                        case Direction::DOWNLEFT:
                            xcoord = x - even;
                            ycoord = y + 1;
                            break;

                        case Direction::DOWN:
                            xcoord = x;
                            ycoord = y + 2;
                            break;

                        case Direction::DOWNRIGHT:
                            xcoord = x + !even;
                            ycoord = y + 1;
                            break;

                        case Direction::UPRIGHT:
                            xcoord = x + !even;
                            ycoord = y - 1;
                            break;

                        default:
                            tmpLattice[y][x].snow = true;
                            continue;
                    }

                    // if target cell is solid, check for stable position
                    if (lattice[ycoord][xcoord].solid) {
                        // check for stable position
                        if (lattice[y + 2][x].solid)
                        // cannot fall through
                        {
                            if (lattice[y + 1][x - even].solid && lattice[y + 1][x + !even].solid) {
                                // position is stable
                                tmpLattice[y][x].solid = true;
                                tmpLattice[y][x].snow = true;
                                tmpLattice[y][x].erosion = COHESION;
                            } else if (lattice[y + 1][x + !even].solid)
                            // go left
                            {
                                if (lattice[y + 1][x - even].snow || tmpLattice[y + 1][x - even].snow) {
                                    tmpLattice[y][x].snow = true;
                                } else {
                                    tmpLattice[y + 1][x - even].snow = true;
                                }
                            } else
                            // go right
                            {
                                if (lattice[y + 1][x + !even].snow || tmpLattice[y + 1][x + !even].snow) {
                                    tmpLattice[y][x].snow = true;
                                } else {
                                    tmpLattice[y + 1][x + !even].snow = true;
                                }
                            }
                        }
                        // fall through
                        else {
                            if (lattice[y + 2][x].snow || tmpLattice[y + 2][x].snow) {
                                tmpLattice[y][x].snow = true;
                            }

                            else {
                                tmpLattice[y + 2][x].snow = true;
                            }
                        }
                    } else {
                        // move
                        if (xcoord > LATTICE_WIDTH / 2 - 1)
                            // if (xcoord > LATTICE_WIDTH / 2 - 1 && ycoord < LATTICE_HEIGHT * 10 / 6)
                            xcoord = 2;

                        // check if new place is not already taken
                        if (lattice[ycoord][xcoord].snow || tmpLattice[ycoord][xcoord].snow) {
                            auto left = shiftVector(xcoord, ycoord, v, true);
                            // check place on the left
                            if (!lattice[left.second][left.first].snow && !tmpLattice[left.second][left.first].snow && !lattice[left.second][left.first].solid) {
                                tmpLattice[left.second][left.first].snow = true;
                                continue;
                            }

                            // check place on the right
                            auto right = shiftVector(xcoord, ycoord, v, false);
                            if (!lattice[right.second][right.first].snow && !tmpLattice[right.second][right.first].snow && !lattice[right.second][right.first].solid) {
                                tmpLattice[right.second][right.first].snow = true;
                                continue;
                            } else {
                                tmpLattice[y][x].snow = true;
                                continue;
                            }

                        } else {
                            tmpLattice[ycoord][xcoord].snow = true;
                        }
                    }
                }
            }
        }
    }

    lattice = std::move(tmpLattice);
}

void latticeStep(Lattice& lattice) {
    // Lattice tmpLattice(LATTICE_HEIGHT, LATTICE_WIDTH);

    // wind step - FHP
    // propagation
    latticeStepPropagation(lattice);

    // wind collision
    for (int y = 2; y < LATTICE_HEIGHT * 2 + 2; y++) {
        // cout << "============Y " << y << " =============" << endl;
        for (int x = 1; x < LATTICE_WIDTH / 2 + 1; x++) {
            auto wind = lattice[y][x].wind;
            bool even = y % 2 == 0;  // compensation for matrix representation

            //   \ _
            //   /
            if (wind[0] && wind[2] && wind[4] && !wind[1] && !wind[3] && !wind[5]) {
                wind[0] = false;
                wind[2] = false;
                wind[4] = false;

                wind[1] = true;
                wind[3] = true;
                wind[5] = true;
                continue;
            }

            //   _ /
            //     \    

            if (!wind[0] && !wind[2] && !wind[4] && wind[1] && wind[3] && wind[5]) {
                wind[0] = true;
                wind[2] = true;
                wind[4] = true;

                wind[1] = false;
                wind[3] = false;
                wind[5] = false;
                continue;
            }

            //   \  
            //    \ 

            if (!wind[0] && !wind[2] && wind[4] && wind[1] && !wind[3] && !wind[5]) {
                if (rand() % 2 == 0) {
                    if (lattice[y - 1][x + !even].solid || lattice[y + 1][x - even].solid)  // rarely causes particles dissapear
                        continue;
                    lattice[y][x].wind[4] = false;
                    lattice[y][x].wind[1] = false;
                    lattice[y][x].wind[2] = true;
                    lattice[y][x].wind[5] = true;
                } else {
                    if (lattice[y - 2][x].solid || lattice[y + 2][x].solid)  // rarely causes particles dissapear
                        continue;
                    lattice[y][x].wind[4] = false;
                    lattice[y][x].wind[1] = false;
                    lattice[y][x].wind[0] = true;
                    lattice[y][x].wind[3] = true;
                }
                continue;
            }

            //    /
            //   /

            if ((!wind[0]) && wind[2] && (!wind[4]) && (!wind[1]) && (!wind[3]) && wind[5]) {
                if (rand() % 2 == 0) {
                    if (lattice[y - 1][x - even].solid || lattice[y + 1][x + !even].solid)  // rarely causes particles dissapear
                        continue;
                    lattice[y][x].wind[2] = false;
                    lattice[y][x].wind[5] = false;
                    lattice[y][x].wind[4] = true;
                    lattice[y][x].wind[1] = true;
                } else {
                    if (lattice[y - 2][x].solid || lattice[y + 2][x].solid)  // rarely causes particles dissapear
                        continue;
                    lattice[y][x].wind[2] = false;
                    lattice[y][x].wind[5] = false;
                    lattice[y][x].wind[0] = true;
                    lattice[y][x].wind[3] = true;
                }
                continue;
            }

            //   |
            //   |
            if (wind[0] && !wind[2] && !wind[4] && !wind[1] && wind[3] && !wind[5]) {
                if (rand() % 2 == 0) {
                    if (lattice[y - 1][x - even].solid || lattice[y + 1][x + !even].solid)  // rarely causes particles dissapear
                        continue;
                    lattice[y][x].wind[0] = false;
                    lattice[y][x].wind[3] = false;
                    lattice[y][x].wind[4] = true;
                    lattice[y][x].wind[1] = true;
                } else {
                    if (lattice[y - 1][x + !even].solid || lattice[y + 1][x - even].solid)  // rarely causes particles dissapear
                        continue;
                    lattice[y][x].wind[0] = false;
                    lattice[y][x].wind[3] = false;
                    lattice[y][x].wind[2] = true;
                    lattice[y][x].wind[5] = true;
                }
                continue;
            }
        }
    }

    // generate new wind particles
    for (int i = 0; i < wind_strength; i++) {
        int directions[2] = {4, 5};
        auto place = rand() % LATTICE_HEIGHT * 2;
        auto direction = directions[rand() % 2];
        lattice[place][1].wind[direction] = true;
    }

    if (snowing) {
        for (int i = 0; i < snow_strength; i++) {
            auto place = rand() % (LATTICE_WIDTH / 2 - 1);
            lattice[2][place].snow = true;
        }

        // for (int i = 0; i < snow_strength / 3; i++) {
        //     auto place = rand() % (LATTICE_HEIGHT * 2 - 1);
        //     if (place < LATTICE_HEIGHT * 9 / 5)
        //         lattice[place][2].snow = true;
        // }
    }
}

void setScene(const int which) {
    scene = vector<vector<bool>>(LATTICE_HEIGHT, vector<bool>(LATTICE_WIDTH, false));
    // ground
    scene[LATTICE_HEIGHT - 1] = vector<bool>(LATTICE_WIDTH, true);

    if (which == 0) {
    } else if (which == 1) {
        sceneFloatingStick(scene);
    } else if (which == 2) {
        sceneStick(scene);
    } else if (which == 3) {
        sceneHouse(scene);
    } else if (which == 4) {
        sceneOpenHouse(scene);
    } else if (which == 5) {
        sceneOpenWindow(scene);
    } else if (which == 6) {
        sceneWindBreakers1(scene);
    } else if (which == 7) {
        sceneWindBreakers2(scene);
    } else if (which == 8) {
        sceneWindBreaker(scene);
    }
    return;
}

void init() {
    setScene(0);
    wind_strength = LATTICE_HEIGHT / 3;
    snow_strength = LATTICE_WIDTH / 30;
}

void printHelp() {
    cout << "======================================\n";
    cout << "Cellular automata: snow & wind\n\n";
    cout << "In-simulation controls: \n";
    cout << "\tESC: terminate simulation and close the program\n\n";
    cout << "\t0-8: pick scene\n";
    cout << "\t+-: simulation speed\n";
    cout << "\tp: pause\n\n";
    cout << "\tw: stronger wind\n";
    cout << "\ts: gentler wind\n\n";
    cout << "\tw: stronger snowing\n";
    cout << "\ts: gentler snowing\n\n";
    cout << "\tspacebar: toggle snowing\n";
}

int main(int argc, char* argv[]) {
    assert(LATTICE_WIDTH != 0);
    assert(LATTICE_HEIGHT != 0);

    printHelp();

    int timeWarp = 11;
    int timeIterations = 1;
    bool pause = false;

    init();

    Mat latticeMat = Mat::zeros(Size(LATTICE_WIDTH / 2, LATTICE_HEIGHT * 2), CV_8U);
    Mat toShow(Size(LATTICE_WIDTH / 2 * 4 + LATTICE_WIDTH + 1, LATTICE_HEIGHT * 4 + 4), CV_8U);
    Lattice lattice(LATTICE_HEIGHT, LATTICE_WIDTH, scene);

    // show
    namedWindow("simulation", WINDOW_NORMAL);
    int a = 0;
    while (true) {
        if (!pause) {
            for (int i = 0; i < timeIterations; i++)
                latticeStep(lattice);

            toMat(lattice, latticeMat);
            toCartesian(latticeMat, toShow);
            imshow("simulation", toShow);
        }

        // keyboard control
        int k;
        if (!pause)
            k = waitKey(timeWarp);
        else {
            k = waitKey(0);
        }

        // esc
        if (k == 27) {
            break;
        }
        // spacebar
        else if (k == 32) {
            snowing = !snowing;
        }
        // +
        else if (k == 43) {
            if (timeWarp != 1) {
                timeWarp -= 10;
            } else {
                timeIterations += 5;
            }
        }
        // -
        else if (k == 45) {
            if (timeIterations != 1) {
                timeIterations -= 5;
            } else {
                timeWarp += 10;
            }
        }
        // p
        else if (k == 112) {
            pause = !pause;
        }
        // wind strength
        // w
        else if (k == 119) {
            if (wind_strength < LATTICE_HEIGHT - 5)
                wind_strength += 5;
        }
        // s
        else if (k == 115) {
            if (wind_strength > 5)
                wind_strength -= 5;
            else
                wind_strength = 0;
        }

        // snow strength
        // e
        else if (k == 101) {
            if (snow_strength < LATTICE_WIDTH - 5)
                snow_strength += 5;
        }
        // d
        else if (k == 100) {
            if (snow_strength > 5)
                snow_strength -= 5;
            else
                snow_strength = 0;
        }
        // scenes
        // 0
        else if (k == 48) {
            setScene(0);
        }
        // 1
        else if (k == 49) {
            setScene(1);
        }
        // 2
        else if (k == 50) {
            setScene(2);
        }
        // 3
        else if (k == 51) {
            setScene(3);
        }
        // 4
        else if (k == 52) {
            setScene(4);
        }
        // 5
        else if (k == 53) {
            setScene(5);
        }
        // 6
        else if (k == 54) {
            setScene(6);
        }
        // 7
        else if (k == 55) {
            setScene(7);
        }
        // 8
        else if (k == 56) {
            setScene(8);
        }
    }

    destroyAllWindows();
    return 0;
}
