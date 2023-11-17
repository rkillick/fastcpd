#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <algorithm>

#if defined(__cplusplus) && __cplusplus >= 201703L

#include <array>
template<typename C, typename T>
bool contain(C&& c, T e) {
  return std::find(std::begin(c), std::end(c), e) != std::end(c);
};
constexpr std::array FASTCPD_FAMILIES =
  {"gaussian", "binomial", "poisson", "lasso", "arma"};

#else

#include <string>
#include <vector>
inline bool contain(std::vector<std::string> c, std::string e) {
  return std::find(std::begin(c), std::end(c), e) != std::end(c);
};
const std::vector<std::string> FASTCPD_FAMILIES =
  {"gaussian", "binomial", "poisson", "lasso", "arma"};

#endif  // defined(__cplusplus) && __cplusplus >= 201703L

// ``` r
// set.seed(1)
// arima.sim(list(ar = c(0.1, -0.2, 0.6), ma = c(0.1, 0.5)), n = 200)
// ```
constexpr std::array<double, 200> time_series_arma_3_2 = {
   0.16086394,  1.36633450,  0.71425776, -0.23698470,  1.13222067,
  -0.81209066,  1.04080322,  2.50485157,  0.10240048,  0.04309484,
   1.76843576, -0.37048506,  2.30809336,  1.49936227,  1.35265076,
   1.29760473,  0.36324437,  0.71686001, -1.38011604,  1.31601817,
   0.23507054,  1.85294823,  1.69728898,  0.36409193,  1.58620681,
  -0.12382253, -1.15284844,  0.56023783, -0.82866400, -0.78412375,
   0.27626904, -0.89428088, -1.20560631, -0.26274899,  0.55851260,
  -2.08831014,  0.55245008,  0.43857619,  0.07374949,  0.21973100,
   1.14151925,  0.26646413, -0.40061958,  1.87872196,  1.43780397,
   0.94785207,  3.17151372,  2.05753994, -0.28716435,  1.04100239,
  -0.52417819, -1.31541151, -0.68211713, -0.37625744, -1.90734262,
  -0.43675851, -0.98220438,  0.62556888,  0.56308654,  1.20736086,
   1.21701663,  2.39376315,  0.44496334,  0.61182334,  2.47669709,
  -0.34600745,  0.28084458,  0.84441002, -0.64229642, -0.57212145,
   0.88417372, -0.45000449, -0.84170441,  1.74011253, -0.26090482,
  -0.40863069,  0.82994931,  0.62104715, -0.40145671,  0.64495703,
  -0.20479077, -0.80159791,  0.03467575, -0.70410088, -0.05561702,
  -1.60362778, -0.15123335, -1.99275907, -1.43254808, -1.16204543,
  -1.88076120, -1.20144209, -2.68731572, -0.20020090, -2.70791602,
  -1.88487685, -1.76178438, -2.51812889,  0.42374247, -0.66036783,
  -1.90264031, -1.56448276, -0.52916463, -1.67544799, -1.09523234,
  -1.06238132, -2.63189117, -2.39647166, -0.20191830, -2.17889216,
  -2.56192766,  1.47864928, -0.72726882, -1.16710509,  2.16310135,
   0.88631727, -1.04561085,  3.60342576,  0.75721680, -1.61927610,
   1.43432190,  0.40246854,  1.03834924,  1.32430373,  1.78780989,
   0.55837792,  0.37964132,  0.89227190,  0.96549226,  2.28252559,
   2.19925399,  2.69098578,  0.60192677,  2.30626534,  1.42748530,
  -0.91065824,  1.49145646,  0.34749113,  0.89928610,  0.21630369,
   0.27574153, -0.82828660, -0.49301554,  0.20304732, -1.15816310,
   0.50596914, -1.08686163, -1.65838396,  1.08743329, -1.60742076,
  -0.34229029,  0.09191278, -0.30952153,  1.28456656,  2.20568531,
   0.45012999, -1.15041623,  3.22202770,  0.59657857,  0.58186733,
   2.24635394,  1.24564121, -0.09565102,  1.74843029,  0.50099276,
  -1.55686262,  1.44386747,  1.68977984, -0.71676002, -0.06760279,
   1.51300168, -0.87236517, -1.84696719,  0.70403277, -1.58031874,
  -1.80995143, -0.94833112,  0.83596631, -1.54181203, -1.62742880,
   0.51827539, -1.06763563, -2.04778834, -2.53620199, -1.90693103,
  -1.85012658, -1.64826101, -0.75785666, -0.33506819, -2.05035803,
   0.66580539, -0.21328442, -0.12953955,  1.53135295,  0.73161908,
  -0.65013812,  1.89430814, -1.56479241, -1.08874870,  0.03043624
};

#endif  // CONSTANTS_H_
