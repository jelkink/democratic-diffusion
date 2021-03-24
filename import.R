dyn.load("model.so")

factor.to.numeric <- function(x)
  {
    if (is.null(dim(x)))
      x <- as.numeric(levels(x))[x]
    else
      for (i in 1:dim(x)[2])
        x[,i] <- as.numeric(levels(x[,i]))[x[,i]]

    x
  }

simulation <- function(iter=8000, thin=10, nlevels=100, propDemoc=.01,
                       randRev=.0001, crossB=.5, regDelay=50, broadcast=NA,
                       comm.effect=1, width=20, height=20, multiplier=6,
                       useDecay=TRUE, decayStart=.05, decayStrength=-.15,
                       isoMean=50, isoStd=10, popMean=50, popStd=10,
                       attMean=10, attStd=5, inGMean=10, inGStd=4,
                       outGMean=90, outGStd=4, verbose=TRUE, detailThin=100,
                       batchID=0, runID=0) {

  if (is.na(broadcast))
    broadcast = floor(nlevels / 10)

  nCountryVariables <- 8
  
  outMoran <- as.double(rep(0, iter / thin))
  outProportion <- as.double(rep(0, iter / thin))
  outConnMatrix <- as.double(rep(0, (width*height)^2))
  outAttMap <- as.double(rep(0, (width*height)*iter/100))
  outPopulation <- as.integer(rep(0, width*height))
  outDetails <- as.integer(rep(0, nCountryVariables * width*height
                               * (iter / detailThin)))
  outTimeTaken <- as.integer(0)
  nCountries <- as.integer(0)
  
  out <- .C("runSimulation",
            as.integer(iter),
            as.integer(thin),
            as.integer(detailThin),
            as.integer(nlevels),
            as.double(propDemoc),
            as.double(randRev),
            as.double(crossB),
            as.integer(regDelay),
            as.integer(broadcast),
            as.integer(comm.effect),
            as.integer(width),
            as.integer(height),
            as.integer(multiplier),
            as.integer(useDecay),
            as.double(decayStart),
            as.double(decayStrength),
            as.integer(isoMean),
            as.integer(isoStd),
            as.integer(popMean),
            as.integer(popStd),
            as.integer(attMean),
            as.integer(attStd),
            as.integer(inGMean),
            as.integer(inGStd),
            as.integer(outGMean),
            as.integer(outGStd),
            as.integer(batchID),
            as.integer(runID),
            outMoran = outMoran,
            outProportion = outProportion,
            outConnMatrix = outConnMatrix,
            outAttMap = outAttMap,
            outPopulation = outPopulation,
            outDetails = outDetails,
            outTimeTaken = outTimeTaken,
            nCountries = nCountries,
            as.integer(verbose))

  cat("\nReached end of simulation - returning data\n");

  nData <- iter / detailThin
  country <- rep(1:out$nCountries, nCountryVariables * nData)
  iteration <- rep(1:nData, each = nCountryVariables * out$nCountries)
  variable <- rep(rep(c("democracy","protest","avg.att","ncoups","nrevs",
                        "reg.age","last.change","init.reg"),
                      each = out$nCountries), nData)
  details <- data.frame(cbind(country, iteration, variable,
                   out$outDetails[1:length(country)]))
  colnames(details) <- c("country","iteration","variable","value")
  details <- reshape(details, direction="wide", idvar=c("country","iteration"),
                     timevar="variable")
  colnames(details) <- c("country","iteration","democracy","protest","avg.att",
                         "ncoups","nrevs","reg.age","last.change","init.reg")
  details[,-1] <- factor.to.numeric(details[,-1])
  details$population <- rep(out$outPopulation[1:out$nCountries], nData)
  
  connMatrix <- t(matrix(out$outConnMatrix[1:(out$nCountries^2)],
                         nrow=out$nCountries, ncol=out$nCountries))
  
  list(moran=out$outMoran, propDemoc=out$outProportion, attMap=out$outAttMap,
       nCountries=out$nCountries, countryDetails=details, Wstd=connMatrix,
       iter=1:iter, time.taken=out$outTimeTaken, out=out)
}

