#include <stdio.h>
#include <math.h>

#include "utils.h"

double calcVectorLength(double *ele, int countOfElements)
{
    double tempSum = 0;
    for (int i = 0; i < countOfElements; i++)
    {
        tempSum += pow(ele[i], 2);
    }
    return sqrt(tempSum);
}

double calcVectorDotProduct(double *vecA, double *vecB, int vecLen)
{
    double sum = 0;
    for (int i = 0; i < vecLen; i++)
    {
        sum += vecA[i] * vecB[i];
    }
}