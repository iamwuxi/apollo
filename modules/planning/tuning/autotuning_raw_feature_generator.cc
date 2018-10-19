/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "modules/planning/tuning/autotuning_raw_feature_generator.h"
#include "modules/planning/common/planning_gflags.h"

namespace apollo {
namespace planning {

namespace {
constexpr double kMinTimeRange = 0.001;
// used when speed profile is invalid
constexpr double kDefaultSpeedLimit = 10.0;

constexpr double kMaxAwareDistance = 1e3;
}  // namespace

using apollo::common::ErrorCode;
using apollo::common::Status;

AutotuningRawFeatureGenerator::AutotuningRawFeatureGenerator(
    const double time_range, const std::size_t num_points,
    const ReferenceLineInfo& reference_line_info, const Frame& frame,
    const SpeedLimit& speed_limit)
    : reference_line_info_(reference_line_info),
      frame_(frame),
      speed_limit_(speed_limit),
      obs_boundaries_(num_points, std::vector<std::array<double, 3>>()),
      stop_boundaries_(num_points, std::vector<std::array<double, 3>>()),
      nudge_boundaries_(num_points, std::vector<std::array<double, 3>>()),
      side_pass_boundaries_(num_points, std::vector<std::array<double, 3>>()) {
  CHECK_GT(num_points, 0);
  CHECK_GT(time_range, kMinTimeRange);
  double res = time_range / num_points;
  for (double t = 0; t < res + time_range; t += res) {
    eval_time_.push_back(t);
  }
}

common::Status AutotuningRawFeatureGenerator::EvaluateTrajectory(
    const std::vector<common::TrajectoryPoint>& trajectory,
    autotuning::TrajectoryRawFeature* const trajectory_feature) const {
  return common::Status::OK();
}

common::Status AutotuningRawFeatureGenerator::EvaluateTrajectoryPoint(
    const common::TrajectoryPoint& trajectory_point,
    autotuning::TrajectoryPointRawFeature* const trajectory_point_feature)
    const {
  return common::Status::OK();
}

common::Status AutotuningRawFeatureGenerator::EvaluateSpeedPoint(
    const common::SpeedPoint& speed_point, const std::size_t index,
    autotuning::TrajectoryPointRawFeature* const trajectory_point_feature)
    const {
  auto* speed_feature = trajectory_point_feature->mutable_speed_feature();
  // setup basic speed profile
  const double s = speed_point.s();
  speed_feature->set_s(s);
  speed_feature->set_t(speed_point.t());
  speed_feature->set_v(speed_point.v());
  speed_feature->set_a(speed_point.a());
  speed_feature->set_j(speed_point.da());

  // get speed limit at certain s();
  if (speed_limit_.MinValidS() > s) {
    speed_feature->set_speed_limit(kDefaultSpeedLimit);
  } else {
    double cur_speed_limit = speed_limit_.GetSpeedLimitByS(s);
    speed_feature->set_speed_limit(cur_speed_limit);
  }

  // extracting obstacle related infos

  autotuning::SpeedPointRawFeature_ObjectDecisionFeature* decision_obj =
      nullptr;
  double distance = kMaxAwareDistance;
  for (const auto& stop_obs : stop_boundaries_[index]) {
    double lower_s = stop_obs[0];
    double speed = stop_obs[2];
    // stop line is in the front
    if (lower_s < s) {
      distance = lower_s - s;
      decision_obj = speed_feature->add_stop();
    } else {
      distance = 0.0;
      decision_obj = speed_feature->add_collision();
    }
    decision_obj->set_relative_s(distance);
    decision_obj->set_relative_v(speed - speed_point.v());
  }

  for (const auto& obs : obs_boundaries_[index]) {
    double lower_s = obs[0];
    double upper_s = obs[1];
    double speed = obs[2];
    if (upper_s < s) {
      decision_obj = speed_feature->add_overtake();
      distance = s - upper_s;
    } else if (lower_s > s) {
      decision_obj = speed_feature->add_follow();
      distance = lower_s - s;
    } else {
      decision_obj = speed_feature->add_collision();
      distance = 0.0;
    }
    decision_obj->set_relative_s(distance);
    decision_obj->set_relative_v(speed - speed_point.v());
  }
  return common::Status::OK();
}

common::Status AutotuningRawFeatureGenerator::EvaluateSpeedProfile(
    const std::vector<common::SpeedPoint>& speed_profile,
    autotuning::TrajectoryRawFeature* const trajectory_feature) const {
  if (speed_profile.size() != eval_time_.size()) {
    AERROR << "Evaluated time size and speed profile size is different";
    return Status(ErrorCode::PLANNING_ERROR,
                  "mismatched evaluated time and speed profile size");
  }
  for (std::size_t i = 0; i < eval_time_.size(); ++i) {
    auto* trajectory_point_feature = trajectory_feature->add_point_feature();
    auto status =
        EvaluateSpeedPoint(speed_profile[i], i, trajectory_point_feature);
    if (status != common::Status::OK()) {
      return Status(ErrorCode::PLANNING_ERROR,
                    "Extracting speed profile error");
    }
  }
  return common::Status::OK();
}

void AutotuningRawFeatureGenerator::GenerateSTBoundaries(
    const ReferenceLineInfo& reference_line_info) {
  const auto& path_decision = reference_line_info.path_decision();
  const auto& adc_sl_boundary = reference_line_info.AdcSlBoundary();
  for (auto* obstacle : path_decision.path_obstacles().Items()) {
    auto id = obstacle->Id();
    const double speed = obstacle->obstacle()->Speed();
    if (!obstacle->st_boundary().IsEmpty()) {
      // fill discretized boundary info
      ConvertToDiscretizedBoundaries(obstacle->st_boundary(), speed);
    } else if (FLAGS_enable_side_vehicle_st_boundary &&
               (adc_sl_boundary.start_l() > 2.0 ||
                adc_sl_boundary.end_l() < -2.0)) {
      if (path_decision.Find(id)->reference_line_st_boundary().IsEmpty()) {
        continue;
      }
      ADEBUG << "obstacle " << id << " is NOT blocking.";
      auto st_boundary_copy =
          path_decision.Find(id)->reference_line_st_boundary();
      auto st_boundary = st_boundary_copy.CutOffByT(3.5);
      if (!st_boundary.IsEmpty()) {
        auto decision = obstacle->LongitudinalDecision();
        if (decision.has_yield()) {
          st_boundary.SetBoundaryType(StBoundary::BoundaryType::YIELD);
        } else if (decision.has_overtake()) {
          st_boundary.SetBoundaryType(StBoundary::BoundaryType::OVERTAKE);
        } else if (decision.has_follow()) {
          st_boundary.SetBoundaryType(StBoundary::BoundaryType::FOLLOW);
        } else if (decision.has_stop()) {
          st_boundary.SetBoundaryType(StBoundary::BoundaryType::STOP);
        }
        st_boundary.SetId(st_boundary_copy.id());
        st_boundary.SetCharacteristicLength(
            st_boundary_copy.characteristic_length());
        boundaries_.push_back(&obstacle->st_boundary());
        ConvertToDiscretizedBoundaries(obstacle->st_boundary(), speed);
      }
    }
  }
}

void AutotuningRawFeatureGenerator::ConvertToDiscretizedBoundaries(
    const StBoundary& boundary, const double speed) {
  for (std::size_t i = 0; i < eval_time_.size(); ++i) {
    double upper = 0.0;
    double lower = 0.0;
    bool suc = boundary.GetBoundarySRange(eval_time_[i], &upper, &lower);
    if (suc) {
      if (boundary.boundary_type() == StBoundary::BoundaryType::STOP) {
        stop_boundaries_[i].push_back({{lower, upper, speed}});
      } else {
        obs_boundaries_[i].push_back({{lower, upper, speed}});
      }
    }
  }
}

}  // namespace planning
}  // namespace apollo