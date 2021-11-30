#ifndef UTILS_H
#define UTILS_H

float genFloat();

typedef struct 
{
    int x;
    int y;
} Point;

/**
 * @brief Fill an array with the 8 directions ordered randomly.
 * 
 * @param dest 
 */
void genDirList(Point dest[]);

float min(float a, float b);

float max(float a, float b);

#endif