#include "normal.h"

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)>(b)?(b):(a))

#include <cmath>
#include "uniform.h"

CNormal::CNormal(double p_dMean, double p_dStandardDeviation)
  : m_dMean(p_dMean),
    m_dStandardDeviation(p_dStandardDeviation),
    m_bHasStoredValue(false)
{
}

CNormal::~CNormal()
{
}

int CNormal::GetNextIntFromTo(int p_nFrom, int p_nTo)
{
  int nValue = (int) round(GetNextDouble());
  return MIN(MAX(p_nFrom, nValue), p_nTo);
}

int CNormal::GetNextIntFrom(int p_nFrom)
{
  int nValue = (int) round(GetNextDouble());
  return MAX(p_nFrom, nValue);
}

double CNormal::GetNextDouble()
{
  if (m_bHasStoredValue)
    {
      m_bHasStoredValue = false;
      return m_dStoredValue;
    }
  else
    {
      // See G.E.P. Box and Mervin E. Muller, "A note on the generation
      // of random normal deviates", The Annals of Mathematical Statistics,
      // 1958 (29:2), pp. 610-611.

      double u1 = CUniform::GetDouble();
      double u2 = CUniform::GetDouble();

      m_dStoredValue = pow(-2 * log(u1), .5) * sin(6.28318530717959 * u2) 
	* m_dStandardDeviation + m_dMean;
      m_bHasStoredValue = true;

      return pow(-2 * log(u2), .5) * cos(6.28318530717959 * u1)
	* m_dStandardDeviation + m_dMean;
    }
}
