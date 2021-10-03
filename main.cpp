#include <benchmark/benchmark.h>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj.h>
#include <unistd.h>

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

class EmptyPoolFixture : public benchmark::Fixture {
public:
  void SetUp(const State &state) override {
    if (state.thread_index() != 0)
      return;
    std::remove(POOL_PATH);
    pop = pool<Root>::create(POOL_PATH, LAYOUT, POOL_SIZE, 0666);
    root = pop.root();
  }

  void TearDown(const State &state) override {
    if (state.thread_index() != 0)
      return;
    pop.close();
  }
};

BENCHMARK_DEFINE_F(EmptyPoolFixture, pmemobj_alloc)(State &state) {
  auto size = state.range(0);
  auto ptr = root->data.raw_ptr();
  for (auto _ : state) {
    pmemobj_alloc(pop.handle(), ptr, size, 0, nullptr, nullptr);
  }
  state.SetItemsProcessed(state.iterations());
  state.SetBytesProcessed(state.iterations() * size);
}

BENCHMARK_REGISTER_F(EmptyPoolFixture, pmemobj_alloc)
    ->Range(8, 8 << 10)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->Threads(32)
    ->Threads(64)
    ->Threads(128);

BENCHMARK_DEFINE_F(EmptyPoolFixture, pmemobj_reserve)(State &state) {
  auto size = state.range(0);
  struct pobj_action act;
  for (auto _ : state) {
    pmemobj_reserve(pop.handle(), &act, size, 0);
  }
  state.SetItemsProcessed(state.iterations());
  state.SetBytesProcessed(state.iterations() * size);
}

BENCHMARK_REGISTER_F(EmptyPoolFixture, pmemobj_reserve)
    ->Range(8, 8 << 10)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->Threads(32)
    ->Threads(64)
    ->Threads(128);

BENCHMARK_MAIN();