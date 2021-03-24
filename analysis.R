source("convergence.R")

temporal.cor <- function(x, lags=5)
{
	n <- length(x)

	c <- rep(NA, lags)

	for (l in 1:lags)
		c[l] <- cor(x[-(1:l)], x[-c((n-l+1):n)])

	c
}

getCountryState <- function(batch, run, file.thin)
  {
    curr <- (run-1) %/% file.thin + 1
    i <- (run-1) %% file.thin + 1

    load(sprintf("output/batch%d_%d.Rdata", batch, curr))

    countryState <- as.integer(unlist(strsplit(
           readLines(sprintf("output/batch%d_%d_countries.data",
                                batch, run)), fixed=TRUE, split=NULL)))
    countryState <- matrix(countryState, nCountries[i])

    countryState.diff <- countryState[,-1] - countryState[,-dim(countryState)[2]]
    trans.dem <- apply(countryState.diff == 1, 2, sum)
    trans.aut <- apply(countryState.diff == -1, 2, sum)

    list(countryState=countryState, countryState.diff=countryState.diff, trans.dem=trans.dem, trans.aut=trans.aut)
  }

analyse <- function(batch, run, file.thin)
{
  prev <- 0
  nlags <- 10

  for (r in run)
    {
      cat("Analysing run", r, "\n")

      ## Only load files when needed (ie. when crossing the file thinning
      ## border)
      curr <- (r-1) %/% file.thin + 1
      if (curr != prev)
        {
          load(sprintf("output/batch%d_%d.Rdata", batch, curr))

          ## Only the first time, load parameter settings
          if (prev == 0)
            {
              par.data <- as.data.frame(params)
              par.data$avg.moran <- NA
              par.data$n.moran <- NA
              par.data$conv.moran <- NA
              par.data$conv.value.moran <- NA
              par.data$conv.n.moran <- NA
              par.data$conv.democ <- NA
              par.data$conv.value.democ <- NA
              par.data$conv.n.democ <- NA
              par.data$ncountries <- NA

              trans.stats <- NULL
            }

          prev <- curr
        }
      i <- (r-1) %% file.thin + 1

      ## Calculate clustering statistics
      par.data$avg.moran[r] <- mean(outMoran[,i], na.rm=TRUE)
      par.data$n.moran[r] <- sum(!is.na(outMoran[,i]))

      ## Calculate equilibrium statistics
      conv <- convergence(outMoran[,i], .05)
      par.data$conv.moran[r] <- conv$conv
      par.data$conv.value.moran[r] <- conv$value
      par.data$conv.n.moran[r] <- conv$n

      conv <- convergence(propDemoc[,i], .05)
      par.data$conv.democ[r] <- conv$conv
      par.data$conv.value.democ[r] <- conv$value
      par.data$conv.n.democ[r] <- conv$n

      ## Store number of countries
      par.data$ncountries[r] <- nCountries[i]

      ## Calculate transition statistics
      cs <- getCountryState(batch, curr, file.thin)
      tc.dem <- suppressWarnings(temporal.cor(cs$trans.dem, nlags))
      tc.aut <- suppressWarnings(temporal.cor(cs$trans.aut, nlags))
      trans.stats <- rbind(trans.stats, c(tc.dem, tc.aut))
    }

  colnames(trans.stats) <- rep("", 2*nlags)
  for (i in 1:nlags)
    {
      colnames(trans.stats)[i] <- sprintf("dem.lag.%d", i)
      colnames(trans.stats)[i + nlags] <- sprintf("aut.lag.%d", i)
    }

  cbind(par.data[run,], trans.stats)
}

