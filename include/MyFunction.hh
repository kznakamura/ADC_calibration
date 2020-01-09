#ifndef MYFUNCTION_HH
#define MYFUNCTION_HH

double integralWave(int* wave, double baseline, int integral_min, int integral_max);

double averageWave(int *wave, int average_min, int average_max, int skip_min=-1, int skip_max=-1);

#endif
