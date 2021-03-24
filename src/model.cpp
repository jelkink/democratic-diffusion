#include "model.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <exception>
#include "exception.h"
#include "uniform.h"

using std::cout;
using std::endl;
using std::ostringstream;
using std::fstream;
using std::ios;
using std::fixed;
using std::setprecision;
using std::exception;

extern "C" {
#include <R.h>
}

CModel::CModel(int p_nIterations, 
	       int p_nThinning, int p_nDetailThinning,
	       int p_nNumberOfLevels,
	       double p_dInitialProportionDemocratic,
	       double p_dRandomRevolutionChance,
	       double p_dCrossBorderChance,
	       int p_nRegimeDelay,
	       int p_nBroadcastEffect, int p_nCommunicationEffect,
	       int p_nFieldWidth, int p_nFieldHeight,
	       int p_nBorderMultiplier,
	       int p_bUseDecay, 
	       double p_dDecayStart, double p_dDecayStrength, 
	       int p_nIsoMean, int p_nIsoStd,
	       int p_nPopMean, int p_nPopStd,
	       int p_nAttMean, int p_nAttStd,
	       int p_nInGroupMean, int p_nInGroupStd,
	       int p_nOutGroupMean, int p_nOutGroupStd,
	       int p_nBatchID, int p_nRunID,
	       int p_nVerbose)
  : m_nRuns(1),
    m_nIterations(p_nIterations),
    m_nNumberOfLevels(p_nNumberOfLevels),
    m_dInitialProportionDemocratic(p_dInitialProportionDemocratic),
    m_dRandomRevolutionChance(p_dRandomRevolutionChance),
    m_dCrossBorderChance(p_dCrossBorderChance),
    m_nRegimeDelay(p_nRegimeDelay),
    m_nBroadcastEffect(p_nBroadcastEffect),
    m_nCommunicationEffect(p_nCommunicationEffect),
    m_nFieldWidth(p_nFieldWidth),
    m_nFieldHeight(p_nFieldHeight),
    m_nBorderMultiplier(p_nBorderMultiplier),
    m_bUseDecay(p_bUseDecay),
    m_dDecayStart(p_dDecayStart),
    m_dDecayStrength(p_dDecayStrength),
    m_nThinning(p_nThinning),
    m_nMapThinning(100),
    m_nDetailThinning(p_nDetailThinning),
    m_nBatchID(p_nBatchID),
    m_nRunID(p_nRunID)
{
  srand((unsigned) time(0));

  m_bVerbose = p_nVerbose == 1;

  m_pIsolationNormal = new CNormal(p_nIsoMean, p_nIsoStd);
  m_pPopulationNormal = new CNormal(p_nPopMean, p_nPopStd);
  m_pAttitudeNormal = new CNormal(p_nAttMean, p_nAttStd);
  m_pInGroupNormal = new CNormal(p_nInGroupMean, p_nInGroupStd);
  m_pOutGroupNormal = new CNormal(p_nOutGroupMean, p_nOutGroupStd);

  m_nDotProgress = p_nIterations / 100;
  m_nLineProgress = p_nIterations / 10;
}

CModel::~CModel()
{
  int i;

  delete m_pIsolationNormal;
  delete m_pPopulationNormal;
  delete m_pAttitudeNormal;
  delete m_pInGroupNormal;
  delete m_pOutGroupNormal;

  for (i = 0; i < m_vProvinces.size(); ++i)
    delete m_vProvinces[i];

  for (i = 0; i < m_vCitizens.size(); ++i)
    delete m_vCitizens[i];

  for (i = 0; i < m_vCountries.size(); ++i)
    delete m_vCountries[i];

  delete m_pdConnectionMatrix;

  m_vCitizens.clear();
  m_vCountries.clear();
  m_vProvinces.clear();

  WriteCountryState(ACTION_CLOSE);
}

