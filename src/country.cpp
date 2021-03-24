#include "country.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "province.h"
#include "model.h"
#include "uniform.h"

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)>(b)?(b):(a))

using std::find;
using std::cout;
using std::endl;
using std::left;
using std::right;
using std::setw;

CCountry::CCountry(CModel* p_pModel, CProvince* p_pProvince)
  : m_bOriginalRegime(true),
    m_nDelay(0),
    m_nLastRevolution(0),
    m_nRegimeAge(0),
    m_dCoupChance(0),
    m_nCoupCount(0),
    m_nRevolutionCount(0),
    m_pModel(p_pModel)
{
  // Countries are created by the province which becomes the capital
  m_vProvinces.push_back(p_pProvince);
  m_pCapital = p_pProvince;
  m_nID = p_pProvince->GetID();
  m_nIsolation = p_pModel->GetIsolationNormal().GetNextIntFromTo(0,100);
  m_bDemocracy = CUniform::GetBoolean(p_pModel->
				      GetInitialProportionDemocratic());
}

CCountry::~CCountry()
{
}

void CCountry::UpdateIsolation()
{
  if (m_bDemocracy)
    m_nIsolation = 0;
  else
    {
      m_nIsolation = m_nIsolation + CUniform::GetNextIntFromTo(-1,1);
      m_nIsolation = MAX(0,MIN(100,m_nIsolation));
    }
}

void CCountry::TestRevolution()
{
  m_nRegimeAge++;

  if (m_pModel->UseDecay() && !m_bOriginalRegime)
    {
      m_dCoupChance = m_pModel->GetDecayStart() 
	                 * exp(m_pModel->GetDecayStrength() 
			       * m_nRegimeAge);
      m_dCoupChance = MAX(m_dCoupChance, 
			  m_pModel->GetRandomRevolutionChance());
    }
  else
    m_dCoupChance = m_pModel->GetRandomRevolutionChance();

  bool bChange = false;

  if (m_pCapital->AllProtesting() && !m_nDelay)
    {
      bChange = true;
      m_pModel->AddRevolution(false);
      m_nLastRevolution = 1;
      m_nRevolutionCount++;
    }
  else if (CUniform::GetBoolean(m_dCoupChance))
    {
      bChange = true;
      m_pModel->AddRevolution(true);
      m_nLastRevolution = 2;
      m_nCoupCount++;
    }
  
  if (bChange)
    {
      int i;

      m_bDemocracy = !m_bDemocracy;

      for (i = 0; i < m_vCitizens.size(); ++i)
	m_vCitizens[i]->SetProtesting(false);

      m_nDelay = m_pModel->GetRegimeDelay();
      m_bOriginalRegime = false;
      m_nRegimeAge = 0;
    }
  
  if (m_nDelay)
    --m_nDelay;
}

CProvince& CCountry::GetCapital()
{ 
  return *m_pCapital;
}

void CCountry::SetCapital(CProvince& p_Capital) 
{
  m_pCapital = &p_Capital;
}

void CCountry::Conquer(CProvince& p_Province)
{
  // If the province to be conquered is not already part of this country
  // and the country of the province would still be connected after conquering
  // then set the country of the province to the this one
  // and add the province to this country's provinces
  if (p_Province.GetCountry().GetID() != m_nID
      && p_Province.GetCountry().CheckConnectednessWithoutProvince(p_Province))
    {
      if (p_Province.GetCountry().GetProvinces().size() == 1)
	delete &p_Province.GetCountry();

      p_Province.SetCountry(this);
      m_vProvinces.push_back(&p_Province);
    }
}

void CCountry::RemoveProvince(CProvince* p_pProvince)
{
  vector<CProvince*>::iterator iter;

  // Find this province and remove it
  iter = find(m_vProvinces.begin(), m_vProvinces.end(), p_pProvince);
  if (iter != m_vProvinces.end())
    m_vProvinces.erase(iter);
  
  // If the province to be removed is the capital, and there are other 
  // provinces left, set a new, random capital
  if (m_pCapital == p_pProvince && m_vProvinces.size())
    m_pCapital = m_vProvinces[rand() % m_vProvinces.size()];
}

bool CCountry::CheckConnectednessWithoutProvince(CProvince& p_RemovedProvince)
{
  vector<int> vCheckedProvinces;

  // If this is the only province in the country, no further checking needed
  if (IsAtom())
    return true;

  // Recursively find out whether provinces remaining are still connected
  CheckCWPRecursive(*m_pCapital, p_RemovedProvince, vCheckedProvinces);

  return vCheckedProvinces.size() == m_vProvinces.size() - 1;
}

void CCountry::CheckCWPRecursive(CProvince& p_CurrentProvince, 
				 CProvince& p_RemovedProvince, 
				 vector<int>& p_vCheckedProvinces)
{
  int k, nCurrentID;

  nCurrentID = p_CurrentProvince.GetID();

  // If the current province is a compatriot of the capital
  // and it is not the province to be removed
  // and it is not among the provinces already checked
  // then add to provinces checked 
  // and check all four neighbouring provinces
  if (m_pCapital->IsCompatriot(p_CurrentProvince)
      && nCurrentID != p_RemovedProvince.GetID()
      && find(p_vCheckedProvinces.begin(), p_vCheckedProvinces.end(),
	      nCurrentID) == p_vCheckedProvinces.end())
    {
      p_vCheckedProvinces.push_back(nCurrentID);

      // Check all four neighbouring provinces, unless all provinces of
      // this country have already been checked
      for (k = 0;
	   k < 4 && p_vCheckedProvinces.size() < m_vProvinces.size() - 1; 
	   ++k)
	CheckCWPRecursive(p_CurrentProvince.GetNeighbour(k), 
			  p_RemovedProvince, p_vCheckedProvinces);
    }
}

int CCountry::GetPopulation()
{
  int i, s = 0;

  for (i = 0; i < m_vProvinces.size(); ++i)
    s += m_vProvinces[i]->GetCitizens().size();

  return s;
}

int CCountry::GetProtesting()
{
  int i, s = 0;

  for (i = 0; i < m_vCitizens.size(); ++i)
    s += (int) m_vCitizens[i]->IsProtesting();

  return s;
}

int CCountry::GetAvgAttitude()
{
  int i, s = 0;

  for (i = 0; i < m_vCitizens.size(); ++i)
    s += m_vCitizens[i]->GetAttitude();

  return (int) round((double) s / (double) m_vCitizens.size());
}
