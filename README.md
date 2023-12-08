
<!-- README.md is generated from README.Rmd. Please edit that file -->

# fastcpd <a href="https://fastcpd.xingchi.li"><img src="man/figures/logo.svg" align="right" height="138" /></a>

[![CRAN
status](https://www.r-pkg.org/badges/version-last-release/fastcpd)](https://cran.r-project.org/package=fastcpd)
[![Downloads](https://cranlogs.r-pkg.org/badges/grand-total/fastcpd)](https://cran.r-project.org/package=fastcpd)
[![R-CMD-check](https://github.com/doccstat/fastcpd/workflows/R-CMD-check/badge.svg)](https://github.com/doccstat/fastcpd/actions)
[![Codecov test
coverage](https://codecov.io/gh/doccstat/fastcpd/branch/main/graph/badge.svg)](https://app.codecov.io/gh/doccstat/fastcpd?branch=main)
[![Last
Commit](https://img.shields.io/github/last-commit/doccstat/fastcpd)](https://github.com/doccstat/fastcpd)
[![r-universe](https://doccstat.r-universe.dev/badges/fastcpd)](https://doccstat.r-universe.dev)
[![CodeFactor](https://www.codefactor.io/repository/github/doccstat/fastcpd/badge)](https://www.codefactor.io/repository/github/doccstat/fastcpd)

## Overview

The fastcpd (**fast** **c**hange **p**oint **d**etection) is a fast
implmentation of change point detection methods in R. The **fastcpd**
package is designed to find change points in a fast manner. It is easy
to install and extensible to all kinds of change point problems with a
user specified cost function apart from the built-in cost functions.

To learn more behind the algorithms:

- [Sequential Gradient Descent and Quasi-Newton’s Method for
  Change-Point
  Analysis](https://proceedings.mlr.press/v206/zhang23b.html)

## Installation

``` r
# Install from CRAN
install.packages("fastcpd")
```

<details close>
<summary>
Development version
</summary>

``` r
# Development version from r-universe with CRAN version as a fallback
install.packages(
  "fastcpd",
  repos = c("https://doccstat.r-universe.dev", "https://cloud.r-project.org")
)

## install.packages("pak")
pak::pak("doccstat/fastcpd")

## install.packages("devtools")
devtools::install_github("doccstat/fastcpd")
```

</details>
<details close>
<summary>
With mamba or conda (available soon)
</summary>

``` bash
# conda-forge is a fork from CRAN and may not be up-to-date

# Use mamba
mamba install r-fastcpd
# Use conda
conda install -c conda-forge r-fastcpd
```

</details>

### FAQ

<details close>
<summary>
Package dependencies
</summary>

`fastcpd` depends on the following packages:

- [Rcpp](https://github.com/RcppCore/Rcpp), for C++ source code
  compilation.
- [RcppArmadillo](https://github.com/RcppCore/RcppArmadillo), for fast
  linear algebra.
- [fastglm](https://github.com/jaredhuling/fastglm), for fast
  generalized linear models.
- [glmnet](https://glmnet.stanford.edu/), for penalized regression.
- [ggplot2](https://github.com/tidyverse/ggplot2), for data
  visualization.

If you’re compiling from source, you can run the following command to
see the complete set of system packages needed on your machine.

``` r
pak::pkg_sysreqs("doccstat/fastcpd")
#> ── Install scripts ───────────────────────────────────────────── Ubuntu 20.04
#> apt-get -y update
#> apt-get -y install libcurl4-openssl-dev libssl-dev zlib1g-dev make
#>
#> ── Packages and their system dependencies ───────────────────────────────────
#> curl       – libcurl4-openssl-dev, libssl-dev
#> data.table – zlib1g-dev
#> fs         – make
#> openssl    – libssl-dev
```

</details>
<details close>
<summary>
I countered problems related to gfortran on Mac OSX or Linux!
</summary>

The package should be able to install on Mac and any Linux distribution
without any problems if all the dependencies are installed. However, if
you encountered problems related to gfortran, it might be because
`RcppArmadillo` is not installed previously. Try [Mac OSX stackoverflow
solution](https://stackoverflow.com/a/72997915) or [Linux stackover
solution](https://stackoverflow.com/a/15540919) if you have trouble
installing `RcppArmadillo`.

</details>

## Cheatsheet

[![fastcpd
cheatsheet](man/figures/cheatsheets.png)](https://github.com/doccstat/fastcpd/blob/main/man/figures/cheatsheets.pdf)

## Usage

It is hard to demonstrate all the features of `fastcpd` in a single
example due to the flexibility of the package. For more examples, please
refer to the [function
reference](https://fastcpd.xingchi.li/reference/index.html).

``` r
set.seed(1)
n <- 1000
x <- rep(0, n + 3)
for (i in 1:600) {
  x[i + 3] <- 0.6 * x[i + 2] - 0.2 * x[i + 1] + 0.1 * x[i] + rnorm(1, 0, 3)
}
for (i in 601:1000) {
  x[i + 1] <- 0.3 * x[i + 2] + 0.4 * x[i + 1] + 0.2 * x[i] + rnorm(1, 0, 3)
}
result <- fastcpd::fastcpd.ar(x[3 + seq_len(n)], 3)
summary(result)
#> 
#> Call:
#> fastcpd::fastcpd.ar(data = x[3 + seq_len(n)], order = 3)
#> 
#> Change points:
#> 612 
#> 
#> Cost values:
#> 2748.404 2022.597 
#> 
#> Parameters:
#>     segment 1   segment 2
#> 1  0.57656238  0.13006290
#> 2 -0.21582749 -0.03084403
#> 3  0.07985424 -0.04544551
plot(result)
```

![](man/figures/README-ar3-1.png)<!-- -->

## Examples

<!-- This example section is a direct copy from `fastcpd` documentation -->
<details close>
<summary>
Click to open
</summary>

### linear regression

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
  family = "lm"
)
plot(result)
```

![](man/figures/README-linear_regression-1.png)<!-- -->

``` r
summary(result)
#> 
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data.frame(y = y, x = x), 
#>     family = "lm")
#> 
#> Change points:
#> 98 202 
#> 
#> Cost values:
#> 53.44023 53.1441 45.04974 
#> 
#> Parameters:
#>    segment 1   segment 2  segment 3
#> 1  0.9704022 -1.07884004  0.5925092
#> 2  1.1786074 -0.01757927 -0.5287126
#> 3 -0.9258587  0.63906143  0.1929411
```

### linear regression with one-dimensional covariate

``` r
library(fastcpd)
set.seed(1)
p <- 1
x <- mvtnorm::rmvnorm(300, rep(0, p), diag(p))
theta_0 <- matrix(c(1, -1, 0.5))
y <- c(
  x[1:100, ] * theta_0[1, ] + rnorm(100, 0, 1),
  x[101:200, ] * theta_0[2, ] + rnorm(100, 0, 1),
  x[201:300, ] * theta_0[3, ] + rnorm(100, 0, 1)
)
result <- fastcpd(
  formula = y ~ . - 1,
  data = data.frame(y = y, x = x),
  family = "lm"
)
plot(result)
```

![](man/figures/README-linear_regression_with_one_dimensional_covariate-1.png)<!-- -->

``` r
summary(result)
#> 
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data.frame(y = y, x = x), 
#>     family = "lm")
#> 
#> Change points:
#> 99 197 
#> 
#> Cost values:
#> 48.36336 65.71493 46.37389 
#> 
#> Parameters:
#>   segment 1  segment 2 segment 3
#> 1  0.956995 -0.8488617 0.4562731
```

### linear regression with noise variance not equal to 1

``` r
library(fastcpd)
set.seed(1)
p <- 4
n <- 300
cp <- c(100, 200)
x <- mvtnorm::rmvnorm(n, rep(0, p), diag(p))
theta_0 <- rbind(c(1, 3.2, -1, 0), c(-1, -0.5, 2.5, -2), c(0.8, -0.3, 1, 1))
y <- c(
  x[1:cp[1], ] %*% theta_0[1, ] + rnorm(cp[1], 0, sd = 3),
  x[(cp[1] + 1):cp[2], ] %*% theta_0[2, ] + rnorm(cp[2] - cp[1], 0, sd = 3),
  x[(cp[2] + 1):n, ] %*% theta_0[3, ] + rnorm(n - cp[2], 0, sd = 3)
)

result <- fastcpd(
  data = data.frame(y = y, x = x),
  family = "lm"
)
summary(result)
#> 
#> Call:
#> fastcpd(data = data.frame(y = y, x = x), family = "lm")
#> 
#> Change points:
#> 97 214 
#> 
#> Cost values:
#> 519.8276 488.0173 408.1008 
#> 
#> Parameters:
#>     segment 1  segment 2  segment 3
#> 1  0.74291290 -0.4872500 0.80982091
#> 2  3.69465275 -0.6402096 0.09460024
#> 3 -1.24746871  2.1728986 0.87547701
#> 4  0.09579985 -1.6546439 1.52837626
```

### logistic regression

``` r
library(fastcpd)
set.seed(1)
x <- matrix(rnorm(1500, 0, 1), ncol = 5)
theta <- rbind(rnorm(5, 0, 1), rnorm(5, 2, 1))
y <- c(
  rbinom(125, 1, 1 / (1 + exp(-x[1:125, ] %*% theta[1, ]))),
  rbinom(175, 1, 1 / (1 + exp(-x[126:300, ] %*% theta[2, ])))
)
result <- suppressWarnings(fastcpd(
  formula = y ~ . - 1,
  data = data.frame(y = y, x = x),
  family = "binomial"
))
summary(result)
#> 
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data.frame(y = y, x = x), 
#>     family = "binomial")
#> 
#> Change points:
#> 126 
#> 
#> Cost values:
#> 56.90525 30.76875 
#> 
#> Parameters:
#>    segment 1 segment 2
#> 1  0.7259293  1.878525
#> 2 -1.0294802  2.704376
#> 3  1.0576503  3.702310
#> 4 -0.8812767  2.258796
#> 5  0.2419351  2.524173
```

### poisson regression

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
  epsilon = 1e-5
)
summary(result)
#> 
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data.frame(y = y, x = x), 
#>     beta = (p + 1) * log(1500)/2, k = function(x) 0, family = "poisson", 
#>     epsilon = 1e-05)
#> 
#> Change points:
#> 329 728 1021 1107 1325 
#> 
#> Cost values:
#> 14425.87 13971.23 697.2187 107.5353 380.7153 51.93594 
#> 
#> Parameters:
#>     segment 1  segment 2  segment 3  segment 4 segment 5  segment 6
#> 1  2.60927673  1.9255183  0.7405125 -0.3965022  1.117753  2.5479308
#> 2  0.02398457  0.1068924  1.4721444  1.8677797  1.019035  0.4947115
#> 3 -1.34361104 -2.7353603 -0.8906937  0.4651667 -1.178933 -2.5038966
result_two_epochs <- fastcpd(
  formula = y ~ . - 1,
  data = data.frame(y = y, x = x),
  beta = (p + 1) * log(1500) / 2,
  k = function(x) 1,
  family = "poisson",
  epsilon = 1e-4
)
summary(result_two_epochs)
#> 
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data.frame(y = y, x = x), 
#>     beta = (p + 1) * log(1500)/2, k = function(x) 1, family = "poisson", 
#>     epsilon = 1e-04)
#> 
#> Change points:
#> 328 716 1020 1102 1323 
#> 
#> Cost values:
#> 14417.14 2976.961 717.4614 31.48528 296.6285 53.94423 
#> 
#> Parameters:
#>     segment 1  segment 2  segment 3  segment 4 segment 5  segment 6
#> 1  2.60955822  2.4484869  0.7832980 -0.5008107  1.105317  2.5479958
#> 2  0.02371536  0.4084502  1.4456715  1.9282798  1.057743  0.4951862
#> 3 -1.34277129 -2.5426556 -0.8989812  0.5197285 -1.128259 -2.5035143
```

### penalized linear regression

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
  family = "lasso"
)
plot(result)
```

![](man/figures/README-penalized_linear_regression-1.png)<!-- -->

``` r
summary(result)
#> 
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data.frame(y = y, x = x), 
#>     family = "lasso")
#> 
#> Change points:
#> 300 700 1000 
#> 
#> Cost values:
#> 209.3166 262.0191 189.6773 308.4771 
#> 
#> Parameters:
#> 50 x 4 sparse Matrix of class "dgCMatrix"
#>       segment 1  segment 2 segment 3  segment 4
#>  [1,] -2.876798  0.3208273  4.030105  .        
#>  [2,] -2.813362 -0.3583706  3.895664  3.0989168
#>  [3,] -2.801067 -0.1878700  2.557583  2.7598671
#>  [4,] -1.910738  0.4607120  3.309766 -0.5228579
#>  [5,] -3.052728 -0.4509677  2.079685 -3.3649996
#>  [6,] -1.823872  0.4025762  4.783195  .        
#>  [7,]  .         .          .         .        
#>  [8,]  .         .          .         .        
#>  [9,]  .         .          .         .        
#> [10,]  .         .          .         .        
#> [11,]  .         .          .         .        
#> [12,]  .         .          .         .        
#> [13,]  .         .          .         .        
#> [14,]  .         .          .         .        
#> [15,]  .         .          .         .        
#> [16,]  .         .          .         .        
#> [17,]  .         .          .         .        
#> [18,]  .         .          .         .        
#> [19,]  .         .          .         .        
#> [20,]  .         .          .         .        
#> [21,]  .         .          .         .        
#> [22,]  .         .          .         .        
#> [23,]  .         .          .         .        
#> [24,]  .         .          .         .        
#> [25,]  .         .          .         .        
#> [26,]  .         .          .         .        
#> [27,]  .         .          .         .        
#> [28,]  .         .          .         .        
#> [29,]  .         .          .         .        
#> [30,]  .         .          .         .        
#> [31,]  .         .          .         .        
#> [32,]  .         .          .         .        
#> [33,]  .         .          .         .        
#> [34,]  .         .          .         .        
#> [35,]  .         .          .         .        
#> [36,]  .         .          .         .        
#> [37,]  .         .          .         .        
#> [38,]  .         .          .         .        
#> [39,]  .         .          .         .        
#> [40,]  .         .          .         .        
#> [41,]  .         .          .         .        
#> [42,]  .         .          .         .        
#> [43,]  .         .          .         .        
#> [44,]  .         .          .         .        
#> [45,]  .         .          .         .        
#> [46,]  .         .          .         .        
#> [47,]  .         .          .         .        
#> [48,]  .         .          .         .        
#> [49,]  .         .          .         .        
#> [50,]  .         .          .         .
```

### ar(1) model

``` r
library(fastcpd)
set.seed(1)
n <- 1000
p <- 1
x <- rep(0, n + 1)
for (i in 1:600) {
  x[i + 1] <- 0.6 * x[i] + rnorm(1)
}
for (i in 601:1000) {
  x[i + 1] <- 0.3 * x[i] + rnorm(1)
}
result <- fastcpd_ts(x, "ar", 1)
summary(result)
#> 
#> Call:
#> fastcpd_ts(data = x, family = "ar", order = 1)
#> 
#> Change points:
#> 609 
#> 
#> Cost values:
#> 304.2952 228.4288 
#> 
#> Parameters:
#>   segment 1 segment 2
#> 1 0.5648258 0.2227463
plot(result)
```

![](man/figures/README-ar1_model-1.png)<!-- -->

### ar(3) model with innovation standard deviation 3

``` r
library(fastcpd)
set.seed(1)
n <- 1000
p <- 1
x <- rep(0, n + 3)
for (i in 1:600) {
  x[i + 3] <- 0.6 * x[i + 2] - 0.2 * x[i + 1] + 0.1 * x[i] + rnorm(1, 0, 3)
}
for (i in 601:1000) {
  x[i + 1] <- 0.3 * x[i + 2] + 0.4 * x[i + 1] + 0.2 * x[i] + rnorm(1, 0, 3)
}
result <- fastcpd.ts(x, "ar", 3)
summary(result)
#> 
#> Call:
#> fastcpd.ts(data = x, family = "ar", order = 3)
#> 
#> Change points:
#> 615 
#> 
#> Cost values:
#> 2753.547 2022.597 
#> 
#> Parameters:
#>     segment 1   segment 2
#> 1  0.57616905  0.13006290
#> 2 -0.21476408 -0.03084403
#> 3  0.07938272 -0.04544551
plot(result)
```

![](man/figures/README-ar3_model_with_innovation_standard_deviation_3-1.png)<!-- -->

### custom logistic regression

``` r
library(fastcpd)
set.seed(1)
p <- 5
x <- matrix(rnorm(375 * p, 0, 1), ncol = p)
theta <- rbind(rnorm(p, 0, 1), rnorm(p, 2, 1))
y <- c(
  rbinom(200, 1, 1 / (1 + exp(-x[1:200, ] %*% theta[1, ]))),
  rbinom(175, 1, 1 / (1 + exp(-x[201:375, ] %*% theta[2, ])))
)
data <- data.frame(y = y, x = x)
result_builtin <- suppressWarnings(fastcpd(
  formula = y ~ . - 1,
  data = data,
  family = "binomial"
))
logistic_loss <- function(data, theta) {
  x <- data[, -1]
  y <- data[, 1]
  u <- x %*% theta
  nll <- -y * u + log(1 + exp(u))
  nll[u > 10] <- -y[u > 10] * u[u > 10] + u[u > 10]
  sum(nll)
}
logistic_loss_gradient <- function(data, theta) {
  x <- data[nrow(data), -1]
  y <- data[nrow(data), 1]
  c(-(y - 1 / (1 + exp(-x %*% theta)))) * x
}
logistic_loss_hessian <- function(data, theta) {
  x <- data[nrow(data), -1]
  prob <- 1 / (1 + exp(-x %*% theta))
  (x %o% x) * c((1 - prob) * prob)
}
result_custom <- fastcpd(
  formula = y ~ . - 1,
  data = data,
  epsilon = 1e-5,
  cost = logistic_loss,
  cost_gradient = logistic_loss_gradient,
  cost_hessian = logistic_loss_hessian
)
cat(
  "Change points detected by built-in logistic regression model: ",
  result_builtin@cp_set, "\n",
  "Change points detected by custom logistic regression model: ",
  result_custom@cp_set, "\n",
  sep = ""
)
#> Change points detected by built-in logistic regression model: 200
#> Change points detected by custom logistic regression model: 201
result_custom_two_epochs <- fastcpd(
  formula = y ~ . - 1,
  data = data,
  k = function(x) 1,
  epsilon = 1e-5,
  cost = logistic_loss,
  cost_gradient = logistic_loss_gradient,
  cost_hessian = logistic_loss_hessian
)
summary(result_custom_two_epochs)
#> 
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data, k = function(x) 1, 
#>     epsilon = 1e-05, cost = logistic_loss, cost_gradient = logistic_loss_gradient, 
#>     cost_hessian = logistic_loss_hessian)
#> 
#> Change points:
#> 200 
#> 
#> Parameters:
#>    segment 1  segment 2
#> 1 -0.6235240  2.0066479
#> 2 -1.6767614  1.6278889
#> 3 -1.7973433  4.6422022
#> 4 -0.4842969 -0.1521062
#> 5  2.0797875  2.4047092
```

### custom cost function huber regression

``` r
library(fastcpd)
set.seed(1)
n <- 400 + 300 + 500
p <- 5
x <- mvtnorm::rmvnorm(n, mean = rep(0, p), sigma = diag(p))
theta <- rbind(
  mvtnorm::rmvnorm(1, mean = rep(0, p - 3), sigma = diag(p - 3)),
  mvtnorm::rmvnorm(1, mean = rep(5, p - 3), sigma = diag(p - 3)),
  mvtnorm::rmvnorm(1, mean = rep(9, p - 3), sigma = diag(p - 3))
)
theta <- cbind(theta, matrix(0, 3, 3))
theta <- theta[rep(seq_len(3), c(400, 300, 500)), ]
y_true <- rowSums(x * theta)
factor <- c(
  2 * stats::rbinom(400, size = 1, prob = 0.95) - 1,
  2 * stats::rbinom(300, size = 1, prob = 0.95) - 1,
  2 * stats::rbinom(500, size = 1, prob = 0.95) - 1
)
y <- factor * y_true + stats::rnorm(n)
data <- cbind.data.frame(y, x)
huber_threshold <- 1
huber_loss <- function(data, theta) {
  residual <- data[, 1] - data[, -1, drop = FALSE] %*% theta
  indicator <- abs(residual) <= huber_threshold
  sum(
    residual^2 / 2 * indicator +
      huber_threshold * (
        abs(residual) - huber_threshold / 2
      ) * (1 - indicator)
  )
}
huber_loss_gradient <- function(data, theta) {
  residual <- c(data[nrow(data), 1] - data[nrow(data), -1] %*% theta)
  if (abs(residual) <= huber_threshold) {
    -residual * data[nrow(data), -1]
  } else {
    -huber_threshold * sign(residual) * data[nrow(data), -1]
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
#> 
#> Call:
#> fastcpd(formula = y ~ . - 1, data = data, beta = (p + 1) * log(n)/2, 
#>     cost = huber_loss, cost_gradient = huber_loss_gradient, cost_hessian = huber_loss_hessian)
#> 
#> Change points:
#> 401 726 
#> 
#> Parameters:
#>     segment 1   segment 2    segment 3
#> 1 -0.52615415  2.77991463  8.744706508
#> 2 -1.02443443  5.06390528  9.506534878
#> 3 -0.09220421  0.01647923 -0.008908851
#> 4 -0.01326592 -0.08103008 -0.047909865
#> 5  0.02526703  0.01329142  0.025171681
```

</details>

## Make contributions

We welcome contributions from everyone. Please follow the instructions
below to make contributions.

1.  Fork the repo.

2.  Create a new branch from `main` branch.

3.  Make changes and commit them.

    1.  Please follow the [Google’s R style
        guide](https://google.github.io/styleguide/Rguide.html) for
        naming variables and functions.
    2.  If you are adding a new family of models with new cost functions
        with corresponding gradient and Hessian, please add them to
        `src/fastcpd_class_cost.cc` with proper example and tests in
        `vignettes/gallery.Rmd` and `tests/testthat/test-gallery.R`.
    3.  Add the family name to `src/fastcpd_constants.h`.
    4.  \[Recommended\] Add a new wrapper function in
        `R/fastcpd_wrappers.R` for the new family of models and move the
        examples to the new wrapper function as roxygen examples.
    5.  Add the new wrapper function to the corresponding section in
        `_pkgdown.yml`.

4.  Push the changes to your fork.

5.  Create a pull request.

6.  Make sure the pull request does not create new warnings or errors in
    `devtools::check()`.

## Contact us

Encountered a bug or unintended behavior?

1.  File a ticket at [GitHub
    Issues](https://github.com/doccstat/fastcpd/issues).
2.  Contact the authors specified in
    [DESCRIPTION](https://github.com/doccstat/fastcpd/blob/main/DESCRIPTION#L5-L10).

## Stargazers over time

[![Stargazers over
time](https://starchart.cc/doccstat/fastcpd.svg)](https://starchart.cc/doccstat/fastcpd)

## Codecov Icicle

[![Codecov
Icicle](https://codecov.io/gh/doccstat/fastcpd/graphs/icicle.svg)](https://app.codecov.io/gh/doccstat/fastcpd?branch=main)
