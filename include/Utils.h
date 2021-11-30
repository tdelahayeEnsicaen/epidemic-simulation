#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <memory.h>

float genFloat()
{
    return ((float)rand() / RAND_MAX);
}

typedef struct 
{
    int x;
    int y;
} Point;

const Point DIRECTIONS[8] = 
{ 
    { +1, +1 }, { +1, +0 }, 
    { +1, -1 }, { +0, +1 }, 
    { +0, -1 }, { -1, +1 }, 
    { -1, +0 }, { -1, -1 } 
};

/**
 * @brief Fill an array with the 8 directions ordered randomly.
 * 
 * @param dest 
 */
void genDirList(Point dest[])
{
    memset(dest, 0, sizeof(Point) * 8);

    for (int i = 0; i < 8; i++)
    {
        int random = rand() % (8 - i);
        int destIndex;

        for (destIndex = 0; random != 0 || dest[destIndex].x != 0 || dest[destIndex].y != 0; destIndex++)
        {
            if (dest[destIndex].x == 0 && dest[destIndex].y == 0)
            {
                random--;
            }
        }

        dest[destIndex] = DIRECTIONS[i];
    }
}

float min(float a, float b)
{
    return a > b ? b : a;
}

float max(float a, float b)
{
    return a > b ? a : b;
}

#endif