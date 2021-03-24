#include "citizen.h"

#include <cmath>
#include "province.h"
#include "model.h"
#include "country.h"
#include "uniform.h"

extern "C" {
#include <R.h>
}

using std::abs;

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

CCitizen::CCitizen(CModel* p_pModel, CProvince* p_pProvince)
  : m_pModel(p_pModel),
    m_pProvince(p_pProvince),
    m_nInGroupThreshold(0),
    m_nOutGroupThreshold(0),
    m_bProtesting(false)
{
}

CCitizen::~CCitizen()
{
}

void CCitizen::DetermineCommunication()
{
  CProvince* pProvince = 0;
  CProvince* pNeighbour;
  CCitizen* pCitizen;
  int i,iso1,iso2;
  double k,t;
  int nSumIsolation = 0;

  k = CUniform::GetDouble();

  for (i = 0; i < 4 && pProvince == 0; ++i)
    {
      pNeighbour = &m_pProvince->GetNeighbour(i);
      if (&pNeighbour->GetCountry() != &m_pProvince->GetCountry())
	{
	  iso1 = pNeighbour->GetCountry().GetIsolation();
	  iso2 = m_pProvince->GetCountry().GetIsolation();

	  t = MAX(iso1,iso2) * m_pModel->GetCrossBorderChance() / 400;
	}
      else
	t = m_pModel->GetCrossBorderChance() / 4;

      if (k < t)
	pProvince = pNeighbour;
      else
	k -= t;
    }

  if (pProvince == 0) 
    pProvince = m_pProvince;

  if (pProvince->GetCitizens().size() > 0)
    {
      pCitizen = pProvince->GetCitizens()
	[CUniform::GetNextIntFromTo(0, pProvince->GetCitizens().size() - 1)];

      Communicate(pCitizen);
    }
}

void CCitizen::Communicate(CCitizen* p_pCitizen)
{
  int nDistance, nAttitude1, nAttitude2, nMaxLevel;
  int nCommEffect = m_pModel->GetCommunicationEffect();

  nAttitude1 = GetAttitude();
  nAttitude2 = p_pCitizen->GetAttitude();
  nMaxLevel = m_pModel->GetNumberOfLevels() - 1;

  nDistance = abs(nAttitude1 - nAttitude2);

  if (nDistance < m_nInGroupThreshold)
    nAttitude2 > nAttitude1 ? nAttitude1 += nCommEffect 
      : nAttitude1 -= nCommEffect;
  else if (nDistance > m_nOutGroupThreshold)
    nAttitude2 > nAttitude1 ? nAttitude1 -= nCommEffect 
      : nAttitude1 += nCommEffect;

  SetAttitude(MAX(0,MIN(nMaxLevel,nAttitude1)));
}

void CCitizen::ReceiveBroadcast(bool p_bDemocracy)
{
  if (p_bDemocracy)
    m_nAttitude += m_pModel->GetBroadcastEffect();
  else
    m_nAttitude -= m_pModel->GetBroadcastEffect();

  if (m_nAttitude < 0)
    m_nAttitude = 0;
  else 
    {
      int nMaxLevel = m_pModel->GetNumberOfLevels() - 1;

      if (m_nAttitude > nMaxLevel)
	m_nAttitude = nMaxLevel;
    }
}

void CCitizen::DetermineProtest()
{
  double dProtestProportion, dAttitudeProportion;
  bool bDemocracy;

  dProtestProportion = m_pProvince->GetProtestProportion();
  dAttitudeProportion = (double) m_nAttitude 
    / (double) (m_pModel->GetNumberOfLevels() - 1);
  bDemocracy = m_pProvince->GetCountry().IsDemocracy();

  if (bDemocracy && dAttitudeProportion <= dProtestProportion)
    SetProtesting(true);
  else if (!bDemocracy && (1.0 - dAttitudeProportion) <= dProtestProportion)
    SetProtesting(true);
  else
    SetProtesting(false);
}

void CCitizen::SetInGroupThreshold(int p_nInGroupThreshold)
{
  if (p_nInGroupThreshold > 0)
    if (p_nInGroupThreshold < m_nOutGroupThreshold)
      m_nInGroupThreshold = p_nInGroupThreshold;
    else
      m_nInGroupThreshold = m_nOutGroupThreshold;
  else
    m_nInGroupThreshold = 0;
}

void CCitizen::SetOutGroupThreshold(int p_nOutGroupThreshold)
{
  if (p_nOutGroupThreshold > m_nInGroupThreshold)
    if (p_nOutGroupThreshold > 0)
      m_nOutGroupThreshold = p_nOutGroupThreshold;
    else
      m_nOutGroupThreshold = 0;
  else
    m_nOutGroupThreshold = m_nInGroupThreshold;
}

void CCitizen::SetProtesting(bool p_bProtesting)
{
  if (p_bProtesting && !m_bProtesting)
    {
      m_bProtesting = true;
      m_pProvince->AddProtester();
    }
  else if (!p_bProtesting && m_bProtesting)
    {
      m_bProtesting = false;
      m_pProvince->RemoveProtester();
    }
}
