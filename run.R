source("import.R")

doubleEqual <- function(x,y,margin=.000001)
	(x - margin) < y & (x + margin) > y

nIterations <- 10000
thinning <- 50
detailThinning <- 10
fileThinning <- 1
nPerSetting <- 1
seriesID <- 0

outMoran <- NULL
propDemoc <- NULL
nCountries <- NULL
attMap <- NULL
Wstd <- list()
countryDetails <- NULL

add.params <- function(params, add)
  cbind(params %x% matrix(1, length(add), 1), add)

params <- c(20) # in-group mean
params <- add.params(params, c(60)) # out-group/in-group difference
params <- add.params(params, c(5)) # c(0,1,10))  # broadcast effect
params <- add.params(params, c(.5)) # seq(from=.1, to=.9, by=.4)) # crossB
params <- add.params(params, c(1)) # communication effect
params <- add.params(params, 10) # initial mean attitude
params <- add.params(params, 4) # initial mean standard deviation
params <- add.params(params, .1) # initial proportion democratic
params <- add.params(params, 1:nPerSetting) # run

params

colnames(params) <- c("threshold", "out.add", "broadcast", "crossB", "comm.eff", "att", "att.std", "init.dem", "run")

init.time <- proc.time()[3]

for (i in 1:dim(params)[1]) {

  cat(sprintf("\nSTARTING RUN %d OF %d (approximately %d minutes left):\n\n", i, dim(params)[1], floor((proc.time()[3] - init.time) / i * (dim(params)[1] - i) / 60)))

  batch <- floor(i / fileThinning)

  res <- simulation(iter=nIterations, broadcast=params[i,"broadcast"], crossB=params[i,"crossB"],  comm.effect=params[i,"comm.eff"], attMean=params[i,"att"], attStd=params[i,"att.std"], inGMean=params[i,"threshold"], inGStd=0, outGMean=params[i,"threshold"]+params[i,"out.add"], outGStd=0, propDemoc=params[i,"init.dem"], detailThin=detailThinning, thin=thinning)
  
  cat("Reached end of simulation - storing data\n")

  outMoran <- cbind(outMoran, res$moran)
  propDemoc <- cbind(propDemoc, res$propDemoc)
  nCountries <- c(nCountries, res$nCountries)
  Wstd <- c(Wstd, res$Wstd)
  attMap <- cbind(attMap, res$attMap) # only works because width, height, and iter are constant
  res$countryDetails$run <- i
  countryDetails <- rbind(countryDetails, res$countryDetails)

  if (i %% fileThinning == 0) {

    save(outMoran, propDemoc, nCountries, attMap, params, countryDetails, file=sprintf("output/batch%d_%d.Rdata", seriesID, batch))

    outMoran <- NULL
    propDemoc <- NULL
    nCountries <- NULL
    Wstd <- list()
    attMap <- NULL
    countryDetails <- NULL
  }
}

save(outMoran, propDemoc, nCountries, attMap, params, countryDetails, file=sprintf("output/batch%d_%d.Rdata", seriesID, batch+1))
