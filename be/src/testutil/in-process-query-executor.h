// Copyright (c) 2011 Cloudera, Inc. All rights reserved.

#ifndef IMPALA_TESTUTIL_QUERY_IN_PROCESS_EXECUTOR_H
#define IMPALA_TESTUTIL_QUERY_IN_PROCESS_EXECUTOR_H

#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

#include "testutil/query-executor-if.h"
#include "common/status.h"
#include "runtime/coordinator.h"
#include "runtime/primitive-type.h"
#include "runtime/runtime-state.h"
#include "gen-cpp/Frontend_types.h"  // for TQueryExecRequest
#include "util/runtime-profile.h"

namespace apache { namespace thrift { namespace transport { class TTransport; } } }
namespace apache { namespace thrift { namespace protocol { class TProtocol; } } }

namespace impala {

class DataStreamMgr;
class ExecStats;
class Expr;
class ObjectPool;
class PlanFragmentExecutor;
class RowBatch;
class RowDescriptor;
class TRowBatch;
class ExecEnv;
class ImpalaPlanServiceClient;
class ImpalaInternalServiceClient;
class RuntimeProfile;
class TPlanExecRequest;
class TScanRange;
class TPlanExecParams;

// Query execution for tests.
// Handles local or multi-threaded distributed execution based on the value
// of FLAGS_num_backends (> 0: distributed execution with that many non-coordinator
// threads).
// Results are returned either as the row batches produced by the coordinator
// fragment (FetchResult(RowBatch**)) or as individual rows
// (FetchResult(string*)/FetchResult(vector<void*>*)). Do *not* mix calls to the
// two types of FetchResult().
class InProcessQueryExecutor : public QueryExecutorIf {
 public:
  InProcessQueryExecutor(ExecEnv* exec_env);
  ~InProcessQueryExecutor();

  virtual Status Setup();

  // Start running query. Call this prior to FetchResult().
  // If 'col_types' is non-NULL, returns the types of the select list items.
  virtual Status Exec(
      const std::string& query, std::vector<PrimitiveType>* col_types);

  // Return the explain plan for the query
  virtual Status Explain(const std::string& query, std::string* explain_plan);

  // Returns result batch in 'batch'. The returned rows are the output rows of
  // the execution tree. In other words, they do *not* reflect the query's
  // select list exprs, ie, don't call this if the query
  // doesn't have a FROM clause, this function will not return any result rows for
  // that case.
  // Sets 'batch' to NULL if no more data. Batch is owned by InProcessQueryExecutor
  // and must not be deallocated.
  virtual Status FetchResult(RowBatch** batch);

  // Return single row as comma-separated list of values.
  // Indicates end-of-stream by setting 'row' to the empty string.
  // Returns OK if successful, otherwise error.
  virtual Status FetchResult(std::string* row);

  // Return single row as vector of raw values.
  // Indicates end-of-stream by returning empty 'row'.
  // Returns OK if successful, otherwise error.
  virtual Status FetchResult(std::vector<void*>* row);

  RuntimeState* runtime_state();
  const RowDescriptor& row_desc() const;

  // Returns the error log lines in executor_'s runtime state as a string joined with '\n'.
  virtual std::string ErrorString() const;

  // Returns a string representation of the file_errors_.
  virtual std::string FileErrors() const;

  // Returns the counters for the entire query
  virtual RuntimeProfile* query_profile();

  // Returns query request
  const TQueryExecRequest& query_request() const { return query_request_; }

  // Returns select list expr
  std::vector<Expr*>& select_list_exprs() { return select_list_exprs_; }

  // Disable Jitting.  This is only used by tests to exercise the non-jitted behavior
  void DisableJit();

  virtual bool eos() { return eos_; }

  // Returns any statistics gathered by the coordinator associated with the execution of
  // this query
  virtual ExecStats* exec_stats() { return exec_stats_.get(); }

 private:
  // plan service-related
  boost::shared_ptr<apache::thrift::transport::TTransport> socket_;
  boost::shared_ptr<apache::thrift::transport::TTransport> transport_;
  boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol_;
  boost::scoped_ptr<ImpalaPlanServiceClient> client_;
  bool started_server_;

  TQueryExecRequest query_request_;
  boost::scoped_ptr<Coordinator> coord_;
  boost::scoped_ptr<RuntimeState> local_state_;  // only for queries w/o FROM clause
  boost::scoped_ptr<RuntimeProfile> query_profile_;
  ExecEnv* exec_env_;
  std::vector<Expr*> select_list_exprs_;
  RowBatch* row_batch_;
  int next_row_;  // to return from row batch
  int num_rows_;  // total # of rows returned for current query
  bool eos_;  // if true, no more rows/batches for current query
  boost::scoped_ptr<ObjectPool> obj_pool_;

  boost::scoped_ptr<ExecStats> exec_stats_;

  // Prepare select list expressions of coord fragment.
  Status PrepareSelectListExprs(
      RuntimeState* state, const RowDescriptor& row_desc,
      std::vector<PrimitiveType>* col_types);

  // End execution of the currently-running query.
  Status EndQuery();

  // Stop plan service, if we started one.
  void Shutdown();
};

}

#endif
