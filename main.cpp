#include <iostream>
#include <vector>
#include "intrin.h"
// #include "opencv4/opencv2/opencv.hpp"
#include "opencv2/opencv.hpp"

#include "Lattice.h"

using namespace cv;
using namespace std;

#define LATTICE_WIDTH 900
#define LATTICE_HEIGHT 300
#define WIND_STRENGTH 25 // must be between 0-LATTICE_HEIGHT
#define SNOW_STRENGTH 25 // must be between 0-LATTICE_HEIGHT
#define COHESION 5
#define GRAVITY 12
vector<vector<bool>> scene{};
bool snowing = false;

void toCartesian(const Mat &hex, Mat &dst)
{
    auto stamp = [&](int x, int y, uint8_t state)
    {
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

    for (int y = 0; y < dst.size().height - 4; y += 2)
    {
        for (int x = ((y % 4) == 2) ? 3 : 0; x < dst.size().width - 2; x += 6)
        {
            auto state = hex.at<uint8_t>(y / 2, x / 6);
            stamp(x, y, state);
        }
    }
}

void toMat(const Lattice &lattice, Mat &latticeMat)
{
    for (int y = 2; y < LATTICE_HEIGHT * 2 + 2; y++)
    {
        for (int x = 1; x < LATTICE_WIDTH / 2 + 1; x++)
        {
            auto wind = lattice[y][x].wind;
            if (lattice[y][x].snow)
            {
                latticeMat.at<uint8_t>(y - 2, x - 1) = 255;
            }
            else if (wind[0] || wind[1] || wind[2] || wind[3] || wind[4] || wind[5])
            {
                latticeMat.at<uint8_t>(y - 2, x - 1) = 200;
            }
            else if (lattice[y][x].solid)
            {
                latticeMat.at<uint8_t>(y - 2, x - 1) = 150;
            }
            else
            {
                latticeMat.at<uint8_t>(y - 2, x - 1) = 0;
            }
        }
    }
}

size_t countParticles(Lattice &lattice)
{
    size_t count = 0;
    for (int y = 2; y < LATTICE_HEIGHT * 2 + 2; y++)
    {
        // cout << "============Y " << y << " =============" << endl;
        for (int x = 1; x < LATTICE_WIDTH / 2 + 1; x++)
        {
            for (int i = 0; i < 6; i++)
            {
                count += lattice[y][x].wind[i];
            }
        }
    }
    return count;
}

void latticeStepPropagation(Lattice &lattice)
{
    // flip vertical direction of the wind particles that are about to collide with the sky
    for (int x = 1; x < LATTICE_WIDTH / 2 + 1; x++)
    {
        auto wind = lattice[2][x].wind;

        if (wind[0])
        {
            lattice[2][x].wind[0] = false;
            lattice[2][x].wind[3] = true;
        }
        if (wind[1])
        {
            lattice[2][x].wind[1] = false;
            lattice[2][x].wind[2] = true;
        }
        if (wind[5])
        {
            lattice[2][x].wind[5] = false;
            lattice[2][x].wind[4] = true;
        }

        wind = lattice[3][x].wind;

        if (wind[0])
        {
            lattice[3][x].wind[0] = false;
            lattice[3][x].wind[3] = true;
        }
        if (wind[1])
        {
            lattice[3][x].wind[1] = false;
            lattice[3][x].wind[2] = true;
        }
        if (wind[5])
        {
            lattice[3][x].wind[5] = false;
            lattice[3][x].wind[4] = true;
        }
    }

    Lattice tmpLattice(LATTICE_HEIGHT, LATTICE_WIDTH, scene);

    for (int y = 2; y < LATTICE_HEIGHT * 2 + 2; y++)
    {
        for (int x = 1; x < LATTICE_WIDTH / 2 + 1; x++)
        {
            auto wind = lattice[y][x].wind;

            bool even = y % 2 == 0; // compensation for matrix representation

            int escapeVector = 0;
            // it should not blow inside
            if (!lattice[y][x].solid)
            {
                // wind propagation
                if (wind[0])
                {
                    // check if solid
                    if (lattice[y - 2][x].solid)
                    {
                        if (lattice[y - 1][x + !even].solid && !lattice[y - 1][x - even].solid)
                        {
                            tmpLattice[y + 1][x - even].wind[2] = true;
                            escapeVector = 2;
                        }
                        else if (!lattice[y - 1][x + !even].solid && lattice[y - 1][x - even].solid)
                        {
                            tmpLattice[y + 1][x + !even].wind[4] = true;
                            escapeVector = 4;
                        }
                        else
                        {
                            if (tmpLattice[y + 2][x].wind[3])
                                __debugbreak();
                            tmpLattice[y + 2][x].wind[3] = true;
                            escapeVector = 3;
                        }

                        // decrement counter of the hit particle if it was snow and assign escape vector
                        lattice[y - 2][x].decrementCounter(escapeVector);
                    }
                    // aint solid
                    else
                    {
                        tmpLattice[y - 2][x].wind[0] = true;
                    }
                }
                if (wind[1])
                {
                    // check if solid
                    if (lattice[y - 1][x - even].solid)
                    {
                        if (lattice[y + 1][x - even].solid && !lattice[y - 2][x].solid)
                        {
                            tmpLattice[y - 1][x + !even].wind[5] = true;
                            escapeVector = 5;
                        }
                        else if (!lattice[y + 1][x - even].solid && lattice[y - 2][x].solid)
                        {
                            tmpLattice[y + 1][x - even].wind[2] = true;
                            escapeVector = 2;
                        }
                        else
                        {
                            tmpLattice[y + 1][x + !even].wind[4] = true;
                            escapeVector = 4;
                        }

                        // decrement counter of the hit particle if it was snow and assign escape vector
                        lattice[y - 2][x].decrementCounter(escapeVector);
                    }
                    // aint solid
                    else
                    {
                        tmpLattice[y - 1][x - even].wind[1] = true;
                    }
                }
                if (wind[5])
                {
                    // check if solid
                    if (lattice[y - 1][x + !even].solid)
                    {
                        if (lattice[y + 1][x + !even].solid && !lattice[y - 2][x].solid)
                        {
                            tmpLattice[y - 1][x - even].wind[1] = true;
                            escapeVector = 1;
                        }
                        else if (!lattice[y + 1][x + !even].solid && lattice[y - 2][x].solid)
                        {
                            tmpLattice[y + 1][x + !even].wind[4] = true;
                            escapeVector = 4;
                        }
                        else
                        {
                            tmpLattice[y + 1][x - even].wind[2] = true;
                            escapeVector = 2;
                        }
                        // decrement counter of the hit particle if it was snow and assign escape vector
                        lattice[y - 2][x].decrementCounter(escapeVector);
                    }
                    // aint solid
                    else
                    {
                        tmpLattice[y - 1][x + !even].wind[5] = true;
                    }
                }
                if (wind[2])
                {
                    // check if solid
                    if (lattice[y + 1][x - even].solid)
                    {
                        if (lattice[y - 1][x - even].solid && !lattice[y + 2][x].solid)
                        {
                            tmpLattice[y + 1][x + !even].wind[4] = true;
                            escapeVector = 4;
                        }
                        else if (!lattice[y - 1][x - even].solid && lattice[y + 2][x].solid)
                        {
                            tmpLattice[y - 1][x - even].wind[1] = true;
                            escapeVector = 1;
                        }
                        else
                        {
                            tmpLattice[y - 1][x + !even].wind[5] = true;
                            escapeVector = 5;
                        }
                        // decrement counter of the hit particle if it was snow and assign escape vector
                        lattice[y - 2][x].decrementCounter(escapeVector);
                    }
                    // aint solid
                    else
                    {
                        tmpLattice[y + 1][x - even].wind[2] = true;
                    }
                }
                if (wind[3])
                {
                    // check if solid
                    if (lattice[y + 2][x].solid)
                    {
                        if (lattice[y + 1][x + !even].solid && !lattice[y + 1][x - even].solid)
                        {
                            tmpLattice[y - 1][x - even].wind[1] = true;
                            escapeVector = 1;
                        }
                        else if (!lattice[y + 1][x + !even].solid && lattice[y + 1][x - even].solid)
                        {
                            tmpLattice[y - 1][x + !even].wind[5] = true;
                            escapeVector = 5;
                        }
                        else
                        {
                            if (tmpLattice[y - 2][x].wind[0])
                                __debugbreak();
                            tmpLattice[y - 2][x].wind[0] = true;
                            escapeVector = 0;
                        }
                        // decrement counter of the hit particle if it was snow and assign escape vector
                        lattice[y - 2][x].decrementCounter(escapeVector);
                    }
                    // aint solid
                    else
                    {
                        tmpLattice[y + 2][x].wind[3] = true;
                    }
                }
                if (wind[4])
                {
                    // check if solid
                    if (lattice[y + 1][x + !even].solid)
                    {
                        if (lattice[y - 1][x + !even].solid && !lattice[y + 2][x].solid)
                        {
                            tmpLattice[y + 1][x - even].wind[2] = true;
                            escapeVector = 2;
                        }
                        else if (!lattice[y - 1][x + !even].solid && lattice[y + 2][x].solid)
                        {
                            tmpLattice[y - 1][x + !even].wind[5] = true;
                            escapeVector = 5;
                        }
                        else
                        {
                            tmpLattice[y - 1][x - even].wind[1] = true;
                            escapeVector = 1;
                        }
                        // decrement counter of the hit particle if it was snow and assign escape vector
                        lattice[y + 1][x + !even].decrementCounter(escapeVector);
                    }
                    // aint solid
                    else
                    {
                        tmpLattice[y + 1][x + !even].wind[4] = true;
                    }
                }
            }

            // snow propagation
            if (lattice[y][x].snow)
            {
                // if solid, transfer it to tmpLattice
                if (lattice[y][x].solid)
                {
                    tmpLattice[y][x].solid = true;
                    tmpLattice[y][x].snow = true;
                    tmpLattice[y][x].erosion = lattice[y][x].erosion;
                }
                // snow is not solid, calculate movement
                else
                {
                    // mean wind & gravity
                    int windParticleCount = 0;
                    int vectorSum = 0;
                    int snowVector = 0;
                    static int correction;
                    for (int i = 0; i < 6; i++)
                    {
                        if (wind[i])
                        {
                            windParticleCount++;
                            vectorSum += i;
                        }
                    }
                    // no wind, just gravity
                    if (windParticleCount == 0)
                    {
                        snowVector = 3;
                    }
                    // wind and gravity cancel each other out
                    else if (vectorSum == 0)
                    {
                        snowVector = -1;
                    }
                    // combine gravity and wind
                    else
                    {

                        // gravity should not be 6 times stronger than wind
                        if (rand() % GRAVITY == 0)
                        {
                            snowVector = (vectorSum / windParticleCount + 3) / 2;
                        }
                        else
                        {
                            // correction for integer math
                            if (windParticleCount > 1)
                            {
                                snowVector = (vectorSum + rand() % 2) / windParticleCount;
                            }
                            else
                            {
                                snowVector = vectorSum / windParticleCount;
                            }
                        }
                    }

                    auto v = static_cast<Direction>(snowVector);
                    int xcoord, ycoord;
                    // get target cell
                    switch (v)
                    {
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
                        xcoord = x;
                        ycoord = y;
                        break;
                    }

                    // if target cell is solid, check for stable position
                    if (lattice[ycoord][xcoord].solid)
                    {
                        // check for stable position
                        if (lattice[y + 2][x].solid)
                        // cannot fall through
                        {
                            if (lattice[y + 1][x - even].solid && lattice[y + 1][x + !even].solid)
                            {
                                // position is stable
                                tmpLattice[y][x].solid = true;
                                tmpLattice[y][x].snow = true;
                                tmpLattice[y][x].erosion = COHESION;
                            }
                            else if (lattice[y + 1][x + !even].solid)
                            // go left
                            {
                                tmpLattice[y + 1][x - even].snow = true;
                            }
                            else
                            // go right
                            {
                                tmpLattice[y + 1][x + !even].snow = true;
                            }
                        }
                        // fall through
                        else
                        {
                            tmpLattice[y + 2][x].snow = true;
                        }
                    }
                    else
                    {
                        // move
                        tmpLattice[ycoord][xcoord].snow = true;
                    }
                }
            }
        }
    }

    lattice = std::move(tmpLattice);
}

void latticeStep(Lattice &lattice)
{
    // Lattice tmpLattice(LATTICE_HEIGHT, LATTICE_WIDTH);

    // wind step - FHP
    // propagation
    latticeStepPropagation(lattice);

    // wind collision
    for (int y = 2; y < LATTICE_HEIGHT * 2 + 2; y++)
    {
        // cout << "============Y " << y << " =============" << endl;
        for (int x = 1; x < LATTICE_WIDTH / 2 + 1; x++)
        {
            auto wind = lattice[y][x].wind;
            bool even = y % 2 == 0; // compensation for matrix representation

            //   \ _
            //   /
            if (wind[0] && wind[2] && wind[4] && !wind[1] && !wind[3] && !wind[5])
            {
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

            if (!wind[0] && !wind[2] && !wind[4] && wind[1] && wind[3] && wind[5])
            {
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

            if (!wind[0] && !wind[2] && wind[4] && wind[1] && !wind[3] && !wind[5])
            {
                if (rand() % 2 == 0)
                {
                    if (lattice[y - 1][x + !even].solid || lattice[y + 1][x - even].solid) // rarely causes particles dissapear
                        continue;
                    lattice[y][x].wind[4] = false;
                    lattice[y][x].wind[1] = false;
                    lattice[y][x].wind[2] = true;
                    lattice[y][x].wind[5] = true;
                }
                else
                {
                    if (lattice[y - 2][x].solid || lattice[y + 2][x].solid) // rarely causes particles dissapear
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

            if ((!wind[0]) && wind[2] && (!wind[4]) && (!wind[1]) && (!wind[3]) && wind[5])
            {
                if (rand() % 2 == 0)
                {
                    if (lattice[y - 1][x - even].solid || lattice[y + 1][x + !even].solid) // rarely causes particles dissapear
                        continue;
                    lattice[y][x].wind[2] = false;
                    lattice[y][x].wind[5] = false;
                    lattice[y][x].wind[4] = true;
                    lattice[y][x].wind[1] = true;
                }
                else
                {
                    if (lattice[y - 2][x].solid || lattice[y + 2][x].solid) // rarely causes particles dissapear
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
            if (wind[0] && !wind[2] && !wind[4] && !wind[1] && wind[3] && !wind[5])
            {
                if (rand() % 2 == 0)
                {
                    if (lattice[y - 1][x - even].solid || lattice[y + 1][x + !even].solid) // rarely causes particles dissapear
                        continue;
                    lattice[y][x].wind[0] = false;
                    lattice[y][x].wind[3] = false;
                    lattice[y][x].wind[4] = true;
                    lattice[y][x].wind[1] = true;
                }
                else
                {
                    if (lattice[y - 1][x + !even].solid || lattice[y + 1][x - even].solid) // rarely causes particles dissapear
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
    // for (int i = 0; i < WIND_STRENGTH; i++)
    // {
    //     int directions[2] = {4, 5};
    //     auto place = rand() % LATTICE_HEIGHT * 2;
    //     auto direction = directions[rand() % 2];
    //     lattice[place][1].wind[direction] = true;
    // }

    if (snowing)
    {
        for (int i = 0; i < SNOW_STRENGTH; i++)
        {
            auto place = rand() % LATTICE_HEIGHT * 2;
            if (place < LATTICE_HEIGHT / 7 + 1)
                continue;
            lattice[place - LATTICE_HEIGHT / 7][1].snow = true;
        }
    }

    // lattice[50][25].wind[0] = true;
    // lattice[50][25].wind[1] = true;
    // lattice[50][25].wind[2] = true;
    // lattice[50][25].wind[3] = true;
    // lattice[50][25].wind[4] = true;
    // lattice[50][25].wind[5] = true;
}

void buildScene()
{
    scene = vector<vector<bool>>(LATTICE_HEIGHT, vector<bool>(LATTICE_WIDTH, false));

    scene[LATTICE_HEIGHT - 1] = vector<bool>(LATTICE_WIDTH, true);

    // for (int i = 0; i < LATTICE_HEIGHT / 5; i++)
    // {
    //     scene[LATTICE_HEIGHT / 5 * 2 + i][60 + i / 2] = true;
    //     scene[LATTICE_HEIGHT / 5 * 2 + i + 1][60 + i / 2] = true;
    // }

    // for (int i = 0; i < LATTICE_HEIGHT / 4; i++)
    // {
    //     // scene[i][80] = true;
    //     scene[LATTICE_HEIGHT - i - 1 - 10][120] = true;
    // }

    // for (int i = 0; i < LATTICE_HEIGHT / 5; i++)
    // {
    //     // scene[i][80] = true;
    //     scene[LATTICE_HEIGHT - i - 1][260] = true;
    // }

    for (int i = 0; i < LATTICE_HEIGHT; i++)
    {
        scene[i][1] = true;
        scene[i][2] = true;
        scene[i][LATTICE_WIDTH - 2] = true;
        scene[i][LATTICE_WIDTH - 1] = true;
    };

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
}

int main(int argc, char *argv[])
{
    assert(LATTICE_WIDTH != 0);
    assert(LATTICE_HEIGHT != 0);
    assert(WIND_STRENGTH > 0 && WIND_STRENGTH <= LATTICE_HEIGHT);
    buildScene();

    Mat latticeMat = Mat::zeros(Size(LATTICE_WIDTH / 2, LATTICE_HEIGHT * 2), CV_8U);
    Mat toShow(Size(LATTICE_WIDTH / 2 * 4 + LATTICE_WIDTH + 1, LATTICE_HEIGHT * 4 + 4), CV_8U);
    Lattice lattice(LATTICE_HEIGHT, LATTICE_WIDTH, scene);

    for (int y = 0; y < LATTICE_HEIGHT * 2 / 7; y++)
    {
        for (int x = 0; x < LATTICE_WIDTH / 10; x++)
        {
            lattice[y + LATTICE_HEIGHT * 2 / 7][x + LATTICE_WIDTH / 10].wind[0] = true;
            lattice[y + LATTICE_HEIGHT * 2 / 7][x + LATTICE_WIDTH / 10].wind[1] = true;
            lattice[y + LATTICE_HEIGHT * 2 / 7][x + LATTICE_WIDTH / 10].wind[2] = true;
            lattice[y + LATTICE_HEIGHT * 2 / 7][x + LATTICE_WIDTH / 10].wind[3] = true;
            lattice[y + LATTICE_HEIGHT * 2 / 7][x + LATTICE_WIDTH / 10].wind[4] = true;
            lattice[y + LATTICE_HEIGHT * 2 / 7][x + LATTICE_WIDTH / 10].wind[5] = true;
        }
    }

    // show
    namedWindow("image", WINDOW_NORMAL);
    // namedWindow("image2", WINDOW_NORMAL);
    int a = 0;
    while (true)
    {
        for (int i = 0; i < 1; i++)
            latticeStep(lattice);
        toMat(lattice, latticeMat);
        toCartesian(latticeMat, toShow);
        imshow("image", toShow);
        auto k = waitKey(1);
        if (k == 27)
        {
            break;
        }
        else if (k == 32)
        {
            snowing = !snowing;
        }
        a++;
    }

    destroyAllWindows();
    return 0;
}
