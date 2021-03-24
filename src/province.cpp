#include "province.h"

#include "country.h"
#include "uniform.h"
#include "model.h"

#define MAX(a,b) ((a)>(b)?(a):(b))

CProvince::CProvince(CModel* p_pModel, int p_nX, int p_nY, int p_nID)
  : m_nX(p_nX),
    m_nY(p_nY),
    m_nID(p_nID),
    m_pModel(p_pModel),
    m_nProtesters(0)
{
}

CProvince::~CProvince()
{
}

void CProvince::CreateSovereign()
{
  m_pCountry = new CCountry(m_pModel, this);
}

bool CProvince::IsCompatriot(CProvince& p_Province)
{
  return m_pCountry->GetID() == p_Province.GetCountry().GetID();
}

bool CProvince::IsCapital()
{
  return m_pCountry->GetCapital().GetID() == m_nID;
}

CProvince& CProvince::GetNeighbour(int k)
{
  return m_pModel->GetNeighbour(m_nX, m_nY, k);
}

void CProvince::SetCountry(CCountry* p_pCountry)
{
  m_pCountry->RemoveProvince(this);
  m_pCountry = p_pCountry;
}

void CProvince::AddCitizen(CCitizen* p_pCitizen)
{
  m_vCitizens.push_back(p_pCitizen);
}

void CProvince::AddProtester()
{
  m_nProtesters++;
}

void CProvince::RemoveProtester()
{
  m_nProtesters--;
}

bool CProvince::AllProtesting()
{
  return m_nProtesters == m_vCitizens.size();
}

double CProvince::GetProtestProportion()
{
  return (double) m_nProtesters / (double) m_vCitizens.size();
}

void CProvince::Broadcast()
{
  if (!m_pCountry->IsDemocracy())
    return;

  int i, iso1, iso2;
  vector<CProvince*> vpNeighbours;

  vpNeighbours.push_back(&GetNeighbour(WEST));
  vpNeighbours.push_back(&GetNeighbour(NORTH).GetNeighbour(WEST));
  vpNeighbours.push_back(&GetNeighbour(NORTH));
  vpNeighbours.push_back(&GetNeighbour(NORTH).GetNeighbour(EAST));
  vpNeighbours.push_back(&GetNeighbour(EAST));
  vpNeighbours.push_back(&GetNeighbour(SOUTH).GetNeighbour(EAST));
  vpNeighbours.push_back(&GetNeighbour(SOUTH));
  vpNeighbours.push_back(&GetNeighbour(SOUTH).GetNeighbour(WEST));
  vpNeighbours.push_back(this);

  for (i = 0; i < vpNeighbours.size(); ++i)
    {
      if (&vpNeighbours[i]->GetCountry() != m_pCountry)
	{
	  iso1 = m_pCountry->GetIsolation();
	  iso2 = vpNeighbours[i]->GetCountry().GetIsolation();

	  iso1 = MAX(iso1,iso2);
	}
      else
	iso1 = 0;

      if (CUniform::GetNextIntFromTo(0,100) > iso1)
	vpNeighbours[i]->ReceiveBroadcast(m_pCountry->IsDemocracy());
    }
}

void CProvince::ReceiveBroadcast(bool p_bDemocracy)
{
  int i;

  for (i = 0; i < m_vCitizens.size(); ++i)
    m_vCitizens[i]->ReceiveBroadcast(p_bDemocracy);
}

double CProvince::GetAverageAttitude()
{
  int i, s = 0;

  for (i = 0; i < m_vCitizens.size(); ++i)
    s += m_vCitizens[i]->GetAttitude();

  return (double) s / m_vCitizens.size();
}
