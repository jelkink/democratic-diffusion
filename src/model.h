#ifndef _MODEL_H
#define _MODEL_H

#include <vector>
#include "citizen.h"
#include "country.h"
#include "province.h"
#include "normal.h"

#define NORTH 0
#define WEST 1
#define SOUTH 2
#define EAST 3

#define ACTION_OPEN 0
#define ACTION_CLOSE 1
#define ACTION_WRITE 2

using std::vector;

class CModel
{
 public:
  CModel(int p_nIterations,
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
	 int p_nVerbose);
  virtual ~CModel();

  void Run(double *out_Moran, double *out_Proportion, 
	   double *out_ConnMatrix, double *out_AttMap, 
	   int *out_Population, int *out_Details, 
	   int *n_Countries);

  CProvince& GetProvince(int x, int y);
  CProvince& GetNeighbour(int x, int y, int k);
  void AddRevolution(bool p_bCoup);

  int GetCurrentRun() { return m_nRunID; } // deprecated
  int GetRunID() { return m_nRunID; }
  int GetCurrentIteration() { return m_nCurrentIteration; }
  int GetCurrentSession() { return m_nBatchID; } // deprecated
  int GetBatchID() { return m_nBatchID; }
  int GetNumberOfLevels() { return m_nNumberOfLevels; }
  int GetRegimeDelay() { return m_nRegimeDelay; }
  int GetBroadcastEffect() { return m_nBroadcastEffect; }
  int GetCommunicationEffect() { return m_nCommunicationEffect; }
  double GetInitialProportionDemocratic() { 
    return m_dInitialProportionDemocratic; }
  double GetRandomRevolutionChance() { return m_dRandomRevolutionChance; }
  double GetCrossBorderChance() { return m_dCrossBorderChance; }
  CNormal& GetIsolationNormal() { return *m_pIsolationNormal; }
  bool UseDecay() { return m_bUseDecay; }
  double GetDecayStart() { return m_dDecayStart; }
  double GetDecayStrength() {return m_dDecayStrength; }

 private:
  void Setup();
  void Step();
  void ShuffleCitizens();
  void ClearDataStorage();
  void CreateProvinces();
  void CreateCountries();
  void CountCitizens();
  void CreateCitizens();
  void CreateConnectionMatrix();
  void SaveConnectionMatrix();
  void PrintMap();
  double GetAverageAttitude();
  double GetProportionDemocratic();
  double GetProportionRevolutions();
  double GetMoranI();
  void PrintProgress();
  void WriteCountryState(int action);
  void ReportToR();

  // Run parameters
  int m_nRuns;
  int m_nIterations;
  int m_nBatchID;
  int m_nRunID;
  int m_nCurrentIteration;
  bool m_bVerbose;
  int m_nThinning;
  int m_nMapThinning;
  int m_nDetailThinning;
  int m_nDotProgress;
  int m_nLineProgress;

  FILE *m_fCountryFile;

  // Model parameters
  int m_nFieldWidth;
  int m_nFieldHeight;
  int m_nBorderMultiplier;
  int m_nNumberOfLevels;
  int m_nRegimeDelay;
  int m_nBroadcastEffect;
  int m_nCommunicationEffect;
  double m_dInitialProportionDemocratic;
  double m_dRandomRevolutionChance;
  double m_dCrossBorderChance;
  bool m_bUseDecay;
  double m_dDecayStart;
  double m_dDecayStrength;

  // Model statistics
  int m_nCoups;
  int m_nRevolutions;
  double m_nrgDemocracyMemory[80];

  // R output
  double* m_R_out_Moran;
  double* m_R_out_Proportion;
  double* m_R_out_ConnMatrix;
  double* m_R_out_AttMap;
  int* m_R_out_Population;
  int* m_R_out_Details;

  // Distribution
  CNormal* m_pIsolationNormal;
  CNormal* m_pPopulationNormal;
  CNormal* m_pInGroupNormal;
  CNormal* m_pOutGroupNormal;
  CNormal* m_pAttitudeNormal;

  // Internal data storage
  vector<CCitizen*> m_vCitizens;
  vector<CCountry*> m_vCountries;
  vector<CProvince*> m_vProvinces;
  double* m_pdConnectionMatrix;
};

#endif
