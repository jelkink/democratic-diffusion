#ifndef _UNIFORM_H
#define _UNIFORM_H

class CUniform
{
 public:
  static double GetDouble();
  static bool GetBoolean(double p_dProbability);
  static int GetNextIntFromTo(int p_nFrom, int p_nTo);
};

#endif
