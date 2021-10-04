#include <benchmark/benchmark.h>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/make_persistent_atomic.hpp>
#include <libpmemobj++/transaction.hpp>
#include <libpmemobj.h>
#include <unistd.h>

using benchmark::RegisterBenchmark;
using benchmark::State;

constexpr auto LAYOUT = "hash_table";
constexpr auto POOL_PATH = "pool";
constexpr auto POOL_SIZE = 64 * PMEMOBJ_MIN_POOL;

struct Root {
  pmem::obj::persistent_ptr<char[]> ptr;
};

pmem::obj::pool<Root> pop;
pmem::obj::persistent_ptr<Root> root;

class BenchmarkContext {
  State &state;

public:
  explicit BenchmarkContext(State &state) : state(state) {
    if (state.thread_index() == 0) {
      std::remove(POOL_PATH);
      pop = pmem::obj::pool<Root>::create(POOL_PATH, LAYOUT, POOL_SIZE, 0666);
      root = pop.root();
    }
  }
  ~BenchmarkContext() {
    state.SetItemsProcessed(state.iterations());
    if (state.range(0) != 0) {
      state.SetBytesProcessed(state.iterations() * state.range(0));
    }
    if (state.thread_index() == 0) {
      pop.close();
    }
  }
};

void BM_pmemobj_alloc(State &state) {
  BenchmarkContext context(state);
  auto size = state.range(0);
  auto ptr = root->ptr.raw_ptr();
  for (auto _ : state) {
    pmemobj_alloc(pop.handle(), ptr, size, 0, nullptr, nullptr);
  }
}

void BM_pmemobj_reserve(State &state) {
  BenchmarkContext context(state);
  auto size = state.range(0);
  struct pobj_action act {};
  for (auto _ : state) {
    pmemobj_reserve(pop.handle(), &act, size, 0);
  }
}

void BM_pmemobj_tx_alloc(State &state) {
  BenchmarkContext context(state);
  auto size = state.range(0);
  for (auto _ : state) {
    // clang-format off
        TX_BEGIN(pop.handle()) {
          pmemobj_tx_alloc(size, 0);
        } TX_END
    // clang-format on
  }
}

void BM_make_persistent(State &state) {
  BenchmarkContext context(state);
  auto size = state.range(0);
  for (auto _ : state) {
    pmem::obj::transaction::run(
        pop, [&]() { pmem::obj::make_persistent<char[]>(size); });
  }
}

void BM_make_persistent_atomic(State &state) {
  BenchmarkContext context(state);
  auto size = state.range(0);
  for (auto _ : state) {
    pmem::obj::make_persistent_atomic<char[]>(pop, root->ptr, size);
  }
}

void BM_transaction(State &state) {
  BenchmarkContext context(state);
  for (auto _ : state) {
    pmem::obj::transaction::run(pop, []() {});
  }
}

int main(int argc, char **argv) {
  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;

  auto alloc_benchmarks = {
      RegisterBenchmark("pmemobj_alloc", BM_pmemobj_alloc),
      RegisterBenchmark("pmemobj_reserve", BM_pmemobj_reserve),
      RegisterBenchmark("pmemobj_tx_alloc", BM_pmemobj_tx_alloc),
      RegisterBenchmark("make_persistent", BM_make_persistent),
      RegisterBenchmark("make_persistent_atomic", BM_make_persistent_atomic),
  };

  for (auto &benchmark : alloc_benchmarks) {
    benchmark->Range(8, 8 << 10)->ThreadRange(1, 128);
  }

  RegisterBenchmark("transaction", BM_transaction)
      ->Range(0, 0)
      ->ThreadRange(1, 128);

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}