void CModel::Run(double *out_Moran, double *out_Proportion,
		 double *out_ConnMatrix, double *out_AttMap,
		 int *out_Population, int *out_Details,
		 int *n_Countries)
{
  if (m_bVerbose) Rprintf("Setup...\n");

  if (out_Moran == NULL || out_Proportion == NULL || out_ConnMatrix == NULL
      || out_Population == NULL || out_Details == NULL) 
    Rprintf("Null pointer passed to RRun() (%x; %x; %x; %x; %x)!\n",
	    out_Moran, out_Proportion, out_ConnMatrix, out_Population,
	    out_Details);

  m_R_out_Moran = out_Moran;
  m_R_out_Proportion = out_Proportion;
  m_R_out_ConnMatrix = out_ConnMatrix;
  m_R_out_AttMap = out_AttMap;
  m_R_out_Population = out_Population;
  m_R_out_Details = out_Details;

  Setup();

  if (m_bVerbose) Rprintf("Saving number of countries... (%d)\n",
			  m_vCountries.size());
  *n_Countries = m_vCountries.size();

  if (m_bVerbose) Rprintf("Start simulation... (%d iterations)\n", m_nIterations);

  for (m_nCurrentIteration = 1;
       m_nCurrentIteration <= m_nIterations; 
       ++m_nCurrentIteration)
    Step();
}

void CModel::Step()
{
  int i,t,n;

  WriteCountryState(ACTION_WRITE);

  ReportToR();

  for (i = 0; i < m_vCountries.size(); ++i)
    m_vCountries[i]->UpdateIsolation();

  for (i = 0; i < m_vCitizens.size() / 10; ++i)
    m_vCitizens[CUniform::GetNextIntFromTo(0, m_vCitizens.size()-1)]
      ->DetermineCommunication();

  ShuffleCitizens();

  for (i = 0; i < m_vCitizens.size(); ++i)
    m_vCitizens[i]->DetermineProtest();

  m_vCountries[CUniform::GetNextIntFromTo(0, m_vCountries.size()-1)]
    ->GetCapital().Broadcast();

  for (i = 0; i < m_vCountries.size(); ++i)
    m_vCountries[i]->TestRevolution();

  PrintProgress();
}

void CModel::ShuffleCitizens()
{
  int i,a,b;
  CCitizen* pSwap;
  int nCitizens = m_vCitizens.size() - 1;

  for (i = 0; i < nCitizens / 2; ++i)
    {
      a = CUniform::GetNextIntFromTo(0, nCitizens);
      b = CUniform::GetNextIntFromTo(0, nCitizens);

      pSwap = m_vCitizens[a];
      m_vCitizens[a] = m_vCitizens[b];
      m_vCitizens[b] = pSwap;
    }
}

void CModel::Setup()
{
  int i;

  ClearDataStorage();
  m_nCoups = 0;
  m_nRevolutions = 0;

  if (m_bVerbose) Rprintf("Creating provinces...\n");
  CreateProvinces();
  if (m_bVerbose) Rprintf("Creating countries...\n");
  CreateCountries();
  if (m_bVerbose) Rprintf("Creating citizens...\n");
  CreateCitizens();
  if (m_bVerbose) Rprintf("Counting citizen...\n");
  CountCitizens();

  if (m_bVerbose) Rprintf("Creating connection matrix...\n");
  CreateConnectionMatrix();

  if (m_bVerbose) Rprintf("Storing connection matrix...\n");
  SaveConnectionMatrix();

  if (m_bVerbose) Rprintf("Resetting democracy memory...\n");
  for (i = 0; i < 80; ++i)
    m_nrgDemocracyMemory[i] = 0;

  WriteCountryState(ACTION_OPEN);
}

void CModel::ClearDataStorage()
{
  int i;

  for (i = 0; i < m_vCitizens.size(); ++i)
    delete m_vCitizens[i];

  for (i = 0; i < m_vCountries.size(); ++i)
    delete m_vCountries[i];

  for (i = 0; i < m_vProvinces.size(); ++i)
    delete m_vProvinces[i];

  m_vCitizens.clear();
  m_vCountries.clear();
  m_vProvinces.clear();
}