calculate.autocor <- function(batch, run, file.thin)
{
  prev <- 0

  for (r in run)
    {
      cat("Analysing run", r, "\n")

      if (batch == 14) {
        save(r, file="last.Rdata")
        cat(r, file="last.txt")
      }

      ## Only load files when needed (ie. when crossing the file thinning
      ## border)
      curr <- (r-1) %/% file.thin + 1
      if (curr != prev)
        {
          load(sprintf("output/batch%d_%d.Rdata", batch, curr))

          ## Only the first time, load parameter settings
          if (prev == 0)
            {
              par.data <- as.data.frame(params)
              par.data$autocor.dem <- NA
              par.data$autocor.aut <- NA
            }

          prev <- curr
        }
      i <- (r-1) %% file.thin + 1

      ## Calculate autocorrelation statistics
      cs <- getCountryState(batch, curr, file.thin)
      dem <- tapply(cs$trans.dem, (1:length(cs$trans.dem) - 1) %/% 100, sum)
      aut <- tapply(cs$trans.aut, (1:length(cs$trans.aut) - 1) %/% 100, sum)
      par.data$autocor.dem[r] <- cor(dem[-1], dem[-length(dem)])
      par.data$autocor.aut[r] <- cor(aut[-1], aut[-length(aut)])
    }

  par.data[run,]
}

calculate.trans.moran <- function(batch, run, prefix="output/")
{
  lag <- function(x)
  {
    rbind(NA, x[-1,])
  }

  for (r in run)
    {
      fn <- sprintf("%sbatch%d_%d.Rdata", prefix, batch, r)
      cat("Loading", fn, "\n")
      load(fn)

      fn <- sprintf("%sbatch%d_%d_countries.data", prefix, batch, run)
      cat("Loading", fn, "\n")
	countryState <- as.integer(unlist(strsplit(readLines(fn), fixed=TRUE, split=NULL)))
      countryState <- matrix(countryState, ncol=nCountries)
      cat("Dimensions countryState:", dim(countryState), "\n")

      transCountryState <- rbind(NA, countryState[-1,] - countryState[-nrow(countryState),])
      blocks <- rep(1:100, each=1000)
      transToDem <- transCountryState == 1
      cat("Dimensions transToDem:", dim(transToDem), "\n")
      transToDem <- apply(transToDem, 2, tapply, blocks, sum)
      cat("Dimensions transToDem:", dim(transToDem), "\n")
      transToAut <- transCountryState == -1
      transToAut <- apply(transToAut, 2, tapply, blocks, sum)
      mode(transToAut) <- "numeric"

      cat("Preparing Wstd\n")
      Wstd <- matrix(Wstd, nrow=nCountries)
      mode(Wstd) <- "numeric"
      cat("Dimensions Wstd:", dim(Wstd), "\n")
    }
}


calculate.trans.moran2 <- function(batch, run, prefix="output/")
{
  lag <- function(x)
  {
    rbind(NA, x[-1,])
  }

  for (r in run)
    {
      fn <- sprintf("%sbatch%d_%d.Rdata", prefix, batch, r)
      cat("Loading", fn, "\n")
      load(fn)

      fn <- sprintf("%sbatch%d_%d_countries.data", prefix, batch, run)
      cat("Loading", fn, "\n")
	countryState <- as.integer(unlist(strsplit(readLines(fn), fixed=TRUE, split=NULL)))
      countryState <- matrix(countryState, ncol=nCountries)
      cat("Dimensions countryState:", dim(countryState), "\n")

      cat("Preparing Wstd\n")
      Wstd <- matrix(Wstd, nrow=nCountries)
      mode(Wstd) <- "numeric"
      cat("Dimensions Wstd:", dim(Wstd), "\n")

      cat("Calculating W %*% countryState\n")
      lWy <- as.vector(lag(countryState %*% Wstd))

      cat("Setting up y\n")
      y <- as.vector(countryState)
      y.inv <- 1 - y

      cat("Setting up lag(y)\n")
      ly <- as.vector(lag(countryState))
      ly.inv <- 1 - ly

      cat("Setting up population\n")
      population <- rep(countryDetails$population[countryDetails$iteration == 1], nrow(countryState))

      cat("Setting up time variable\n")
      t <- rep(1:nrow(countryState), each=nCountries)

      cat("Estimate model\n")
      m <- glm(y ~ 0 + ly + ly.inv + lWy:ly.inv + lWy:ly + population:ly.inv + population:ly,
	       family=binomial(link="logit"), subset=(t %% 1000 == 0))

      cat(table(y[t %% 1000 == 0], ly[t %% 1000 == 0]))
    }

  m
}
