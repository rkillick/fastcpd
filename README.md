
<!-- README.md is generated from README.Rmd. Please edit that file -->



# fastcpd

<!-- badges: start -->
[![CRAN status](https://www.r-pkg.org/badges/version/fastcpd)](https://cran.r-project.org/package=fastcpd) [![R-CMD-check](https://github.com/doccstat/fastcpd/workflows/R-CMD-check/badge.svg)](https://github.com/doccstat/fastcpd/actions) [![Codecov test coverage](https://codecov.io/gh/doccstat/fastcpd/branch/main/graph/badge.svg)](https://app.codecov.io/gh/doccstat/fastcpd?branch=main) [![Last Commit](https://img.shields.io/github/last-commit/doccstat/fastcpd)](https://github.com/doccstat/fastcpd) <!-- badges: end -->

## Overview

The fastcpd (**FAST** **C**hange **P**oint **D**etection) is a fast implmentation of change point detection methods in R. The **fastcpd** package is designed to find change points in a fast manner. It is easy to install and extensible to all kinds of change point problems with a user specified cost function apart from the built-in cost functions.

If you'd like to learn how to use the fastcpd effectively, please refer to the following references:

-   [Sequential Gradient Descent and Quasi-Newton's Method for Change-Point Analysis](https://proceedings.mlr.press/v206/zhang23b.html)

## Installation

::: .pkgdown-release

``` {.r}
# Install from CRAN, (not yet available)
install.packages("fastcpd")
```

:::

::: .pkgdown-devel

``` {.r}
# Install the development version from GitHub
# install.packages("pak")
pak::pak("doccstat/fastcpd")
```

:::

If you're compiling from source, you can run `pak::pkg_system_requirements("fastcpd")`, to see the complete set of system packages needed on your machine.

## Usage

`library(fastcpd)` will load the core fastcpd package in full or in part:

-   [Rcpp](https://github.com/RcppCore/Rcpp), for C++ source code compilation.
-   [RcppArmadillo](https://github.com/RcppCore/RcppArmadillo), for fast linear algebra.
-   [fastglm](https://github.com/jaredhuling/fastglm), for fast generalized linear models.
-   [DescTools](https://github.com/AndriSignorell/DescTools), for Winsorizing Poisson data.
-   [glmnet](https://glmnet.stanford.edu/), for penalized regression.
-   [ggplot2](https://github.com/tidyverse/ggplot2), for data visualization.

### Binomial

``` {.r}
library(fastcpd)
# Binomial
set.seed(1)
x <- matrix(rnorm(1500, 0, 1), ncol = 5)
theta <- rbind(rnorm(5, 0, 1), rnorm(5, 2, 1))
y <- c(
  rbinom(125, 1, 1 / (1 + exp(-x[1:125, ] %*% theta[1, ]))),
  rbinom(175, 1, 1 / (1 + exp(-x[126:300, ] %*% theta[2, ])))
)
result <- fastcpd(
  formula = y ~ . - 1,
  data = data.frame(y = y, x = x),
  family = "binomial",
  cp_only = FALSE
)
plot(result)
summary(result)
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data.frame(y = y, x = x),
#>     family = "binomial", cp_only = FALSE)
#>
#> Residuals:
#>       Min        1Q    Median        3Q       Max
#> -14.09576  -1.07218  -1.00000   1.07353  35.39472
#>
#> Change points:
#> 126
```