void CModel::CreateProvinces()
{
  int x,y,i = 0;
  CProvince *pNewProvince;

  for (x = 0; x < m_nFieldWidth; ++x)
    for (y = 0; y < m_nFieldHeight; ++y)
      {
	pNewProvince = new CProvince(this, x, y, ++i);
	pNewProvince->CreateSovereign();
	m_vProvinces.push_back(pNewProvince);
      }
}

void CModel::CreateCountries()
{
  int i, x, y, k;

  for (i = 0; i < m_nFieldWidth * m_nFieldHeight * m_nBorderMultiplier; ++i)
    {
      x = rand() % m_nFieldWidth;
      y = rand() % m_nFieldHeight;
      k = rand() % 4;

      GetNeighbour(x,y,k).GetCountry().Conquer(GetProvince(x,y));
    }

  for (i = 0; i < m_vProvinces.size(); ++i)
    if (m_vProvinces[i]->IsCapital())
      m_vCountries.push_back(&m_vProvinces[i]->GetCountry());

  for (i = 0; i < m_vCountries.size(); ++i)
    m_vCountries[i]->SetID(i);
}

void CModel::CountCitizens()
{
  int i;

  for (i = 0; i < m_vCountries.size(); ++i)
    m_R_out_Population[i] = m_vCountries[i]->GetPopulation();
}

void CModel::CreateCitizens()
{
  int i, j, nPopulation;
  CCitizen* pNewCitizen;

  for (i = 0; i < m_vProvinces.size(); ++i)
    {
      nPopulation = m_pPopulationNormal->GetNextIntFrom(1);

      for (j = 0; j < nPopulation; ++j)
	{
	  pNewCitizen = new CCitizen(this, m_vProvinces[i]);
	  pNewCitizen->SetAttitude(m_pAttitudeNormal
				   ->GetNextIntFromTo(0,m_nNumberOfLevels));
	  pNewCitizen->SetOutGroupThreshold(m_pOutGroupNormal
					    ->GetNextIntFrom(0));
	  pNewCitizen->SetInGroupThreshold(m_pInGroupNormal
					   ->GetNextIntFrom(0));
	  m_vProvinces[i]->GetCountry().AddCitizen(pNewCitizen);
	  m_vProvinces[i]->AddCitizen(pNewCitizen);
	  m_vCitizens.push_back(pNewCitizen);
	}
    }
}

void CModel::CreateConnectionMatrix()
{
  int i,j,k,id1,id2,s;
  int nCountries = m_vCountries.size();

  // Create empty connection matrix
  m_pdConnectionMatrix = new double[nCountries * nCountries];
  for (i = 0; i < nCountries; ++i)
    for (j = 0; j < nCountries; ++j)
      m_pdConnectionMatrix[i * nCountries + j] = 0.0;

  // Set all borders to one
  for (i = 0; i < m_vProvinces.size(); ++i)
    {
      id1 = m_vProvinces[i]->GetCountry().GetID();

      id2 = m_vProvinces[i]->GetNeighbour(NORTH).GetCountry().GetID();
      if (id1 != id2)
	{
	  m_pdConnectionMatrix[id1 * nCountries + id2] = 1;
	  m_pdConnectionMatrix[id2 * nCountries + id1] = 1;
	}

      id2 = m_vProvinces[i]->GetNeighbour(WEST).GetCountry().GetID();
      if (id1 != id2)
	{
	  m_pdConnectionMatrix[id1 * nCountries + id2] = 1;
	  m_pdConnectionMatrix[id2 * nCountries + id1] = 1;
	}
    }

  // Standardize to have rows add up to one
  for (i = 0; i < nCountries; ++i)
    {
      s = 0;

      for (j = 0; j < nCountries; ++j)
	s += (int) m_pdConnectionMatrix[i * nCountries + j];

      for (j = 0; j < nCountries; ++j)
	m_pdConnectionMatrix[i * nCountries + j] /= (double) s;
    }
}

