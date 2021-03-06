//=================================================================================================
// Copyright (c) 2011, Johannes Meyer, TU Darmstadt
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Flight Systems and Automatic Control group,
//       TU Darmstadt, nor the names of its contributors may be used to
//       endorse or promote products derived from this software without
//       specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//=================================================================================================

#include <hector_pose_estimation/measurements/zerorate.h>
#include <hector_pose_estimation/system/imu_model.h>
#include <hector_pose_estimation/filter/set_filter.h>

namespace hector_pose_estimation {

template class Measurement_<ZeroRateModel>;

ZeroRateModel::ZeroRateModel()
{
  parameters().add("stddev", stddev_, 90.0*M_PI/180.0);
}

ZeroRateModel::~ZeroRateModel() {}

bool ZeroRateModel::init(PoseEstimation &estimator, State &state)
{
  gyro_drift_ = state.addSubState<3>(this, "gyro");

  if (!gyro_drift_ && state.getRateIndex() < 0) {
    ROS_WARN_NAMED("zerorate", "Updating with zero rate is a no-op, as the state does not contain rates and gyro drift estimation is disabled.");
    // return false;
  }

  return true;
}

void ZeroRateModel::getMeasurementNoise(NoiseVariance& R, const State&, bool init)
{
  if (init) {
    R = pow(stddev_, 2);
  }
}

void ZeroRateModel::getExpectedValue(MeasurementVector& y_pred, const State& state)
{
  y_pred(0) = state.getRate().z();

  if (state.getRateIndex() < 0 && gyro_drift_) {
    y_pred(0) += gyro_drift_->getVector().z();
  }
}

void ZeroRateModel::getStateJacobian(MeasurementMatrix& C0, SubMeasurementMatrix& C1, const State& state, bool)
{
  if (state.getRateIndex() >= 0) {
    C0(0, State::RATE_Z) = 1.0;
  } else if (gyro_drift_) {
    C1(0, GyroModel::BIAS_GYRO_Z) = 1.0;
  }
}

const ZeroRateModel::MeasurementVector* ZeroRateModel::getFixedMeasurementVector()
{
  static MeasurementVector zero = 0.0;
  return &zero;
}

} // namespace hector_pose_estimation
