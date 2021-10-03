# pmbench

## Prerequisites

- Google Benchmark: [build from source](https://github.com/google/benchmark#installation) or `sudo apt install libbenchmark-dev`

- `libpmemobj`: [build from source](https://github.com/pmem/pmdk#building-pmdk-on-linux-or-freebsd) or `sudo apt install libpmem-dev libpmemobj-dev`

- `libpmemobj-cpp`: [build from source](https://github.com/pmem/libpmemobj-cpp#linux-build) or `sudo apt install libpmemobj-cpp-dev`

- Python packages: `pip install -r requirements.txt`

- Disable CPU scaling: `sudo cpupower frequency-set --governor performance`


## Usage

- Build: `make build`

- Run & Plot: `make run`
