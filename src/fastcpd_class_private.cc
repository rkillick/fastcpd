#include "fastcpd_classes.h"
#include "fastcpd_constants.h"
#include "RProgress.h"

namespace fastcpd::classes {

void Fastcpd::create_cost_function_wrapper(Nullable<Function> cost) {
  DEBUG_RCOUT(family);
  if (contain(FASTCPD_FAMILIES, family)) {
    cost_function_wrapper = std::bind(  // # nocov start
      &Fastcpd::get_cost_result,  // # nocov end
      this,
      std::placeholders::_1,
      std::placeholders::_2,
      std::placeholders::_3,
      std::placeholders::_4,
      std::placeholders::_5
    );
  } else {
    fastcpd::classes::CostFunction costFunction(cost.get());
    cost_function_wrapper = costFunction;
  }
}

void Fastcpd::create_cost_gradient_wrapper(Nullable<Function> cost_gradient) {
  if (contain(FASTCPD_FAMILIES, family)) {
    cost_gradient_wrapper = std::bind(  // # nocov start
      &Fastcpd::cost_update_gradient,  // # nocov end
      this,
      std::placeholders::_1,
      std::placeholders::_2
    );
  } else if (cost_gradient.isNotNull()) {
    fastcpd::classes::CostGradient costGradient(cost_gradient.get());
    cost_gradient_wrapper = costGradient;
  } else if (cost_gradient.isNull()) {
    // `cost_gradient` can be `NULL` in the case of vanilla PELT.
  } else {
    // # nocov start
    stop("This branch should not be reached at classes.cc: 290.");
    // # nocov end
  }
}

void Fastcpd::create_cost_hessian_wrapper(Nullable<Function> cost_hessian) {
  if (contain(FASTCPD_FAMILIES, family)) {
    cost_hessian_wrapper = std::bind(  // # nocov start
      &Fastcpd::cost_update_hessian,  // # nocov end
      this,
      std::placeholders::_1,
      std::placeholders::_2
    );
  } else if (cost_hessian.isNotNull()) {
    fastcpd::classes::CostHessian costHessian(cost_hessian.get());
    cost_hessian_wrapper = costHessian;
  } else if (cost_hessian.isNull()) {
    // `cost_hessian` can be `NULL` in the case of vanilla PELT.
  } else {
    // # nocov start
    stop("This branch should not be reached at classes.cc: 304.");
    // # nocov end
  }
}

void Fastcpd::create_gradients() {
  if (family == "binomial") {
    theta_sum.col(0) = segment_theta_hat.row(0).t();
    theta_hat.col(0) = segment_theta_hat.row(0).t();
    const double prob = 1 / (1 + exp(
      -as_scalar(theta_hat.t() * data.row(0).tail(data.n_cols - 1).t())
    ));
    hessian.slice(0) = (
      data.row(0).tail(data.n_cols - 1).t() * data.row(0).tail(data.n_cols - 1)
    ) * as_scalar(prob * (1 - prob));
  } else if (family == "poisson") {
    theta_hat.col(0) = segment_theta_hat.row(0).t();
    theta_sum.col(0) = segment_theta_hat.row(0).t();
    hessian.slice(0) = (
      data.row(0).tail(data.n_cols - 1).t() * data.row(0).tail(data.n_cols - 1)
    ) * as_scalar(
      exp(theta_hat.t() * data.row(0).tail(data.n_cols - 1).t())
    );
  } else if (family == "lasso" || family == "gaussian") {
    theta_hat.col(0) = segment_theta_hat.row(0).t();
    theta_sum.col(0) = segment_theta_hat.row(0).t();
    hessian.slice(0) = epsilon * eye<mat>(p, p) +
      data.row(0).tail(data.n_cols - 1).t() * data.row(0).tail(data.n_cols - 1);
  } else if (!contain(FASTCPD_FAMILIES, family)) {
    theta_hat.col(0) = segment_theta_hat.row(0).t();
    theta_sum.col(0) = segment_theta_hat.row(0).t();
    hessian.slice(0) = zeros<mat>(p, p);
  }
}

void Fastcpd::create_segment_indices() {
  const unsigned int segment_length = floor(n / segment_count);
  const int segment_remainder = n % segment_count;
  for (
    int segment_index = 1; segment_index <= segment_count; segment_index++
  ) {
    if (segment_index <= segment_remainder) {
      segment_indices(span(
        (segment_index - 1) * (segment_length + 1),
        segment_index * (segment_length + 1)
      )).fill(segment_index);
    } else {
      segment_indices(span(
        (segment_index - 1) * segment_length + segment_remainder,
        segment_index * segment_length + segment_remainder - 1
      )).fill(segment_index);
    }
  }
}

// TODO(doccstat): Use `segment_theta` as warm start.

void Fastcpd::create_segment_statistics() {
  for (
    int segment_index = 0; segment_index < segment_count; ++segment_index
  ) {
    DEBUG_RCOUT(segment_index);
    ucolvec segment_indices_ = find(segment_indices == segment_index + 1);
    mat data_segment = data.rows(segment_indices_);
    rowvec segment_theta;
    if (!contain(FASTCPD_FAMILIES, family)) {
      segment_theta = get_optimized_cost(data_segment).par;
    } else {
      segment_theta = cost_function_wrapper(
        data_segment, R_NilValue, 0, true, R_NilValue
      ).par;
    }
    DEBUG_RCOUT(segment_theta);

    // Initialize the estimated coefficients for each segment to be the
    // estimated coefficients in the segment.
    segment_theta_hat.row(segment_index) = segment_theta;
    if (family == "lasso" || family == "gaussian") {
      colvec segment_residual = data_segment.col(0) -
        data_segment.cols(1, data_segment.n_cols - 1) * segment_theta.t();
        double err_var = as_scalar(mean(square(segment_residual)));
        update_err_sd(segment_index, err_var);
        DEBUG_RCOUT(err_sd);
        act_num(segment_index) = accu(abs(segment_theta) > 0);
    }
  }
  if (family == "lasso") {
    beta = beta * (1 + mean(act_num));
  }
}

double Fastcpd::get_cost_adjustment_value(const unsigned nrows) {
  double adjusted = 0;
  if (cost_adjustment == "MBIC" || cost_adjustment == "MDL") {
    adjusted = data.n_cols * std::log(nrows) / 2.0;
  }
  if (cost_adjustment == "MDL") {
    adjusted *= std::log2(M_E);
  }
  return adjusted;
}

CostResult Fastcpd::get_cost_result(
    const mat& data_segment,
    Nullable<colvec> theta,
    double lambda,
    bool cv,
    Nullable<colvec> start
) {
  CostResult cost_result;
  if (theta.isNull()) {
    cost_result = get_nll_wo_theta(
      data_segment, lambda, cv, start
    );
  } else {
    cost_result = CostResult{
      {colvec()},
      {colvec()},
      get_nll_wo_cv(data_segment, as<colvec>(theta), lambda)
    };
  }
  cost_result.value = update_cost_value(cost_result.value, data_segment.n_rows);
  return cost_result;
}

List Fastcpd::get_cp_set(const colvec raw_cp_set, const double lambda) {
  colvec cp_set = update_cp_set(raw_cp_set);

  if (cp_only) {
    return List::create(
      Named("raw_cp_set") = raw_cp_set,
      Named("cp_set") = cp_set,
      Named("cost_values") = R_NilValue,
      Named("residual") = R_NilValue,
      Named("thetas") = R_NilValue
    );
  }

  colvec cp_loc_ = zeros<colvec>(cp_set.n_elem + 2);
  if (cp_set.n_elem) {
    cp_loc_.rows(1, cp_loc_.n_elem - 2) = cp_set;
  }
  cp_loc_(cp_loc_.n_elem - 1) = n;
  colvec cp_loc = unique(std::move(cp_loc_));
  colvec cost_values = zeros<vec>(cp_loc.n_elem - 1);
  mat thetas = zeros<mat>(p, cp_loc.n_elem - 1);
  mat residual;
  if (
    family == "mean" || family == "variance" ||
    family == "meanvariance" || family == "mv"
  ) {
    residual = zeros<mat>(data.n_rows, data.n_cols);
  } else if (family == "mgaussian") {
    residual = zeros<mat>(data.n_rows, p_response);
  } else {
    residual = zeros<mat>(data.n_rows, 1);
  }
  unsigned int residual_next_start = 0;

  for (unsigned int i = 0; i < cp_loc.n_elem - 1; i++) {
    colvec segment_data_index_ =
        linspace(cp_loc(i), cp_loc(i + 1) - 1, cp_loc(i + 1) - cp_loc(i));
    ucolvec segment_data_index =
        arma::conv_to<ucolvec>::from(std::move(segment_data_index_));

    mat data_segment = data.rows(segment_data_index);
    CostResult cost_result;
    if (!contain(FASTCPD_FAMILIES, family)) {
      cost_result = get_optimized_cost(data_segment);
    } else {
      cost_result = cost_function_wrapper(
        data_segment, R_NilValue, lambda, false, R_NilValue
      );
    }

    cost_values(i) = cost_result.value;

    // Parameters are not involved for PELT.
    if (vanilla_percentage < 1) {
      thetas.col(i) = colvec(cost_result.par);
    }

    // Residual is only calculated for built-in families.
    if (
      contain(FASTCPD_FAMILIES, family) &&
      !(family == "mean" || family == "variance")
    ) {
      mat cost_optim_residual = cost_result.residuals;
      residual.rows(
        residual_next_start,
        residual_next_start + cost_optim_residual.n_rows - 1
      ) = cost_optim_residual;
      residual_next_start += cost_optim_residual.n_rows;
    }
  }
  return List::create(
    Named("raw_cp_set") = raw_cp_set,
    Named("cp_set") = cp_set,
    Named("cost_values") = cost_values,
    Named("residual") = residual,
    Named("thetas") = thetas
  );
}

double Fastcpd::get_cval_for_r_t_set(
  const int tau,
  const unsigned int i,
  const int t,
  double lambda
) {
  DEBUG_RCOUT(i);
  if (family == "lasso") {
    // Mean of `err_sd` only works if error sd is unchanged.
    lambda = mean(err_sd) * sqrt(2 * std::log(p) / (t - tau));
  }
  mat data_segment;
  if (family == "mean" || family == "variance") {
    if (tau == 0) {
      data_segment =
        join_cols(zeros<rowvec>(data.n_cols), data.rows(tau, t - 1));
    } else {
      data_segment = data.rows(tau - 1, t - 1);
    }
  } else {
    data_segment = data.rows(tau, t - 1);
  }
  DEBUG_RCOUT(data_segment);
  if (t > vanilla_percentage * n) {
    return get_cval_sen(data_segment, i, t, tau, lambda);
  } else {
    return get_cval_pelt(data_segment, i, t, tau, lambda);
  }
}

double Fastcpd::get_cval_pelt(
  const mat& data_segment,
  const unsigned int i,
  const int t,
  const int tau,
  const double lambda
) {
  double cval = 0;
  CostResult cost_result;
  if (!contain(FASTCPD_FAMILIES, family)) {
    cost_result = get_optimized_cost(data_segment);
  } else {
    if (warm_start && t - tau >= 10 * p) {
      cost_result = cost_function_wrapper(
        data_segment, R_NilValue, lambda, false,
        wrap(segment_theta_hat[segment_indices(t - 1) - 1])
        // Or use `wrap(start.col(tau))` for warm start.
      );
      update_start(tau, colvec(cost_result.par));
    } else {
      cost_result = cost_function_wrapper(
        data_segment, R_NilValue, lambda, false, R_NilValue
      );
    }
  }
  cval = cost_result.value;

  // If `vanilla_percentage` is not 1, then we need to keep track of
  // thetas for later `fastcpd` steps.
  if (vanilla_percentage < 1 && t <= vanilla_percentage * n) {
    update_theta_hat(i - 1, cost_result.par);
    update_theta_sum(i - 1, cost_result.par);
  }
  return cval;
}

double Fastcpd::get_cval_sen(
  const mat& data_segment,
  const unsigned int i,
  const int t,
  const int tau,
  const double lambda
) {
  double cval = 0;
  update_cost_parameters(t, tau, i, k.get(), lambda, line_search);
  colvec theta = theta_sum.col(i - 1) / (t - tau);
  DEBUG_RCOUT(theta);
  if (!contain(FASTCPD_FAMILIES, family)) {
    Function cost_non_null = cost.get();
    SEXP cost_result = cost_non_null(data_segment, theta);
    cval = as<double>(cost_result);
  } else if (
    (family != "lasso" && t - tau >= p) ||
    (family == "lasso" && t - tau >= 3)
  ) {
    cval = cost_function_wrapper(
      data_segment, wrap(theta), lambda, false, R_NilValue
    ).value;
  }
  // else t - tau < p or for lasso t - tau < 3
  return cval;
}

CostResult Fastcpd::get_optimized_cost(const mat& data_segment) {
  Function cost_ = cost.get();
  CostResult cost_result;
  if (cost_gradient.isNull() && cost_hessian.isNull()) {
    cost_result = {{colvec()}, {colvec()}, as<double>(cost_(data_segment))};
  } else if (p == 1) {
    Environment stats = Environment::namespace_env("stats");
    Function optim = stats["optim"];
    List optim_result = optim(
      Named("par") = 0,
      Named("fn") = InternalFunction(
        +[](double theta, mat data, Function cost_) {
          return cost_(
            Named("data") = data,
            Named("theta") = std::log(theta / (1 - theta))
          );
        }
      ),
      Named("method") = "Brent",
      Named("lower") = 0,
      Named("upper") = 1,
      Named("data") = data_segment,
      Named("cost") = cost_
    );
    colvec par = as<colvec>(optim_result["par"]);
    double value = as<double>(optim_result["value"]);
    cost_result =
      {{log(par / (1 - par))}, {colvec()}, exp(value) / (1 + exp(value))};
  } else if (p > 1) {
    Environment stats = Environment::namespace_env("stats");
    Function optim = stats["optim"];
    List optim_result = optim(
      Named("par") = zeros<vec>(p),
      Named("fn") = cost_,
      Named("method") = "L-BFGS-B",
      Named("data") = data_segment,
      Named("lower") = lower,
      Named("upper") = upper
    );
    cost_result =
      {{as<colvec>(optim_result["par"])}, {colvec()}, optim_result["value"]};
  } else {
    // # nocov start
    stop("This branch should not be reached at classes.cc: 945.");
    // # nocov end
  }
  return cost_result;
}

CostResult Fastcpd::get_nll_arma(
  const mat& data,
  const colvec& order
) {
  Environment stats = Environment::namespace_env("stats");
  Function arima = stats["arima"];
  List out = arima(
    Named("x") = data.col(0),
    Named("order") = NumericVector::create(order(0), 0, order(1)),
    Named("include.mean") = false
  );
  colvec par = zeros(sum(order) + 1);
  par.rows(0, sum(order) - 1) = as<colvec>(out["coef"]);
  par(sum(order)) = as<double>(out["sigma2"]);

  return {{par}, {as<vec>(out["residuals"])}, -as<double>(out["loglik"])};
}

CostResult Fastcpd::get_nll_glm(
  const mat& data,
  Nullable<colvec> start,
  const std::string& family
) {
  vec y = data.col(0);
  Environment fastglm = Environment::namespace_env("fastglm");
  Function fastglm_ = fastglm["fastglm"];
  List out;
  if (start.isNull()) {
    mat x = data.cols(1, data.n_cols - 1);
    out = fastglm_(x, y, family);
  } else {
    colvec start_ = as<colvec>(start);
    mat x = data.cols(1, data.n_cols - 1);
    out = fastglm_(x, y, family, Named("start") = start_);
  }
  vec par = as<vec>(out["coefficients"]);
  vec residuals = as<vec>(out["residuals"]);
  double value = out["deviance"];
  return {{par}, {residuals}, value / 2};
}

CostResult Fastcpd::get_nll_lasso_cv(const mat& data) {
  Environment glmnet = Environment::namespace_env("glmnet"),
               stats = Environment::namespace_env("stats");
  Function cv_glmnet = glmnet["cv.glmnet"],
      predict_glmnet = glmnet["predict.glmnet"],
            deviance = stats["deviance"];
  List out = cv_glmnet(
    data.cols(1, data.n_cols - 1), data.col(0), Named("family") = "gaussian"
  );
  colvec index_vec = as<colvec>(out["index"]),
            values = as<colvec>(deviance(out["glmnet.fit"]));
  S4 out_coef = predict_glmnet(
    out["glmnet.fit"],
    Named("s") = out["lambda.1se"],
    Named("type") = "coefficients",
    Named("exact") = false
  );
  vec glmnet_i = as<vec>(out_coef.slot("i"));
  vec glmnet_x = as<vec>(out_coef.slot("x"));
  vec par = zeros(data.n_cols - 1);
  for (unsigned int i = 1; i < glmnet_i.n_elem; i++) {
    par(glmnet_i(i) - 1) = glmnet_x(i);
  }
  return {{par}, {mat()}, values(index_vec(1) - 1)};
}

CostResult Fastcpd::get_nll_lasso_wo_cv(
  const mat& data,
  const double lambda
) {
  Environment stats = Environment::namespace_env("stats"),
             glmnet = Environment::namespace_env("glmnet");
  Function deviance = stats["deviance"], glmnet_ = glmnet["glmnet"],
     predict_glmnet = glmnet["predict.glmnet"];
  List out = glmnet_(
    data.cols(1, data.n_cols - 1), data.col(0),
    Named("family") = "gaussian", Named("lambda") = lambda
  );
  S4 out_par = out["beta"];
  vec par_i = as<vec>(out_par.slot("i"));
  vec par_x = as<vec>(out_par.slot("x"));
  vec par = zeros(data.n_cols - 1);
  for (unsigned int i = 0; i < par_i.n_elem; i++) {
    par(par_i(i)) = par_x(i);
  }
  double value = as<double>(deviance(out));
  vec fitted_values = as<vec>(
    predict_glmnet(out, data.cols(1, data.n_cols - 1), Named("s") = lambda)
  );
  vec residuals = data.col(0) - fitted_values;
  return {{par}, {residuals}, value / 2};
}

CostResult Fastcpd::get_nll_mean(
  const mat& data,
  const mat& variance_estimate
) {
  rowvec data_n = data.row(data.n_rows - 1);
  rowvec data_1 = data.row(0);
  unsigned int p = data.n_cols;
  return {
    {zeros<colvec>(p - 1)},  // # nocov
    {zeros<mat>(data.n_rows, p - 1)},
    std::log(2.0 * M_PI) * data.n_cols + log_det_sympd(variance_estimate) *
      data.n_rows / 2.0 + (
      data_n(p - 1) - data_1(p - 1) - dot(
        data_n.subvec(0, p - 2) - data_1.subvec(0, p - 2),
        data_n.subvec(0, p - 2) - data_1.subvec(0, p - 2)
      ) / (data.n_rows - 1)
    ) / 2.0
  };
}

CostResult Fastcpd::get_nll_meanvariance(
  const mat& data,
  const double epsilon
) {
  mat covariance = cov(data);

  double value = data.n_rows * data.n_cols * (std::log(2.0 * M_PI) + 1) / 2.0;
  if (data.n_rows >= data.n_cols) {
    value += data.n_rows * log_det_sympd(
      covariance + epsilon * eye<mat>(data.n_cols, data.n_cols)
    ) / 2.0;
  }

  colvec par = zeros(data.n_cols * data.n_cols + data.n_cols);
  par.rows(0, data.n_cols - 1) = mean(data, 0).t();
  par.rows(data.n_cols, par.n_rows - 1) =
    covariance.reshape(data.n_cols * data.n_cols, 1);
  mat residuals = data.each_row() - par.rows(0, data.n_cols - 1).t();

  return {{par}, {residuals}, value};
}

CostResult Fastcpd::get_nll_mgaussian(
  const mat& data,
  const unsigned int p_response,
  const mat& variance_estimate
) {
  mat x = data.cols(p_response, data.n_cols - 1);
  mat y = data.cols(0, p_response - 1);
  mat x_t_x;

  if (data.n_rows <= data.n_cols - p_response + 1) {
    x_t_x = eye<mat>(data.n_cols - p_response, data.n_cols - p_response);
  } else {
    x_t_x = x.t() * x;
  }

  mat par = solve(x_t_x, x.t()) * y;
  DEBUG_RCOUT(par);
  mat residuals = y - x * par;
  double value =
    p_response * std::log(2.0 * M_PI) + log_det_sympd(variance_estimate);
  value *= data.n_rows;
  value += trace(solve(variance_estimate, residuals.t() * residuals));
  DEBUG_RCOUT(value);
  return {{par}, {residuals}, value / 2};
}

CostResult Fastcpd::get_nll_variance(const mat& data) {
  rowvec data_n = data.row(data.n_rows - 1);
  rowvec data_1 = data.row(0);
  const unsigned int p = sqrt(data.n_cols);
  double det_value =
    det(arma::reshape(data_n - data_1, p, p) / (data.n_rows - 1));
  if (det_value <= 0) {
    det_value = 1e-10;
  }
  return {
    {zeros<mat>(p, p)},
    {zeros<mat>(data.n_rows, p)},
    (std::log(2.0 * M_PI) * p + p + log(det_value)) * (data.n_rows - 1) / 2.0
  };
}

void Fastcpd::update_cost_parameters(
  const unsigned int t,
  const unsigned int tau,
  const unsigned int i,
  Function k,
  const double lambda,
  const colvec& line_search
) {
  List cost_update_result = update_cost_parameters_steps(
    data.rows(0, t - 1), tau, i, k, momentum, lambda, line_search
  );
  DEBUG_RCOUT(line_search);
  update_theta_hat(i - 1, as<colvec>(cost_update_result[0]));
  create_theta_sum(i - 1, as<colvec>(cost_update_result[1]));
  update_hessian(i - 1, as<mat>(cost_update_result[2]));
  update_momentum(as<colvec>(cost_update_result[3]));
}

void Fastcpd::update_cost_parameters_step(
  const mat& data,
  const int i,
  const int data_start,
  const int data_end,
  const double lambda,
  const colvec& line_search
) {
  DEBUG_RCOUT(data_start);
  mat hessian_i = hessian.slice(i - 1);
  colvec gradient;

  if (!contain(FASTCPD_FAMILIES, family)) {
    mat cost_hessian_result = cost_hessian_wrapper(
      data.rows(data_start, data_end), theta_hat.col(i - 1)
    );
    DEBUG_RCOUT(cost_hessian_result);
    hessian_i += cost_hessian_result;
    colvec cost_gradient_result = cost_gradient_wrapper(
      data.rows(data_start, data_end), theta_hat.col(i - 1)
    );
    gradient = cost_gradient_result;
    DEBUG_RCOUT(gradient);
  } else {
    hessian_i += cost_update_hessian(
      data.rows(data_start, data_end), theta_hat.col(i - 1)
    );
    gradient = cost_update_gradient(
      data.rows(data_start, data_end), theta_hat.col(i - 1)
    );
  }

  // Add epsilon to the diagonal for PSD hessian
  mat hessian_psd =
    hessian_i + epsilon * eye<mat>(theta_hat.n_rows, theta_hat.n_rows);

  // Calculate momentum step
  momentum = momentum_coef * momentum - solve(hessian_psd, gradient);
  DEBUG_RCOUT(momentum);

  double best_learning_rate = 1;
  colvec line_search_costs = zeros<colvec>(line_search.n_elem);

  // Line search
  if (line_search.n_elem > 1 || line_search[0] != 1) {
    for (
      unsigned int line_search_index = 0;
      line_search_index < line_search.n_elem;
      line_search_index++
    ) {
      colvec theta_candidate =
        theta_hat.col(i - 1) + line_search[line_search_index] * momentum;
      DEBUG_RCOUT(theta_candidate);
      colvec theta_upper_bound = arma::min(std::move(theta_candidate), upper);
      colvec theta_projected = arma::max(std::move(theta_upper_bound), lower);
      line_search_costs[line_search_index] = cost_function_wrapper(
        data, wrap(theta_projected), lambda, false, R_NilValue
      ).value;
    }
  }
  best_learning_rate = line_search[line_search_costs.index_min()];
  DEBUG_RCOUT(best_learning_rate);

  // Update theta_hat with momentum
  theta_hat.col(i - 1) += best_learning_rate * momentum;

  theta_hat.col(i - 1) = arma::min(theta_hat.col(i - 1), upper);
  theta_hat.col(i - 1) = arma::max(theta_hat.col(i - 1), lower);

  if (family == "lasso" || family == "gaussian") {
    // Update theta_hat with L1 penalty
    double hessian_norm = norm(hessian_i, "fro");
    vec normd = abs(theta_hat.col(i - 1)) - lambda / hessian_norm;
    theta_hat.col(i - 1) = sign(theta_hat.col(i - 1)) % arma::max(
      normd, zeros<colvec>(normd.n_elem)
    );
  }

  hessian.slice(i - 1) = std::move(hessian_i);
}

List Fastcpd::update_cost_parameters_steps(
    const mat& data,
    const int tau,
    const int i,
    Function k,
    colvec momentum,
    const double lambda,
    const colvec& line_search
) {
  update_cost_parameters_step(data, i, 0, data.n_rows - 1, lambda, line_search);

  for (int kk = 1; kk <= as<int>(k(data.n_rows - tau)); kk++) {
    for (unsigned j = tau + 1; j <= data.n_rows; j++) {
      update_cost_parameters_step(data, i, tau, j - 1, lambda, line_search);
    }
  }

  theta_sum.col(i - 1) += theta_hat.col(i - 1);
  return List::create(
    theta_hat.col(i - 1), theta_sum.col(i - 1), hessian.slice(i - 1), momentum
  );
}

double Fastcpd::update_cost_value(
  double value,
  const unsigned int nrows
) {
  if (cost_adjustment == "MDL") {
    value = value * std::log2(M_E);
  }
  return value + get_cost_adjustment_value(nrows);
}

colvec Fastcpd::update_cp_set(const colvec raw_cp_set) {
  // Remove change points close to the boundaries.
  colvec cp_set = raw_cp_set;
  cp_set = cp_set(find(cp_set > trim * n));
  cp_set = cp_set(find(cp_set < (1 - trim) * n));
  colvec cp_set_ = zeros<vec>(cp_set.n_elem + 1);
  if (cp_set.n_elem) {
    cp_set_.rows(1, cp_set_.n_elem - 1) = std::move(cp_set);
  }
  cp_set = sort(unique(std::move(cp_set_)));

  // Remove change points close to each other.
  ucolvec cp_set_too_close = find(diff(cp_set) <= trim * n);
  if (cp_set_too_close.n_elem > 0) {
    int rest_element_count = cp_set.n_elem - cp_set_too_close.n_elem;
    colvec cp_set_rest_left = zeros<vec>(rest_element_count),
          cp_set_rest_right = zeros<vec>(rest_element_count);
    for (unsigned int i = 0, i_left = 0, i_right = 0; i < cp_set.n_elem; i++) {
      if (
        ucolvec left_find = find(cp_set_too_close == i);
        left_find.n_elem == 0
      ) {
        cp_set_rest_left(i_left) = cp_set(i);
        i_left++;
      }
      if (
        ucolvec right_find = find(cp_set_too_close == i - 1);
        right_find.n_elem == 0
      ) {
        cp_set_rest_right(i_right) = cp_set(i);
        i_right++;
      }
    }
    cp_set = floor((cp_set_rest_left + cp_set_rest_right) / 2);
  }
  return cp_set(find(cp_set > 0));
}

void Fastcpd::update_err_sd(
  const unsigned int segment_index, const double err_var
) {
  err_sd(segment_index) = sqrt(err_var);
}

void Fastcpd::update_fastcpd_parameters(const unsigned int t) {
  // for tau = t-1
  rowvec new_data = data.row(t - 1).tail(data.n_cols - 1);
  const int segment_index = segment_indices(t - 1);
  rowvec cum_coef_add = segment_theta_hat.row(segment_index - 1),
             coef_add = segment_theta_hat.row(segment_index - 1);
  mat hessian_new;
  if (family == "binomial") {
    const double prob = 1 / (1 + exp(-as_scalar(coef_add * new_data.t())));
    hessian_new = (new_data.t() * new_data) * as_scalar(prob * (1 - prob));
  } else if (family == "poisson") {
    cum_coef_add = coef_add;
    hessian_new =
        (new_data.t() * new_data) * as_scalar(
          exp(coef_add * new_data.t())
        );
  } else if (family == "lasso" || family == "gaussian") {
    hessian_new = new_data.t() * new_data + epsilon * eye<mat>(p, p);
  } else if (family == "arma") {
    hessian_new = cost_update_hessian(data.rows(0, t - 1), coef_add.t());
  } else if (!contain(FASTCPD_FAMILIES, family)) {
    hessian_new = zeros<mat>(p, p);
  }

  update_theta_hat(coef_add.t());
  update_theta_sum(cum_coef_add.t());
  update_hessian(hessian_new);
}

void Fastcpd::update_hessian(
  const unsigned int slice, mat new_hessian
) {
  hessian.slice(slice) = new_hessian;
}

void Fastcpd::update_hessian(mat new_hessian) {
  hessian = join_slices(hessian, new_hessian);
}

void Fastcpd::update_hessian(ucolvec pruned_left) {
  hessian = hessian.slices(pruned_left);
}

void Fastcpd::update_momentum(colvec new_momentum) {
  momentum = new_momentum;
}

void Fastcpd::update_start(const unsigned int col, const colvec start_col) {
  start.col(col) = start_col;
}

void Fastcpd::update_theta_hat(colvec new_theta_hat) {
  theta_hat = join_rows(theta_hat, new_theta_hat);
}

void Fastcpd::update_theta_hat(
  const unsigned int col, colvec new_theta_hat
) {
  theta_hat.col(col) = new_theta_hat;
}

void Fastcpd::update_theta_hat(ucolvec pruned_left) {
  theta_hat = theta_hat.cols(pruned_left);
}

void Fastcpd::update_theta_sum(colvec new_theta_sum) {
  theta_sum = join_rows(theta_sum, new_theta_sum);
}

void Fastcpd::update_theta_sum(ucolvec pruned_left) {
  theta_sum = theta_sum.cols(pruned_left);
}

}  // namespace fastcpd::classes