void CModel::SaveConnectionMatrix()
{
  int i,j;
  int nCountries = m_vCountries.size();

  for (i = 0; i < nCountries * nCountries; ++i)
    m_R_out_ConnMatrix[i] = m_pdConnectionMatrix[i];
}

void CModel::PrintMap()
{
  int x,y,i,t;

  for (y = 0; y < m_nFieldHeight; ++y)
    {
      for (x = 0; x < m_nFieldWidth; ++x)
	if (m_vProvinces.at(x * m_nFieldHeight + y)
	    ->GetCountry().IsDemocracy())
	  //cout << (char) 178 << (char) 178;
	  cout << "##";
	else
	  //cout << (char) 219 << (char) 219;
	  cout << "..";

      cout << endl;
    }

  i = m_nCurrentIteration / 100;

  m_nrgDemocracyMemory[i-1] = GetProportionDemocratic();

  for (t = 0; t < i; t += 2)
    {
      for (x = 0; x < m_nrgDemocracyMemory[t] * 40; ++x)
	cout << " ";
      cout << "*" << endl;
    }

  for (x = 0; x < m_nFieldWidth; ++x)
    cout << "--";

  cout << " " << fixed << setprecision(2) << 
    ((double) m_nCurrentIteration * 100 / 8000) << " %";

  cout << endl;
}

CProvince& CModel::GetProvince(int x, int y)
{
  if (x > m_nFieldWidth - 1 || x < 0
      || y > m_nFieldHeight - 1 || y < 0)
    {
      ostringstream strMessage;
      strMessage << "Attempt to get non-existing province at ("
		 << x << "," << y << ")";
      throw CException(strMessage.str());
    }

  return *m_vProvinces.at(x * m_nFieldHeight + y);
}

CProvince& CModel::GetNeighbour(int x, int y, int k)
{
  if (k > 3 || k < 0 
      || x > m_nFieldWidth - 1 || x < 0
      || y > m_nFieldHeight - 1 || y < 0)
    {
      ostringstream strMessage;
      strMessage << "Neighbour requested of impossible province - x,y,k: " 
		 << x << "," << y << "," << k;
      throw CException(strMessage.str());
    }

  switch (k)
    {
    case NORTH:
      if (y == 0) y = m_nFieldHeight - 1;
      else --y;
      break;

    case SOUTH:
      if (y == m_nFieldHeight - 1) y = 0;
      else ++y;
      break;

    case WEST:
      if (x == 0) x = m_nFieldWidth - 1;
      else --x;
      break;

    case EAST:
      if (x == m_nFieldWidth - 1) x = 0;
      else ++x;
      break;
    }

  return GetProvince(x,y);
}

void CModel::AddRevolution(bool p_bCoup)
{
  p_bCoup ? m_nCoups++ : m_nRevolutions++;
}

double CModel::GetAverageAttitude()
{
  int i;
  double p = 0.0;

  for (i = 0; i < m_vCitizens.size(); ++i)
    p += m_vCitizens[i]->GetAttitude();

  return p / (double) (m_vCitizens.size() * m_nNumberOfLevels);
}

double CModel::GetProportionDemocratic()
{
  int i;
  double p = 0.0;

  for (i = 0; i < m_vCountries.size(); ++i)
    p += (m_vCountries[i]->IsDemocracy() ? 1 : 0) 
      * m_vCountries[i]->GetCitizens().size();

  return p / (double) m_vCitizens.size();
}

double CModel::GetProportionRevolutions()
{
  if (m_nRevolutions + m_nCoups > 0)
    return (double) m_nRevolutions 
      / (double) (m_nRevolutions + m_nCoups);
  else
    return 0.0;
}

