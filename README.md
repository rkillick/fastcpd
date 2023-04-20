
<!-- README.md is generated from README.Rmd. Please edit that file -->

# fastcpd

<!-- badges: start -->

[![CRAN
status](https://www.r-pkg.org/badges/version/fastcpd)](https://cran.r-project.org/package=fastcpd)
[![R-CMD-check](https://github.com/doccstat/fastcpd/workflows/R-CMD-check/badge.svg)](https://github.com/doccstat/fastcpd/actions)
[![Codecov test
coverage](https://codecov.io/gh/doccstat/fastcpd/branch/main/graph/badge.svg)](https://app.codecov.io/gh/doccstat/fastcpd?branch=main)
[![Last
Commit](https://img.shields.io/github/last-commit/doccstat/fastcpd)](https://github.com/doccstat/fastcpd)
![r-universe](https://doccstat.r-universe.dev/badges/fastcpd)
<!-- badges: end -->

## Overview

The fastcpd (**FAST** **C**hange **P**oint **D**etection) is a fast
implmentation of change point detection methods in R. The **fastcpd**
package is designed to find change points in a fast manner. It is easy
to install and extensible to all kinds of change point problems with a
user specified cost function apart from the built-in cost functions.

If you’d like to learn how to use the fastcpd effectively, please refer
to the following references:

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

## Usage

`library(fastcpd)` will load the core fastcpd package in full or in
part:

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

### Documentation

<https://www.xingchi.li/fastcpd>

### Linear regression

``` r
library(fastcpd)
set.seed(1)
p <- 3
x <- mvtnorm::rmvnorm(300, rep(0, p), diag(p))
theta_0 <- rbind(c(1, 1.2, -1), c(-1, 0, 0.5), c(0.5, -0.3, 0.2))
y <- c(
  x[1:100, ] %*% theta_0[1, ] + rnorm(100, 0, 1),
  x[101:200, ] %*% theta_0[2, ] + rnorm(100, 0, 1),
  x[201:300, ] %*% theta_0[3, ] + rnorm(100, 0, 1)
)
result <- fastcpd(
  formula = y ~ . - 1,
  data = data.frame(y = y, x = x),
  family = "gaussian",
  cp_only = FALSE
)
plot(result)
summary(result)
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data.frame(y = y, x = x),
#>     family = "gaussian", cp_only = FALSE)
#>
#> Change points:
#> 98 202
```

### Logistic regression

``` r
library(fastcpd)
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

### Poisson regression

``` r
library(fastcpd)
set.seed(1)
p <- 3
x <- mvtnorm::rmvnorm(1500, rep(0, p), diag(p))
delta <- rnorm(p)
theta_0 <- c(1, 1.2, -1)
y <- c(
  rpois(300, exp(x[1:300, ] %*% theta_0)),
  rpois(400, exp(x[301:700, ] %*% (theta_0 + delta))),
  rpois(300, exp(x[701:1000, ] %*% theta_0)),
  rpois(100, exp(x[1001:1100, ] %*% (theta_0 - delta))),
  rpois(200, exp(x[1101:1300, ] %*% theta_0)),
  rpois(200, exp(x[1301:1500, ] %*% (theta_0 + delta)))
)
result <- fastcpd(
  formula = y ~ . - 1,
  data = data.frame(y = y, x = x),
  beta = (p + 1) * log(1500) / 2,
  k = function(x) 0,
  family = "poisson",
  epsilon = 1e-5,
  cp_only = FALSE
)
plot(result)
summary(result)
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data.frame(y = y, x = x),
#>     beta = (p + 1) * log(1500)/2, k = function(x) 0, family = "poisson",
#>     epsilon = 1e-05, cp_only = FALSE)
#>
#> Residuals:
#>       Min        1Q    Median        3Q       Max
#>   -1.0000   -1.0000   -0.5785    0.3564 1793.2299
#>
#> Change points:
#> 329 728 1021 1107 1325
```

### Penalized linear regression

``` r
library(fastcpd)
set.seed(1)
n <- 1500
p_true <- 6
p <- 50
x <- mvtnorm::rmvnorm(1500, rep(0, p), diag(p))
theta_0 <- rbind(
  runif(p_true, -5, -2),
  runif(p_true, -3, 3),
  runif(p_true, 2, 5),
  runif(p_true, -5, 5)
)
theta_0 <- cbind(theta_0, matrix(0, ncol = p - p_true, nrow = 4))
y <- c(
  x[1:300, ] %*% theta_0[1, ] + rnorm(300, 0, 1),
  x[301:700, ] %*% theta_0[2, ] + rnorm(400, 0, 1),
  x[701:1000, ] %*% theta_0[3, ] + rnorm(300, 0, 1),
  x[1001:1500, ] %*% theta_0[4, ] + rnorm(500, 0, 1)
)
result <- fastcpd(
  formula = y ~ . - 1,
  data = data.frame(y = y, x = x),
  family = "lasso",
  cp_only = FALSE
)
plot(result)
summary(result)
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data.frame(y = y, x = x),
#>     family = "lasso", cp_only = FALSE)
#>
#> Change points:
#> 300 700 1000
```

### Custom cost function: mean shift

``` r
library(fastcpd)
set.seed(1)
p <- 1
data <- rbind(
  mvtnorm::rmvnorm(300, mean = rep(0, p), sigma = diag(100, p)),
  mvtnorm::rmvnorm(400, mean = rep(50, p), sigma = diag(100, p)),
  mvtnorm::rmvnorm(300, mean = rep(2, p), sigma = diag(100, p))
)
segment_count_guess <- 10
block_size <- max(floor(sqrt(nrow(data)) / (segment_count_guess + 1)), 2)
block_count <- ceiling(nrow(data) / block_size)
data_all_vars <- rep(0, block_count)
for (block_index in seq_len(block_count)) {
  block_start <- (block_index - 1) * block_size + 1
  block_end <- min(block_index * block_size, nrow(data))
  data_all_vars[block_index] <- var(data[block_start:block_end, ])
}
data_all_var <- mean(data_all_vars)
mean_loss <- function(data) {
  n <- nrow(data)
  (norm(data, type = "F") ^ 2 - colSums(data) ^ 2 / n) / 2 / data_all_var +
    n / 2 * (log(data_all_var) + log(2 * pi))
}
mean_loss_result <- fastcpd(
  formula = ~ . - 1,
  data = data.frame(data),
  beta = (p + 1) * log(nrow(data)) / 2,
  p = p,
  cost = mean_loss
)
summary(mean_loss_result)
#> Call:
#> fastcpd(formula = ~. - 1, data = data.frame(data), beta = (p +
#>     1) * log(nrow(data))/2, p = p, cost = mean_loss)
#>
#> Change points:
#> 300 700
```

### Custom cost function: variance change

``` r
library(fastcpd)
set.seed(1)
p <- 1
data <- rbind.data.frame(
  mvtnorm::rmvnorm(300, mean = rep(0, p), sigma = diag(1, p)),
  mvtnorm::rmvnorm(400, mean = rep(0, p), sigma = diag(50, p)),
  mvtnorm::rmvnorm(300, mean = rep(0, p), sigma = diag(2, p))
)
data_all_mu <- colMeans(data)
var_loss <- function(data) {
  demeaned_data_norm <- norm(sweep(data, 2, data_all_mu), type = "F")
  nrow(data) * (1 + log(2 * pi) + log(demeaned_data_norm ^ 2 / nrow(data))) / 2
}
var_loss_result <- fastcpd(
  formula = ~ . - 1,
  data = data,
  beta = (p + 1) * log(nrow(data)) / 2,
  p = p,
  cost = var_loss
)
summary(var_loss_result)
#> Call:
#> fastcpd(formula = ~. - 1, data = data, beta = (p + 1) * log(nrow(data))/2,
#>     p = p, cost = var_loss)
#>
#> Change points:
#> 300 699
```

### Custom cost function: mean shift and variance change

``` r
library(fastcpd)
set.seed(1)
p <- 1
data <- rbind.data.frame(
  mvtnorm::rmvnorm(300, mean = rep(0, p), sigma = diag(1, p)),
  mvtnorm::rmvnorm(400, mean = rep(10, p), sigma = diag(1, p)),
  mvtnorm::rmvnorm(300, mean = rep(0, p), sigma = diag(50, p)),
  mvtnorm::rmvnorm(300, mean = rep(0, p), sigma = diag(1, p)),
  mvtnorm::rmvnorm(400, mean = rep(10, p), sigma = diag(1, p)),
  mvtnorm::rmvnorm(300, mean = rep(10, p), sigma = diag(50, p))
)
meanvar_loss <- function(data) {
  loss_part <- (colSums(data ^ 2) - colSums(data) ^ 2 / nrow(data)) / nrow(data)
  nrow(data) * (1 + log(2 * pi) + log(loss_part)) / 2
}
meanvar_loss_result <- fastcpd(
  formula = ~ . - 1,
  data = data,
  beta = (2 * p + 1) * log(nrow(data)) / 2,
  p = 2 * p,
  cost = meanvar_loss
)
summary(meanvar_loss_result)
#> Call:
#> fastcpd(formula = ~. - 1, data = data, beta = (2 * p + 1) * log(nrow(data))/2,
#>     p = 2 * p, cost = meanvar_loss)
#>
#> Change points:
#> 300 700 1000 1300 1700
```

### Custom cost function: Huber loss

``` r
library(fastcpd)
set.seed(1)
n <- 500 + 700 + 500
p <- 8
x <- mvtnorm::rmvnorm(n, mean = rep(0, p), sigma = diag(p))
theta <- rbind(
  mvtnorm::rmvnorm(1, mean = rep(0, p - 3), sigma = diag(p - 3)),
  mvtnorm::rmvnorm(1, mean = rep(5, p - 3), sigma = diag(p - 3)),
  mvtnorm::rmvnorm(1, mean = rep(9, p - 3), sigma = diag(p - 3))
)
theta <- cbind(theta, matrix(0, 3, 3))
theta <- theta[rep(seq_len(3), c(500, 700, 500)), ]
y_true <- rowSums(x * theta)
factor <- c(
  2 * stats::rbinom(500, size = 1, prob = 0.95) - 1,
  2 * stats::rbinom(700, size = 1, prob = 0.95) - 1,
  2 * stats::rbinom(500, size = 1, prob = 0.95) - 1
)
y <- factor * y_true + stats::rnorm(n)
data <- cbind.data.frame(y, x)
huber_threshold <- 1
huber_loss <- function(data, theta) {
  residual <- data[, 1] - data[, -1, drop = FALSE] %*% theta
  indicator <- abs(residual) <= huber_threshold
  sum(
    residual ^ 2 / 2 * indicator +
    huber_threshold * (abs(residual) - huber_threshold / 2) * (1 - indicator)
  )
}
huber_loss_gradient <- function(data, theta) {
  residual <- c(data[nrow(data), 1] - data[nrow(data), -1] %*% theta)
  if (abs(residual) <= huber_threshold) {
    - residual * data[nrow(data), -1]
  } else {
    - huber_threshold * sign(residual) * data[nrow(data), -1]
  }
}
huber_loss_hessian <- function(data, theta) {
  residual <- c(data[nrow(data), 1] - data[nrow(data), -1] %*% theta)
  if (abs(residual) <= huber_threshold) {
    outer(data[nrow(data), -1], data[nrow(data), -1])
  } else {
    0.01 * diag(length(theta))
  }
}
huber_regression_result <- fastcpd(
  formula = y ~ . - 1,
  data = data,
  beta = (p + 1) * log(n) / 2,
  cost = huber_loss,
  cost_gradient = huber_loss_gradient,
  cost_hessian = huber_loss_hessian
)
summary(huber_regression_result)
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data, beta = (p + 1) * log(n)/2,
#>     cost = huber_loss, cost_gradient = huber_loss_gradient, cost_hessian = huber_loss_hessian)
#>
#> Change points:
#> 575 1215 1395
```

## TODO

- examples in roxygen docs and multivariate custom cost functions,
  pkgdown references
