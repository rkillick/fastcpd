#include "fastcpd_classes.h"
#include "fastcpd_constants.h"
#include "fastcpd_impl.h"
#include "RProgress.h"

List fastcpd_impl(
    mat data,
    double beta,
    const string cost_adjustment,
    const int segment_count,
    const double trim,
    const double momentum_coef,
    Function k,
    const string family,
    const double epsilon,
    const double min_prob,
    const double winsorise_minval,
    const double winsorise_maxval,
    const int p,
    const colvec order,
    Nullable<Function> cost,
    Nullable<Function> cost_gradient,
    Nullable<Function> cost_hessian,
    const bool cp_only,
    const double vanilla_percentage,
    const bool warm_start,
    colvec lower,
    colvec upper,
    colvec line_search,
    const mat variance_estimate,
    const unsigned int p_response,
    const bool r_progress
) {
  // Set up the initial values.
  const int n = data.n_rows;
  double lambda = 0;
  mat start = zeros<mat>(p, n);

  ucolvec r_t_set = {0};

  std::vector<colvec> cp_sets = {zeros<vec>(0)};
  linspace(1, n, n).for_each([&](int i) {
    cp_sets.push_back(zeros<vec>(1));
  });

  colvec fvec = zeros<vec>(n + 1);
  fvec.fill(arma::datum::inf);
  fvec(0) = -beta;

  RProgress::RProgress rProgress("[:bar] :current/:total in :elapsed", n);

  if (r_progress) {
    rProgress.tick(0);
  }

  fastcpd::classes::Fastcpd fastcpd_class(
    beta, cost_adjustment, data, epsilon, family, lower,
    min_prob, momentum_coef, order, p, p_response, segment_count, upper,
    vanilla_percentage, variance_estimate, winsorise_maxval, winsorise_minval
  );

  fastcpd_class.create_cost_function_wrapper(cost);
  fastcpd_class.create_cost_gradient_wrapper(cost_gradient);
  fastcpd_class.create_cost_hessian_wrapper(cost_hessian);

  if (contain(FASTCPD_FAMILIES, family) || vanilla_percentage < 1) {
    fastcpd_class.create_segment_indices();
    fastcpd_class.create_segment_statistics();
  }

  if (vanilla_percentage < 1) {
    fastcpd_class.create_gradients();
  }

  checkUserInterrupt();
  if (r_progress) {
    rProgress.tick();
  }

#ifdef FASTCPD_DEBUG

  colvec cvec = zeros<vec>(n + 1);
  colvec tau_stars = zeros<vec>(n + 1);

  for (int t = 1; t <= n; t++) {
    cvec.fill(arma::datum::inf);
    for (int s = t; s <= n; s++) {
      List cost_optim_result =
        fastcpd_class.negative_log_likelihood(
          data.rows(t - 1, s - 1), R_NilValue, lambda, false, R_NilValue
      );
      cvec(s) = as<double>(cost_optim_result["value"]);
    }

    colvec new_fvec = fvec(t - 1) + cvec + beta;
    ucolvec f_t_condition = arma::find(new_fvec < fvec);
    if (f_t_condition.n_elem > 0) {
      fvec.rows(f_t_condition) = new_fvec.rows(f_t_condition);
      tau_stars.rows(f_t_condition) = (t - 1) * ones<vec>(f_t_condition.n_elem);
    }
    cp_sets[t] = join_cols(cp_sets[tau_stars(t - 1)], colvec{tau_stars(t - 1)});

    checkUserInterrupt();
    if (r_progress) {
      rProgress.tick();
    }
  }

#else

  for (int t = 1; t <= n; t++) {
    unsigned int r_t_count = r_t_set.n_elem;

    // Number of cost values is the same as the number of elements in R_t.
    colvec cval = zeros<vec>(r_t_count);

    // For tau in R_t \ {t-1}.
    for (unsigned int i = 1; i < r_t_count; i++) {
      int tau = r_t_set(i - 1);
      if (family == "lasso") {
        // Mean of `err_sd` only works if error sd is unchanged.
        lambda =
          mean(fastcpd_class.get_err_sd()) * sqrt(2 * std::log(p) / (t - tau));
      }
      mat data_segment = data.rows(tau, t - 1);
      if (t > vanilla_percentage * n) {
        // fastcpd
        fastcpd_class.update_cost_parameters(t, tau, i, k, lambda, line_search);
        colvec theta =
            fastcpd_class.get_theta_sum().col(i - 1) / (t - tau);
        if (family == "poisson" && t - tau >= p) {
          theta = clamp(
            theta, winsorise_minval, winsorise_maxval
          );
        }
        if (!contain(FASTCPD_FAMILIES, family)) {
          Function cost_non_null = fastcpd_class.cost.get();
          SEXP cost_result = cost_non_null(data_segment, theta);
          cval(i - 1) = as<double>(cost_result);
        } else if (
          (family != "lasso" && t - tau >= p) ||
          (family == "lasso" && t - tau >= 3)
        ) {
          List cost_result = fastcpd_class.negative_log_likelihood(
            data_segment, wrap(theta), lambda, false, R_NilValue
          );
          cval(i - 1) = as<double>(cost_result["value"]);
        } else {
          // t - tau < p or for lasso t - tau < 3
        }
      } else {
        // vanilla PELT
        List cost_optim_result;
        if (!contain(FASTCPD_FAMILIES, family)) {
          cost_optim_result = fastcpd_class.get_optimized_cost(data_segment);
        } else {
          if (warm_start && t - tau >= 10 * p) {
            cost_optim_result =
              fastcpd_class.negative_log_likelihood(
                data_segment, R_NilValue, lambda, false,
                fastcpd_class.get_segment_theta_hat(t)
                // Or use `wrap(start.col(tau))` for warm start.
            );
            start.col(tau) = as<colvec>(cost_optim_result["par"]);
          } else {
            cost_optim_result =
              fastcpd_class.negative_log_likelihood(
                data_segment, R_NilValue, lambda, false, R_NilValue
            );
          }
        }
        cval(i - 1) = as<double>(cost_optim_result["value"]);

        // If `vanilla_percentage` is not 1, then we need to keep track of
        // thetas for later `fastcpd` steps.
        if (vanilla_percentage < 1 && t <= vanilla_percentage * n) {
          fastcpd_class.update_theta_hat(
            i - 1, as<colvec>(cost_optim_result["par"])
          );
          fastcpd_class.update_theta_sum(
            i - 1, as<colvec>(cost_optim_result["par"])
          );
        }
      }
    }

    cval(r_t_count - 1) = 0;


    if (vanilla_percentage != 1) {
      fastcpd_class.update_fastcpd_parameters(t);
    }

    // `beta` adjustment seems to work but there might be better choices.
    colvec obj = cval + fvec.rows(r_t_set) + beta;
    double min_obj = min(obj);
    double tau_star = r_t_set(index_min(obj));

    // Step 4
    cp_sets[t] = join_cols(cp_sets[tau_star], colvec{tau_star});

    // Step 5
    ucolvec pruned_left = arma::find(cval + fvec.rows(r_t_set) <= min_obj);
    ucolvec pruned_r_t_set = zeros<ucolvec>(pruned_left.n_elem + 1);
    pruned_r_t_set.rows(0, pruned_left.n_elem - 1) = r_t_set(pruned_left);
    pruned_r_t_set(pruned_left.n_elem) = t;
    r_t_set = std::move(pruned_r_t_set);

    if (vanilla_percentage != 1) {
      fastcpd_class.update_theta_hat(pruned_left);
      fastcpd_class.update_theta_sum(pruned_left);
      fastcpd_class.update_hessian(pruned_left);
    }

    // Objective function F(t).
    fvec(t) = min_obj;

    checkUserInterrupt();
    if (r_progress) {
      rProgress.tick();
    }
  }

#endif

  // Remove change points close to the boundaries.
  colvec raw_cp_set = cp_sets[n],
         cp_set = cp_sets[n];
  cp_set = cp_set(arma::find(cp_set > trim * n));
  cp_set = cp_set(arma::find(cp_set < (1 - trim) * n));
  colvec cp_set_ = zeros<vec>(cp_set.n_elem + 1);
  if (cp_set.n_elem) {
    cp_set_.rows(1, cp_set_.n_elem - 1) = std::move(cp_set);
  }
  cp_set = sort(unique(std::move(cp_set_)));

  // Remove change points close to each other.
  ucolvec cp_set_too_close = arma::find(diff(cp_set) <= trim * n);
  if (cp_set_too_close.n_elem > 0) {
    int rest_element_count = cp_set.n_elem - cp_set_too_close.n_elem;
    colvec cp_set_rest_left = zeros<vec>(rest_element_count),
          cp_set_rest_right = zeros<vec>(rest_element_count);
    for (unsigned int i = 0, i_left = 0, i_right = 0; i < cp_set.n_elem; i++) {
      if (
        ucolvec left_find = arma::find(cp_set_too_close == i);
        left_find.n_elem == 0
      ) {
        cp_set_rest_left(i_left) = cp_set(i);
        i_left++;
      }
      if (
        ucolvec right_find = arma::find(cp_set_too_close == i - 1);
        right_find.n_elem == 0
      ) {
        cp_set_rest_right(i_right) = cp_set(i);
        i_right++;
      }
    }
    cp_set = floor((cp_set_rest_left + cp_set_rest_right) / 2);
  }
  cp_set = cp_set(arma::find(cp_set > 0));

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
    List cost_optim_result;
    if (!contain(FASTCPD_FAMILIES, family)) {
      cost_optim_result = fastcpd_class.get_optimized_cost(data_segment);
    } else {
      cost_optim_result = fastcpd_class.negative_log_likelihood(
        data_segment, R_NilValue, lambda, false, R_NilValue
      );
    }

    cost_values(i) = as<double>(cost_optim_result["value"]);

    // Parameters are not involved for PELT.
    if (vanilla_percentage < 1) {
      thetas.col(i) = as<colvec>(cost_optim_result["par"]);
    }

    // Residual is only calculated for built-in families.
    if (contain(FASTCPD_FAMILIES, family)) {
      mat cost_optim_residual = as<mat>(cost_optim_result["residuals"]);
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
