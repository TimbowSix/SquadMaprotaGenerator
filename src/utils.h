#ifndef utils_h
#define utils_h

/**
 * calc len of vector aka array
 */
double calcVectorLength(double *ele, int countOfElements);

/**
 * calc dot product of two vectors
 */
double calcVectorDotProduct(double *vecA, double *vecB, int vecLen);

/**
 * normalize an array of double values
 * takes sum if not NULL
 */
void normalize(double *arr, int len, double *sum);

/**
 * calculate y values of the sigmoid function
 */
double sigmoid(double x, double slope, double shift);

#endif