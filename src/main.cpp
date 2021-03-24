
#include <iostream>
#include <exception>

extern "C" {
#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
}

#include "model.h"
#include "exception.h"

using std::cout;
using std::endl;
using std::exception;

extern "C" {

  void runSimulation(int *p_nIterations, 
		     int *p_nThinning, 
		     int *p_nDetailThinning,
		     int *p_nNumberOfLevels, 
		     double *p_dInitialProportionDemocratic, 
		     double *p_dRandomRevolutionChance, 
		     double *p_dCrossBorderChance, 
		     int *p_nRegimeDelay, 
		     int *p_nBroadcastEffect,
		     int *p_nCommunicationEffect,
		     int *p_nFieldWidth, 
		     int *p_nFieldHeight, 
		     int *p_nBorderMultiplier, 
		     int *p_bUseDecay, 
		     double *p_dDecayStart, 
		     double *p_dDecayStrength,
		     int *p_nIsoMean,
		     int *p_nIsoStd, 
		     int *p_nPopMean,
		     int *p_nPopStd, 
		     int *p_nAttMean, 
		     int *p_nAttStd, 
		     int *p_nInGroupMean, 
		     int *p_nInGroupStd,
		     int *p_nOutGroupMean,
		     int *p_nOutGroupStd, 
		     int *p_nBatchID,
		     int *p_nRunID,
		     double *out_Moran, 
		     double *out_Proportion, 
		     double *out_ConnMatrix,
		     double *out_AttMap,
		     int *out_Population,
		     int *out_Details,
		     int *out_TimeTaken,
		     int *p_nCountries,
		     int *p_nVerbose) 
  {
    time_t tt = time(NULL); 
  
    try
      {
	CModel Model(*p_nIterations, 
		     *p_nThinning, *p_nDetailThinning, 
		     *p_nNumberOfLevels, 
		     *p_dInitialProportionDemocratic,
		     *p_dRandomRevolutionChance,
		     *p_dCrossBorderChance,
		     *p_nRegimeDelay,
		     *p_nBroadcastEffect, *p_nCommunicationEffect,
		     *p_nFieldWidth, *p_nFieldHeight,
		     *p_nBorderMultiplier,
		     *p_bUseDecay,
		     *p_dDecayStart, *p_dDecayStrength,
		     *p_nIsoMean, *p_nIsoStd,
		     *p_nPopMean, *p_nPopStd,
		     *p_nAttMean, *p_nAttStd,
		     *p_nInGroupMean, *p_nInGroupStd,
		     *p_nOutGroupMean, *p_nOutGroupStd,
		     *p_nBatchID, *p_nRunID,
		     *p_nVerbose);
      
	Model.Run(out_Moran, out_Proportion, out_ConnMatrix,
		  out_AttMap, out_Population, out_Details,
		  p_nCountries);
      }
    catch (CException& e)
      {
	cout << "Uncaught exception: " << e.GetMessage() << endl;
      }
    catch (exception& e)
      {
	Rprintf("Uncaught exception: %s\n", e.what());
      }

    *out_TimeTaken = (int) (tt - time(NULL));
  }

  static R_NativePrimitiveArgType runSimulation_t[] 
  = {INTSXP, INTSXP, INTSXP, INTSXP, REALSXP, REALSXP, REALSXP, 
     INTSXP, INTSXP, INTSXP, INTSXP, INTSXP, INTSXP, INTSXP, REALSXP,
     REALSXP, INTSXP, INTSXP, INTSXP, INTSXP, INTSXP, INTSXP, INTSXP,
     INTSXP, INTSXP, INTSXP, INTSXP, INTSXP, REALSXP, REALSXP, REALSXP,
     REALSXP, INTSXP, INTSXP, INTSXP, INTSXP, INTSXP};

  // Stuff for dynamic loading in R
  R_CMethodDef cMethods[] = {
    {"runSimulation", (DL_FUNC) &runSimulation, 37, runSimulation_t},
    {NULL, NULL, 0}
  };

  void R_init_mylib(DllInfo *info)
  {
    R_registerRoutines(info, cMethods, NULL, NULL, NULL);
  }

  void R_unload_mylib(DllInfo *info)
  {
  }

} // extern "C"
