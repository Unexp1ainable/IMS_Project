#include <iostream>
#include <vector>
#include "opencv4/opencv2/opencv.hpp"

#include "Lattice.h"

using namespace cv;
using namespace std;

#define LATTICE_WIDTH 100
#define LATTICE_HEIGHT 50
#define WIND_STRENGTH 1 // must be between 0-LATTICE_HEIGHT
vector<vector<bool>> scene{};

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
            if (wind[0] || wind[1] || wind[2] || wind[3] || wind[4] || wind[5])
            {
                latticeMat.at<uint8_t>(y - 2, x - 1) = 122;
            }
            else if (lattice[y][x].solid)
            {
                latticeMat.at<uint8_t>(y - 2, x - 1) = 255;
            }
            else
            {
                latticeMat.at<uint8_t>(y - 2, x - 1) = 0;
            }
        }
    }
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
    }

    Lattice tmpLattice(LATTICE_HEIGHT, LATTICE_WIDTH, scene);

    for (int y = 2; y < LATTICE_HEIGHT * 2 + 2; y++)
    {
        for (int x = 1; x < LATTICE_WIDTH / 2 + 1; x++)
        {
            auto ff = tmpLattice[96][48].wind[4];
            auto wind = lattice[y][x].wind;

            bool even = y % 2 == 0; // compensation for matrix representation

            if (wind[0])
            {
                // check if solid
                if (lattice[y - 2][x].solid)
                {
                    if (lattice[y - 1][x + !even].solid && !lattice[y - 1][x - even].solid)
                    {
                        tmpLattice[y + 1][x - even].wind[5] = true;
                    }
                    else if (!lattice[y - 1][x + !even].solid && lattice[y - 1][x - even].solid)
                    {
                        tmpLattice[y - 1][x + !even].wind[1] = true;
                    }
                    else
                    {
                        tmpLattice[y + 2][x].wind[0] = true;
                    }
                }
                // aint solid
                else
                {
                    tmpLattice[y - 2][x].wind[3] = true;
                }
            }
            if (wind[1])
            {
                // check if solid
                if (lattice[y - 1][x - even].solid)
                {
                    if (lattice[y + 1][x - even].solid && !lattice[y - 2][x].solid)
                    {
                        tmpLattice[y - 1][x + !even].wind[2] = true;
                    }
                    else if (!lattice[y + 1][x - even].solid && lattice[y - 2][x].solid)
                    {
                        tmpLattice[y + 2][x].wind[0] = true;
                    }
                    else
                    {
                        tmpLattice[y + 1][x + !even].wind[1] = true;
                    }
                }
                // aint solid
                else
                {
                    tmpLattice[y - 1][x - even].wind[4] = true;
                }
            }
            if (wind[5])
            {
                // check if solid
                if (lattice[y - 1][x + !even].solid)
                {
                    if (lattice[y + 1][x + !even].solid && !lattice[y - 2][x].solid)
                    {
                        tmpLattice[y - 1][x - even].wind[4] = true;
                    }
                    else if (!lattice[y + 1][x + !even].solid && lattice[y - 2][x].solid)
                    {
                        tmpLattice[y + 2][x].wind[0] = true;
                    }
                    else
                    {
                        tmpLattice[y + 1][x - even].wind[5] = true;
                    }
                }
                // aint solid
                else
                {
                    tmpLattice[y - 1][x + !even].wind[2] = true;
                }
            }
            if (wind[2])
            {
                // check if solid
                if (lattice[y + 1][x - even].solid)
                {
                    if (lattice[y - 1][x - even].solid && !lattice[y + 2][x].solid)
                    {
                        tmpLattice[y + 1][x + !even].wind[1] = true;
                    }
                    else if (!lattice[y - 1][x - even].solid && lattice[y + 2][x].solid)
                    {
                        tmpLattice[y - 2][x].wind[3] = true;
                    }
                    else
                    {
                        tmpLattice[y - 1][x + !even].wind[2] = true;
                    }
                }
                // aint solid
                else
                {
                    tmpLattice[y + 1][x - even].wind[5] = true;
                }
            }
            if (wind[3])
            {
                // check if solid
                if (lattice[y + 2][x].solid)
                {
                    if (lattice[y + 1][x + !even].solid && !lattice[y + 1][x - even].solid)
                    {
                        tmpLattice[y - 1][x - even].wind[4] = true;
                    }
                    else if (!lattice[y + 1][x + !even].solid && lattice[y + 1][x - even].solid)
                    {
                        tmpLattice[y - 1][x + !even].wind[2] = true;
                    }
                    else
                    {
                        tmpLattice[y - 2][x].wind[3] = true;
                    }
                }
                // aint solid
                else
                {
                    tmpLattice[y + 2][x].wind[0] = true;
                }
            }
            if (wind[4])
            {
                // check if solid
                if (lattice[y + 1][x + !even].solid)
                {
                    if (lattice[y - 1][x + !even].solid && !lattice[y + 2][x].solid)
                    {
                        tmpLattice[y + 1][x - even].wind[5] = true;
                    }
                    else if (!lattice[y - 1][x + !even].solid && lattice[y + 2][x].solid)
                    {
                        tmpLattice[y - 2][x].wind[3] = true;
                    }
                    else
                    {
                        tmpLattice[y - 1][x - even].wind[4] = true;
                    }
                }
                // aint solid
                else
                {
                    tmpLattice[y + 1][x + !even].wind[1] = true;
                }
            }
        }
    }
    lattice = tmpLattice;
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
            // cout << x << "\n";
            auto wind = lattice[y][x].wind;
            //   \ _
            //   /
            if (wind[0] && wind[2] && wind[4] && !wind[1] && !wind[3] && !wind[5])
            {
                continue;
            }

            //   _ /
            //     \    

            if (!wind[0] && !wind[2] && !wind[4] && wind[1] && wind[3] && wind[5])
            {
                continue;
            }

            //   \  
            //    \ 

            if (!wind[0] && !wind[2] && wind[4] && wind[1] && !wind[3] && !wind[5])
            {
                lattice[y][x].wind[4] = false;
                lattice[y][x].wind[1] = false;
                if (rand() % 2 == 0)
                {
                    lattice[y][x].wind[2] = true;
                    lattice[y][x].wind[5] = true;
                }
                else
                {
                    lattice[y][x].wind[0] = true;
                    lattice[y][x].wind[3] = true;
                }
                continue;
            }

            //    /
            //   /

            if ((!wind[0]) && wind[2] && (!wind[4]) && (!wind[1]) && (!wind[3]) && wind[5])
            {
                lattice[y][x].wind[2] = false;
                lattice[y][x].wind[5] = false;
                if (rand() % 2 == 0)
                {
                    lattice[y][x].wind[4] = true;
                    lattice[y][x].wind[1] = true;
                }
                else
                {
                    lattice[y][x].wind[0] = true;
                    lattice[y][x].wind[3] = true;
                }
                continue;
            }

            //   |
            //   |
            if (wind[0] && !wind[2] && !wind[4] && !wind[1] && wind[3] && !wind[5])
            {
                lattice[y][x].wind[0] = false;
                lattice[y][x].wind[3] = false;
                if (rand() % 2 == 0)
                {
                    lattice[y][x].wind[4] = true;
                    lattice[y][x].wind[1] = true;
                }
                else
                {
                    lattice[y][x].wind[2] = true;
                    lattice[y][x].wind[5] = true;
                }
                continue;
            }

            bool tmpWind[6]{};
            tmpWind[3] = wind[0];
            tmpWind[4] = wind[1];
            tmpWind[5] = wind[2];
            tmpWind[0] = wind[3];
            tmpWind[1] = wind[4];
            tmpWind[2] = wind[5];
            memcpy(lattice[y][x].wind, tmpWind, sizeof(tmpWind));
        }
    }

    //generate new wind particles
    // for (int i = 0; i < WIND_STRENGTH; i++)
    // {
    //     int directions[2] = {4, 5};
    //     auto place = rand() % LATTICE_HEIGHT * 2;
    //     auto direction = directions[rand() % 2];
    //     lattice[place][1].wind[direction] = true;
    // }

    // for (int i = 0; i < WIND_STRENGTH; i++)
    // {
    //     int directions[2] = {1, 2};
    //     auto place = rand() % LATTICE_HEIGHT * 2;
    //     auto direction = directions[rand() % 2];
    //     lattice[place][49].wind[direction] = true;
    //     // lattice[place][1].wind[0] = true;
    // }

    // TESTING
    // lattice[60][40].wind[1] = true;
    // lattice[20][20].wind[4] = true;
    //
    // lattice[20][40].wind[2] = true;
    // lattice[60][20].wind[5] = true;
    //
    // lattice[40][40].wind[0] = true;
    // lattice[20][40].wind[3] = true;
    //
    // lattice[62][40].wind[1] = true;
    // lattice[62][20].wind[5] = true;
    // lattice[2][30].wind[3] = true;
    //
    // lattice[2][40].wind[2] = true;
    // lattice[2][20].wind[4] = true;
    // lattice[62][30].wind[0] = true;

    // lattice[50][25].wind[0] = true;
    // lattice[50][25].wind[1] = true;
    // lattice[50][25].wind[2] = true;
    // lattice[50][25].wind[3] = true;
    lattice[50][25].wind[4] = true;
    // lattice[50][25].wind[5] = true;
}