double CModel::GetMoranI()
{
  int i,j;
  double num1 = 0, num2 = 0, nom1 = 0, nom2 = 0;
  double avg, demi, demj;
  int nCountries = m_vCountries.size();

  avg = GetProportionDemocratic();

  for (i = 0; i < nCountries; ++i)
    {
      demi = (m_vCountries[i]->IsDemocracy() ? 1 : 0);
      nom2 += (demi - avg) * (demi - avg);

      for (j = 0; j < nCountries; ++j)
	{
	  demj = (m_vCountries[j]->IsDemocracy() ? 1 : 0);
	  nom1 += m_pdConnectionMatrix[i * nCountries + j];
	  num1 += m_pdConnectionMatrix[i * nCountries + j] 
	    * (demi - avg) * (demj - avg);
	}
    }

  num2 = nCountries;

  if (nom2 < 0.0000001) // All countries are either democratic or non-democratic
    nom2 = 0.0000001;

  return num1 / nom1 * num2 / nom2;
}

void CModel::PrintProgress()
{
  if (m_nCurrentIteration % m_nDotProgress == 0 && m_bVerbose)
    if (m_nCurrentIteration % m_nLineProgress == 0)
      Rprintf("(%d)", m_nCurrentIteration);
    else
      Rprintf(".");
}

void CModel::WriteCountryState(int action)
{
  int i;
  char czFilename[50];

  switch (action)
    {
    case ACTION_OPEN:
      sprintf(czFilename, "output/batch%d_%d_countries.data", 
	      m_nBatchID, m_nRunID);
      if (m_bVerbose)
	Rprintf("Opening country state file (%s)...\n", czFilename);
      m_fCountryFile = fopen(czFilename, "w");
      if (m_bVerbose && m_fCountryFile == NULL)
	Rprintf("WARNING! Country state file was not opened.\n");
      break;

    case ACTION_CLOSE:
      fclose(m_fCountryFile);
      break;

    case ACTION_WRITE:
      if (m_fCountryFile != NULL)
	for (i = 0; i < m_vCountries.size(); ++i)
	  fputs((m_vCountries[i]->IsDemocracy() ? "1" : "0"), m_fCountryFile);
      fputs("\n", m_fCountryFile);
      break;
    }
}

void CModel::ReportToR()
{
  int i, t, n;
  int nVariables = 8;

  try 
    {
      if (m_nCurrentIteration % m_nThinning == 0)
	{
	  m_R_out_Moran[m_nCurrentIteration / m_nThinning] = GetMoranI();
	  m_R_out_Proportion[m_nCurrentIteration / m_nThinning] 
	    = GetProportionDemocratic();
	}

      if (m_nCurrentIteration % m_nDetailThinning == 0)
	{
	  t = m_nCurrentIteration / m_nDetailThinning - 1;
	  n = m_vCountries.size();

	  for (i = 0; i < n; ++i)
	    {
	      m_R_out_Details[t * n * nVariables + i] 
		= (int) m_vCountries[i]->IsDemocracy();
	      m_R_out_Details[t * n * nVariables + n + i] 
		= m_vCountries[i]->GetProtesting();
	      m_R_out_Details[t * n * nVariables + 2 * n + i]
		= m_vCountries[i]->GetAvgAttitude();
	      m_R_out_Details[t * n * nVariables + 3 * n + i]
		= m_vCountries[i]->GetCoupCount();
	      m_R_out_Details[t * n * nVariables + 4 * n + i]
		= m_vCountries[i]->GetRevolutionCount();
	      m_R_out_Details[t * n * nVariables + 5 * n + i]
		= m_vCountries[i]->GetRegimeAge();
	      m_R_out_Details[t * n * nVariables + 6 * n + i]
		= m_vCountries[i]->GetLastRegimeChange();
	      m_R_out_Details[t * n * nVariables + 7 * n + i]
		= m_vCountries[i]->GetInitialRegime();
	    }
	}

      if (m_nCurrentIteration % m_nMapThinning == 0) 
	{
	  int nProvinces = m_vProvinces.size();
	  for (i = 0; i < nProvinces; ++i)
	    m_R_out_AttMap[nProvinces*(m_nCurrentIteration/m_nMapThinning) 
			   + i - nProvinces] 
	      = m_vProvinces[i]->GetAverageAttitude();	
	}
    }
  catch (exception& e)
    {
      Rprintf("Exception %s caught\n", e.what());
    }
}
