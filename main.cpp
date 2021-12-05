#include <iostream>
#include <vector>
#include "opencv4/opencv2/opencv.hpp"

#include "LatticeCell.h"

using namespace cv;
using namespace std;

using Lattice = vector<vector<LatticeCell>>;

#define LATTICE_WIDTH 100
#define LATTICE_HEIGHT 50
#define WIND_STRENGTH 1 // must be between 0-LATTICE_HEIGHT

void toCartesian(const Mat &hex, Mat &dst)
{
    for (int y = 0; y < dst.size().height - 1; y += 3)
    {
        for (int x = ((y % 6) == 3) ? 2 : 0; x < dst.size().width - 2; x += 4)
        {
            auto state = hex.at<uint8_t>(y / 3, x / 4);

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
        }
    }
}

void toMat(const Lattice &lattice, Mat &latticeMat)
{
    for (int y = 1; y < LATTICE_HEIGHT + 1; y++)
    {
        for (int x = 1; x < LATTICE_WIDTH + 1; x++)
        {
            auto wind = lattice[y][x].wind;
            if (wind[0] || wind[1] || wind[2] || wind[3] || wind[4] || wind[5])
            {
                latticeMat.at<uint8_t>(y - 1, x - 1) = 255;
            }
            else
            {
                latticeMat.at<uint8_t>(y - 1, x - 1) = 0;
            }
        }
    }
}

void latticeStep(Lattice &lattice)
{
    Lattice tmpLattice(LATTICE_HEIGHT + 2, vector<LatticeCell>(LATTICE_WIDTH + 2));

    // wind step - FHP
    // propagation
    for (int y = 1; y < LATTICE_HEIGHT + 1; y++)
    {
        for (int x = 1; x < LATTICE_WIDTH + 1; x++)
        {
            auto wind = lattice[y][x].wind;
            if (wind[0])
            {
                tmpLattice[y][x + 1].wind[3] = true;
            }
            if (wind[1])
            {
                tmpLattice[y + 1][x + 1].wind[4] = true;
            }
            if (wind[2])
            {
                tmpLattice[y - 1][x+1].wind[5] = true;
            }
            if (wind[3])
            {
                tmpLattice[y][x - 1].wind[0] = true;
            }
            if (wind[4])
            {
                tmpLattice[y - 1][x - 1].wind[1] = true;
            }
            if (wind[5])    
            {
                tmpLattice[y + 1][x-1].wind[2] = true;
            }
        }
    }

    // collision
    for (int y = 1; y < LATTICE_HEIGHT + 1; y++)
    {
        for (int x = 1; x < LATTICE_WIDTH + 1; x++)
        {
            auto wind = tmpLattice[y][x].wind;
            //   \ _
            //   /
            if (wind[0] && wind[2] && wind[4] && !wind[1] && !wind[3] && !wind[5])
            {
                break;
            }

            //   _ /
            //     \    

            if (!wind[0] && !wind[2] && !wind[4] && wind[1] && wind[3] && wind[5])
            {
                break;
            }

            //    /
            //   /

            if (!wind[0] && !wind[2] && wind[4] && wind[1] && !wind[3] && !wind[5])
            {
                tmpLattice[y][x].wind[4] = false;
                tmpLattice[y][x].wind[1] = false;
                if (rand() % 2 == 0)
                {
                    tmpLattice[y][x].wind[2] = true;
                    tmpLattice[y][x].wind[5] = true;
                }
                else
                {
                    tmpLattice[y][x].wind[0] = true;
                    tmpLattice[y][x].wind[3] = true;
                }
                break;
            }

            //   \  
            //    \ 

            if ((!wind[0]) && wind[2] && (!wind[4]) && (!wind[1]) && (!wind[3]) && wind[5])
            {
                tmpLattice[y][x].wind[2] = false;
                tmpLattice[y][x].wind[5] = false;
                if (rand() % 2 == 0)
                {
                    tmpLattice[y][x].wind[4] = true;
                    tmpLattice[y][x].wind[1] = true;
                }
                else
                {
                    tmpLattice[y][x].wind[0] = true;
                    tmpLattice[y][x].wind[3] = true;
                }
                break;
            }

            //   --
            if (wind[0] && !wind[2] && !wind[4] && !wind[1] && wind[3] && !wind[5])
            {
                tmpLattice[y][x].wind[0] = false;
                tmpLattice[y][x].wind[3] = false;
                if (rand() % 2 == 0)
                {
                    tmpLattice[y][x].wind[4] = true;
                    tmpLattice[y][x].wind[1] = true;
                }
                else
                {
                    tmpLattice[y][x].wind[2] = true;
                    tmpLattice[y][x].wind[5] = true;
                }
                break;
            }
            bool tmpWind[6]{};
            tmpWind[3] = wind[0];
            tmpWind[4] = wind[1];
            tmpWind[5] = wind[2];
            tmpWind[0] = wind[3];
            tmpWind[1] = wind[4];
            tmpWind[2] = wind[5];
            memcpy(tmpLattice[y][x].wind, tmpWind, sizeof(tmpWind));
        }
    }
    lattice = tmpLattice;

    // generate new wind particles
    for (int i = 0; i < WIND_STRENGTH; i++)
    {
        static int directions[3] = {1,0,5};
        auto place = rand() % LATTICE_HEIGHT;
        lattice[place][1].wind[directions[rand()%3]] = true;
        // lattice[place][1].wind[0] = true;
    }

    // restore the border
    lattice[0] = vector<LatticeCell>(LATTICE_WIDTH + 2);
    lattice[LATTICE_HEIGHT + 1] = vector<LatticeCell>(LATTICE_WIDTH + 2);
    for (int i = 0; i < LATTICE_HEIGHT; i++)
    {
        lattice[i + 1][0] = LatticeCell{};
        lattice[i + 1][LATTICE_WIDTH + 1] = LatticeCell{};
    }
}

int main(int argc, char *argv[])
{
    assert(LATTICE_WIDTH != 0);
    assert(LATTICE_HEIGHT != 0);
    assert(WIND_STRENGTH > 0 && WIND_STRENGTH <= LATTICE_HEIGHT);

    Lattice lattice(LATTICE_HEIGHT + 2, vector<LatticeCell>(LATTICE_WIDTH + 2)); // creating one cell boundary, so I dont have to check if the cell is at the border
    Mat latticeMat = Mat::zeros(Size(LATTICE_WIDTH, LATTICE_HEIGHT), CV_8U);
    Mat toShow((LATTICE_HEIGHT)*3 + 1, (LATTICE_WIDTH)*4 + 2, CV_8U);

    // show
    namedWindow("image", WINDOW_NORMAL);
    while (true)
    {
        latticeStep(lattice);
        toMat(lattice, latticeMat);
        toCartesian(latticeMat, toShow);
        imshow("image", toShow);
        auto k = waitKey(100);
        if (k == 27)
        {
            break;
        }
    }

    destroyAllWindows();
    return 0;
}