#include "uniform.h"

#include <cstdlib>

double CUniform::GetDouble()
{
  return (double) rand() / (double) RAND_MAX;
}

bool CUniform::GetBoolean(double p_dProbability)
{
  return CUniform::GetDouble() < p_dProbability;
}

int CUniform::GetNextIntFromTo(int p_nFrom, int p_nTo)
{
  return rand() % (p_nTo - p_nFrom + 1) + p_nFrom;
}
