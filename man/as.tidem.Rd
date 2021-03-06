% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/tides.R
\name{as.tidem}
\alias{as.tidem}
\title{Create tidem object from fitted harmonic data}
\usage{
as.tidem(tRef, latitude, name, amplitude, phase, debug = getOption("oceDebug"))
}
\arguments{
\item{tRef}{a POSIXt value indicating the mean time of the
observations used to develop the harmonic model. This is rounded
to the nearest hour in \code{\link[=as.tidem]{as.tidem()}}, to match \code{\link[=tidem]{tidem()}}.}

\item{latitude}{Numerical value indicating the latitude of the
observations that were used to create the harmonic model. This
is needed for nodal-correction procedures carried out
by \code{\link[=tidemVuf]{tidemVuf()}}.}

\item{name}{character vector holding names of constituents.}

\item{amplitude}{Numeric vector of constituent amplitudes.}

\item{phase}{Numeric vector of constituent Greenwich phases.}

\item{debug}{an integer specifying whether debugging information is
to be printed during the processing. This is a general parameter that
is used by many \code{oce} functions. Generally, setting \code{debug=0}
turns off the printing, while higher values suggest that more information
be printed. If one function calls another, it usually reduces the value of
\code{debug} first, so that a user can often obtain deeper debugging
by specifying higher \code{debug} values.}
}
\value{
An object of \linkS4class{tidem}, with only minimal
contents.
}
\description{
This function is intended to provide a bridge to
\code{\link[=predict.tidem]{predict.tidem()}}, enabling tidal predictions based
on published tables of harmonic fits.
}
\details{
Note that only constituent names known to \code{\link[=tidem]{tidem()}} are handled.
The permitted names are those listed in Foreman (1978), and
tabulated with
\preformatted{
data(tidedata)
data.frame(name=tidedata$const$name, freq=tidedata$const$freq)
}
Warnings are issued for any constituent name that is not in this list; as
of the late summer of 2019, the only example seen in practice is
\code{M1}, which according to Wikipedia (2019) has frequency 0.0402557, which
is very close to that of \code{NO1}, i.e. 0.04026859, perhaps explaining
why Foreman (1978) did not handle this constituent. A warning is
issued if this or any other unhandled constituent is provided
in the \code{name} argument to \code{as.tidem()}.
}
\section{Known issues}{

There are two known differences between \code{\link[=tidem]{tidem()}} and the Matlab
\code{T_TIDE} package, as listed in references 3 and 4. Work on these
issues is planned for the summer of 2020.
}

\examples{
# Simulate a tide table with output from tidem().
data(sealevelTuktoyaktuk)
# 'm0' is model fitted by tidem()
m0 <- tidem(sealevelTuktoyaktuk)
p0 <- predict(m0, sealevelTuktoyaktuk[["time"]])
m1 <- as.tidem(mean(sealevelTuktoyaktuk[["time"]]), sealevelTuktoyaktuk[["latitude"]],
               m0[["name"]], m0[["amplitude"]], m0[["phase"]])
# Test agreement with tidem() result, by comparing predicted sealevels.
p1 <- predict(m1, sealevelTuktoyaktuk[["time"]])
expect_lt(max(abs(p1 - p0), na.rm=TRUE), 1e-10)
# Simplified harmonic model, using large constituents
# > m0[["name"]][which(m[["amplitude"]]>0.05)]
# [1] "Z0"  "MM"  "MSF" "O1"  "K1"  "OO1" "N2"  "M2"  "S2"
h <- "
name  amplitude      phase
  Z0 1.98061875   0.000000
  MM 0.21213065 263.344739
 MSF 0.15605629 133.795004
  O1 0.07641438  74.233130
  K1 0.13473817  81.093134
 OO1 0.05309911 235.749693
  N2 0.08377108  44.521462
  M2 0.49041340  77.703594
  S2 0.22023705 137.475767"
coef <- read.table(text=h, header=TRUE)
m2 <- as.tidem(mean(sealevelTuktoyaktuk[["time"]]),
               sealevelTuktoyaktuk[["latitude"]],
               coef$name, coef$amplitude, coef$phase)
p2 <- predict(m2, sealevelTuktoyaktuk[["time"]])
expect_lt(max(abs(p2 - p0), na.rm=TRUE), 1)
par(mfrow=c(3, 1))
oce.plot.ts(sealevelTuktoyaktuk[["time"]], p0)
ylim <- par("usr")[3:4] # to match scales in other panels
oce.plot.ts(sealevelTuktoyaktuk[["time"]], p1, ylim=ylim)
oce.plot.ts(sealevelTuktoyaktuk[["time"]], p2, ylim=ylim)

}
\references{
\enumerate{
\item Foreman, M. G. G., 1978. Manual for Tidal Currents Analysis and Prediction.
Pacific Marine Science Report. British Columbia, Canada: Institute of Ocean
Sciences, Patricia Bay.
\item Wikipedia, "Theory of Tides." https://en.wikipedia.org/wiki/Theory_of_tides
Downloaded Aug 17, 2019.
\item Github issue 1653: tidem() and t_tide do not produce identical results
https://github.com/dankelley/oce/issues/1653
\item Github issue 1654: predict(tidem()) uses all constituents, unlike T_TIDE
https://github.com/dankelley/oce/issues/1654
}
}
\seealso{
Other things related to tides: 
\code{\link{[[,tidem-method}},
\code{\link{[[<-,tidem-method}},
\code{\link{plot,tidem-method}},
\code{\link{predict.tidem}()},
\code{\link{summary,tidem-method}},
\code{\link{tidalCurrent}},
\code{\link{tidedata}},
\code{\link{tidem-class}},
\code{\link{tidemAstron}()},
\code{\link{tidemVuf}()},
\code{\link{tidem}},
\code{\link{webtide}()}
}
\concept{things related to tides}
