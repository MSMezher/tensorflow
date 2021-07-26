/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/core/data/service/data_service.h"

#include <string>

#include "tensorflow/core/framework/dataset_options.pb.h"
#include "tensorflow/core/platform/errors.h"
#include "tensorflow/core/platform/status.h"
#include "tensorflow/core/platform/statusor.h"
#include "tensorflow/core/protobuf/data_service.pb.h"

namespace tensorflow {
namespace data {

namespace {
constexpr const char kAuto[] = "AUTO";
constexpr const char kAny[] = "ANY";
constexpr const char kLocal[] = "LOCAL";
}  // namespace

bool IsNoShard(const ProcessingModeDef& processing_mode) {
  return processing_mode.sharding_policy() == ProcessingModeDef::OFF;
}

bool IsDynamicShard(const ProcessingModeDef& processing_mode) {
  return processing_mode.sharding_policy() == ProcessingModeDef::DYNAMIC;
}

bool IsStaticShard(const ProcessingModeDef& processing_mode) {
  return processing_mode.sharding_policy() == ProcessingModeDef::FILE ||
         processing_mode.sharding_policy() == ProcessingModeDef::DATA ||
         processing_mode.sharding_policy() == ProcessingModeDef::FILE_OR_DATA ||
         processing_mode.sharding_policy() == ProcessingModeDef::HINT;
}

StatusOr<AutoShardPolicy> ToAutoShardPolicy(
    const ProcessingModeDef::ShardingPolicy sharding_policy) {
  switch (sharding_policy) {
    case ProcessingModeDef::FILE:
      return AutoShardPolicy::FILE;
    case ProcessingModeDef::DATA:
      return AutoShardPolicy::DATA;
    case ProcessingModeDef::FILE_OR_DATA:
      return AutoShardPolicy::AUTO;
    case ProcessingModeDef::HINT:
      return AutoShardPolicy::HINT;
    case ProcessingModeDef::DYNAMIC:
    case ProcessingModeDef::OFF:
      return AutoShardPolicy::OFF;
    default:
      return errors::Internal(
          "tf.data service sharding policy ",
          ProcessingModeDef::ShardingPolicy_Name(sharding_policy),
          " is not convertible to a valid auto-shard policy. If you're "
          "defining a new sharding policy, please update the policy mapping.");
  }
}

StatusOr<TargetWorkers> ParseTargetWorkers(absl::string_view s) {
  std::string str_upper = absl::AsciiStrToUpper(s);
  if (str_upper.empty() || str_upper == kAuto) {
    return TargetWorkers::AUTO;
  }
  if (str_upper == kAny) {
    return TargetWorkers::ANY;
  }
  if (str_upper == kLocal) {
    return TargetWorkers::LOCAL;
  }
  return errors::InvalidArgument("Unrecognized target workers: ", s);
}

std::string TargetWorkersToString(TargetWorkers target_workers) {
  switch (target_workers) {
    case TargetWorkers::AUTO:
      return kAuto;
    case TargetWorkers::ANY:
      return kAny;
    case TargetWorkers::LOCAL:
      return kLocal;
    default:
      DCHECK(false);
      return "UNKNOWN";
  }
}

}  // namespace data
}  // namespace tensorflow
