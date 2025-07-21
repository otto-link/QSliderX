/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General
   Public License. The full license is in the file LICENSE, distributed with
   this software. */
#pragma once
#include <map>
#include <vector>

#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>

namespace qsx
{

enum InterpolationMethod1D : int
{
  AKIMA,          ///< Akima interpolation
  AKIMA_PERIODIC, ///< Akima periodic interpolation
  CUBIC,          ///< Cubic spline interpolation
  CUBIC_PERIODIC, ///< Cubic spline periodic interpolation
  LINEAR,         ///< Linear interpolation
  POLYNOMIAL,     ///< Polynomial interpolation
  STEFFEN,        ///< Steffen interpolation (monotonic)
};

class Interpolator1D
{
public:
  Interpolator1D(const std::vector<float> &x,
                 const std::vector<float> &y,
                 InterpolationMethod1D     method = InterpolationMethod1D::LINEAR);
  ~Interpolator1D();

  float operator()(float x) const;
  float interpolate(float x) const;

private:
  gsl_spline         *interp; ///< GSL spline object used for interpolation
  gsl_interp_accel   *accel_; ///< GSL interpolation accelerator
  std::vector<double> x_data; ///< x data converted to double for GSL
  std::vector<double> y_data; ///< y data converted to double for GSL
  double              xmin;   ///< Minimum x value in the data set
  double              xmax;   ///< Maximum x value in the data set
};

} // namespace qsx
