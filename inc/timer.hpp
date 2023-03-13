#include <chrono>
namespace chrono = std::chrono;

struct timer {
	using clock = chrono::system_clock;
	using units = chrono::nanoseconds;

	char const* const name;
	clock::time_point start;

	timer(char const* const n) :
		name{ n },
		start{ clock::now() }
	{};

	~timer() {
		const auto duration = chrono::duration_cast<units>(clock::now() - start).count();
		printf("[duration] %lld nanosecs [name] %s\n", duration, name);
	}
};