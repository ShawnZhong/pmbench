all: build run
.PHONY: build run clean

output = results/$(shell date "+%Y-%m-%d-%H-%M-%S").json

build:
	mkdir -p build
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build -j

run:
	mkdir -p results
	build/pmbench \
		--benchmark_counters_tabular=true \
		--benchmark_out=$(output) \
		--benchmark_out_format=json

	python plot.py $(output)

clean:
	rm -rf build