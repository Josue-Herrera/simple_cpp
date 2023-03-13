#include "timer.hpp"
#include "task_queue.hpp"

void task_test () {
	using namespace std::literals::chrono_literals;

	auto total      = 1000;
	auto sleep_time = 3us;
	auto k_bound    = 48u;
	auto n_async    = 5;
	auto chunk_size = total / n_async; 

	{ timer t("single threaded");
		for (int i = 0; i < total; ++i) {
			printf("[task #%d]\n", i);
			std::this_thread::sleep_for(sleep_time);
		}
	}
	
	
	task_system ts{};
	{ timer t2("async function");

		for (int i = 0; i < n_async; ++i) {

			ts.async([=, start = i * chunk_size]{
				auto snum = std::to_string(i);
				timer t3(snum.c_str());
				for (int j = 0; j < chunk_size; ++j) {
					printf("[task #%d]\n", j + start);
					std::this_thread::sleep_for(sleep_time);
				}
			});
		}
	}
}

