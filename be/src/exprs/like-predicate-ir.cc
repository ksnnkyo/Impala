// Copyright 2012 Cloudera Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "exprs/like-predicate.h"

using namespace impala_udf;

namespace impala {

BooleanVal LikePredicate::Like(FunctionContext* context, const StringVal& val,
    const StringVal& pattern) {
  LikePredicateState* state = reinterpret_cast<LikePredicateState*>(
      context->GetFunctionState(FunctionContext::THREAD_LOCAL));
  return (state->function_)(context, val, pattern);
}

BooleanVal LikePredicate::Regex(FunctionContext* context, const StringVal& val,
    const StringVal& pattern) {
  LikePredicateState* state = reinterpret_cast<LikePredicateState*>(
      context->GetFunctionState(FunctionContext::THREAD_LOCAL));
  return (state->function_)(context, val, pattern);
}

BooleanVal LikePredicate::RegexpLike(FunctionContext* context, const StringVal& val,
    const StringVal& pattern, const StringVal& match_parameter) {
  return RegexpLikeInternal(context, val, pattern, match_parameter);
}

}  // namespace impala
