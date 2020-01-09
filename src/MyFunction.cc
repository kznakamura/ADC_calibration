#include "../include/MyFunction.hh"

using namespace std;

double integralWave(int* wave, double baseline, int integral_min, int integral_max){
  double integral = 0;
  for(int smp=integral_min; smp<integral_max; smp++){
    integral += ((double)wave[smp] - baseline);
  }
  return integral;
}

double averageWave(int* wave, int average_min, int average_max, int skip_min, int skip_max){
  double wave_sum = 0;
  double average_length = average_max - average_min;
  for(int smp=average_min; smp<average_max; smp++){
    if(smp>=skip_min && smp<skip_max){
      average_length -= 1.0;
      continue;
    }
    wave_sum += (double)wave[smp];
  }
  return wave_sum/average_length;
}
