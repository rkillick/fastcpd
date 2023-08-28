
<!-- README.md is generated from README.Rmd. Please edit that file -->

# fastcpd

[![CRAN
status](https://www.r-pkg.org/badges/version/fastcpd)](https://cran.r-project.org/package=fastcpd)
[![R-CMD-check](https://github.com/doccstat/fastcpd/workflows/R-CMD-check/badge.svg)](https://github.com/doccstat/fastcpd/actions)
[![Codecov test
coverage](https://codecov.io/gh/doccstat/fastcpd/branch/main/graph/badge.svg)](https://app.codecov.io/gh/doccstat/fastcpd?branch=main)
[![Last
Commit](https://img.shields.io/github/last-commit/doccstat/fastcpd)](https://github.com/doccstat/fastcpd)
[![r-universe](https://doccstat.r-universe.dev/badges/fastcpd)](https://doccstat.r-universe.dev)

## Overview

The fastcpd (**fast** **c**hange **p**oint **d**etection) is a fast
implmentation of change point detection methods in R. The **fastcpd**
package is designed to find change points in a fast manner. It is easy
to install and extensible to all kinds of change point problems with a
user specified cost function apart from the built-in cost functions.

If you’d like to learn how to use the fastcpd effectively, please refer
to:

- [Sequential Gradient Descent and Quasi-Newton’s Method for
  Change-Point
  Analysis](https://proceedings.mlr.press/v206/zhang23b.html)

## Installation

``` r
# Install from CRAN, (not yet available)
install.packages("fastcpd")
```

``` r
# Install the development version from GitHub
# install.packages("pak")
pak::pak("doccstat/fastcpd")
```

If you’re compiling from source, you can run
`pak::pkg_system_requirements("fastcpd")`, to see the complete set of
system packages needed on your machine.

[documentation &
examples](https://fastcpd.xingchi.li/reference/fastcpd.html#ref-examples)

## Dependency

`library(fastcpd)` will load the following packages:

- [Rcpp](https://github.com/RcppCore/Rcpp), for C++ source code
  compilation.
- [RcppArmadillo](https://github.com/RcppCore/RcppArmadillo), for fast
  linear algebra.
- [fastglm](https://github.com/jaredhuling/fastglm), for fast
  generalized linear models.
- [DescTools](https://github.com/AndriSignorell/DescTools), for
  Winsorizing Poisson data.
- [glmnet](https://glmnet.stanford.edu/), for penalized regression.
- [ggplot2](https://github.com/tidyverse/ggplot2), for data
  visualization.

## Contact us

Encountered a bug or unintended behavior?

1.  File a ticket at [GitHub
    Issues](https://github.com/doccstat/fastcpd/issues).
2.  Contact the developers specified in [DESCRIPTION](/DESCRIPTION).
