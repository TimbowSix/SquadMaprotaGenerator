#ifndef RotaMode_h
#define RotaMode_h

typedef struct rotaMode rotaMode;

struct rotaMode
{
    int index;
    char *name;
    double *weightParams;
};

#endif