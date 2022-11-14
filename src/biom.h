#ifndef biom_h
#define biom_h

typedef struct biom biom;
struct biom
{
    char *mapName;
    double *values;
    int len;
};

/**
 * reads bioms file an returns a array of biom structs and the len of that array
 */
int getBioms(biom **bioms);
/**
 * clears mem of bioms
 */
void delBioms(biom *b, int len);

/**
 * print out all biom values
 */
void printBioms(biom *bioms, int len);

#endif