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

void normalize(double *arr, int len, double *sum)
{
    if (len <= 0)
    {
        return;
    }

    double s = 0;
    if (sum != NULL)
    {
        s = *sum;
    }
    else
    {
        for (int i = 0; i < len; i++)
        {
            s += arr[i];
        }
    }
    if (s == 0)
    {
        double temp = 1 / len;
        for (int i = 0; i < len; i++)
        {
            arr[i] = temp;
        }
    }
    else
    {
        for (int i = 0; i < len; i++)
        {
            arr[i] /= s;
        }
    }
}

double sigmoid(double x, double slope, double shift)
{
    double arg = slope * (x + shift);
    return 1 / (1 + exp(-arg));
}