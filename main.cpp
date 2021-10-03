#include <benchmark/benchmark.h>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj.h>
#include <unistd.h>

using benchmark::RegisterBenchmark;
using benchmark::State;
using pmem::obj::persistent_ptr;
using pmem::obj::pool;

constexpr auto LAYOUT = "hash_table";
constexpr auto POOL_PATH = "pool";
constexpr auto POOL_SIZE = 32 * PMEMOBJ_MIN_POOL;

struct Root {
  persistent_ptr<char[]> data;
};

pool<Root> pop;
persistent_ptr<Root> root;

class BenchmarkContext {
  State &state;

public:
  explicit BenchmarkContext(State &state) : state(state) {
    if (state.thread_index() == 0) {
      std::remove(POOL_PATH);
      pop = pool<Root>::create(POOL_PATH, LAYOUT, POOL_SIZE, 0666);
      root = pop.root();
    }
  }
  ~BenchmarkContext() {
    state.SetItemsProcessed(state.iterations());
    if (state.thread_index() == 0) {
      pop.close();
    }
  }
};

void BM_pmemobj_alloc(State &state) {
  BenchmarkContext context(state);
  auto size = state.range(0);
  auto ptr = root->data.raw_ptr();
  for (auto _ : state) {
    pmemobj_alloc(pop.handle(), ptr, size, 0, nullptr, nullptr);
  }
  state.SetBytesProcessed(state.iterations() * size);
}

void BM_pmemobj_reserve(State &state) {
  BenchmarkContext context(state);
  auto size = state.range(0);
  struct pobj_action act {};
  for (auto _ : state) {
    pmemobj_reserve(pop.handle(), &act, size, 0);
  }
  state.SetBytesProcessed(state.iterations() * size);
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
  state.SetBytesProcessed(state.iterations() * size);
}

int main(int argc, char **argv) {
  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;

  for (auto &benchmark : {
           RegisterBenchmark("pmemobj_alloc", BM_pmemobj_alloc),
           RegisterBenchmark("pmemobj_reserve", BM_pmemobj_reserve),
           RegisterBenchmark("pmemobj_tx_alloc", BM_pmemobj_tx_alloc),
       }) {
    benchmark->Range(8, 8 << 10)->ThreadRange(1, 128);
  }

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}