void buildScene()
{
    scene = vector<vector<bool>>(LATTICE_HEIGHT, vector<bool>(LATTICE_WIDTH, false));

    scene[LATTICE_HEIGHT - 1] = vector<bool>(LATTICE_WIDTH, true);
    scene[39][81] = true;
    scene[40][81] = true;
    // scene[41][80] = true;

    // scene[39][16] = true;
    // scene[40][16] = true;
    // scene[40][17] = true;

    // scene[7][80] = true;
    // scene[7][81] = true;
    // scene[8][81] = true;

    // scene[7][15] = true;
    // scene[7][16] = true;
    // scene[8][15] = true;

    // scene[9][48] = true;
    // scene[9][49] = true;
    // scene[9][47] = true;

    // scene[40][78] = true;
    // scene[39][77] = true;
    // scene[39][79] = true;
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

    // show
    namedWindow("image", WINDOW_NORMAL);
    // namedWindow("image2", WINDOW_NORMAL);
    int a = 0;
    while (true)
    {
        auto b = lattice[96][48].wind[1];
        latticeStep(lattice);
        toMat(lattice, latticeMat);
        toCartesian(latticeMat, toShow);
        imshow("image", toShow);
        auto k = waitKey(100);
        if (k == 27)
        {
            break;
        }
        a++;
    }
    // toMat(lattice, latticeMat);
    // toCartesian(latticeMat, toShow);
    // imshow("image", latticeMat);
    // imshow("image2", toShow);
    // waitKey(0);

    destroyAllWindows();
    return 0;
}