#ifndef _PROVINCE_H
#define _PROVINCE_H

#include <vector>

using std::vector;

class CModel;
class CCountry;
class CCitizen;

class CProvince
{
 public:
  CProvince(CModel* p_pModel, int p_nX, int p_nY, int p_nID);
  virtual ~CProvince();

  void CreateSovereign();
  bool IsCompatriot(CProvince& p_Province);
  bool IsCapital();
  CProvince& GetNeighbour(int k);
  void AddProtester();
  void RemoveProtester();
  bool AllProtesting();
  double GetProtestProportion();
  void AddCitizen(CCitizen* p_pCitizen);
  void Broadcast();
  void ReceiveBroadcast(bool p_bDemocracy);

  CCountry& GetCountry() { return *m_pCountry; }
  int GetID() { return m_nID; }
  int GetX() { return m_nX; }
  int GetY() { return m_nY; }
  double GetAverageAttitude();
  vector<CCitizen*>& GetCitizens() { return m_vCitizens; }

  void SetCountry(CCountry* p_pCountry);

 private:
  int m_nX, m_nY;
  int m_nID;
  int m_nProtesters;
  vector<CCitizen*> m_vCitizens;
  CCountry* m_pCountry;
  CModel* m_pModel;
};

#endif
