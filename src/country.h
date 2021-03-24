#ifndef _COUNTRY_H
#define _COUNTRY_H

#include <vector>

using std::vector;

class CProvince;
class CCitizen;
class CModel;

class CCountry
{
 public:
  CCountry(CModel* p_pModel, CProvince* p_pProvince);
  virtual ~CCountry();

  void Conquer(CProvince& p_Province);
  void UpdateIsolation();
  bool OwnsProvince(CProvince& p_Province);
  bool IsAtom() { return m_vProvinces.size() == 1; }
  void RemoveProvince(CProvince* p_pProvince);
  CProvince& GetCapital();
  void SetCapital(CProvince& p_Capital);
  int GetSize() { return m_vProvinces.size(); }
  void AddCitizen(CCitizen* p_pCitizen) { m_vCitizens.push_back(p_pCitizen); }
  void TestRevolution();

  int GetID() { return m_nID; }
  bool IsDemocracy() { return m_bDemocracy; }
  int GetIsolation() { return m_nIsolation; }
  int GetDelay() { return m_nDelay; }
  vector<CProvince*>& GetProvinces() { return m_vProvinces; }
  vector<CCitizen*>& GetCitizens() { return m_vCitizens; }

  void SetID(int p_nID) { m_nID = p_nID; }
  void SetDemocracy(bool p_bDemocracy) { m_bDemocracy = p_bDemocracy; }
  void SetIsolation(int p_nIsolation) { m_nIsolation = p_nIsolation; }

  int GetPopulation();
  int GetProtesting();
  int GetAvgAttitude();
  int GetCoupCount() { return m_nCoupCount; }
  int GetRevolutionCount() { return m_nRevolutionCount; }
  int GetRegimeAge() { return m_nRegimeAge; }
  int GetLastRegimeChange() { return m_nLastRevolution; }
  int GetInitialRegime() { return m_bOriginalRegime ? 1 : 0; }

 private:
  bool CheckConnectednessWithoutProvince(CProvince& p_RemovedProvince);
  void CheckCWPRecursive(CProvince& p_CurrentProvince, 
			 CProvince& p_RemovedProvince, 
			 vector<int>& p_vCheckedProvinces);

  int m_nID;
  bool m_bDemocracy;
  bool m_bOriginalRegime;
  int m_nIsolation;
  int m_nDelay;
  int m_nLastRevolution;
  int m_nRegimeAge;
  int m_nCoupCount;
  int m_nRevolutionCount;
  double m_dCoupChance;
  vector<CProvince*> m_vProvinces;
  vector<CCitizen*> m_vCitizens;
  CProvince* m_pCapital;
  CModel* m_pModel;
};

#endif
