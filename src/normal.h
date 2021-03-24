#ifndef _NORMAL_H
#define _NORMAL_H

class CNormal
{
 public:
  CNormal(double p_dMean, double p_dStandardDeviation);
  virtual ~CNormal();

  int GetNextIntFromTo(int p_nFrom, int p_nTo);
  int GetNextIntFrom(int p_nFrom);
  double GetNextDouble();

  double GetMean() { return m_dMean; }
  double GetStandardDeviation() { return m_dStandardDeviation; }

 private:
  double m_dMean;
  double m_dStandardDeviation;
  bool m_bHasStoredValue;
  double m_dStoredValue;
};

#endif
