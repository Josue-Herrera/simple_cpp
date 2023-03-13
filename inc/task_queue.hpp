
/// *** Task Stealing Queue C++11 ***
#include <atomic>
#include <deque>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

/// *** Common type vocabulary *** //
using function_signiture_t = void();
using function_capture_t   = std::function<function_signiture_t>;

/// @brief  Notification Queue trys to add a task
/// into its deque of for tasks.
class notification_queue {
	// *** notification_queue type vocabulary *** //
	using queue_t    = std::deque<function_capture_t>;
	using mutex_t    = std::mutex;
	using lock_t     = std::unique_lock<mutex_t>;
	using cond_var_t = std::condition_variable;
	
	cond_var_t      ready;
	queue_t         queue;
	mutex_t         mutex;
	bool            finished{ false };

public: 

	/// @brief  Attempt to pop something but if the queue 
	/// is empty or if its busy it will return false.
	///
	/// @param func std::function holding work to be ran.
	/// @return true if it was able to pop the front of the queue.
	/// false if it failed to pop the function.
	bool try_pop(function_capture_t& func) noexcept {
		lock_t lock{ mutex, std::try_to_lock };

		if (not lock or queue.empty()) {
			return false;
		}

		func = std::move(queue.front());
		queue.pop_front();
		return true;
	}

	/// @brief 
	/// @param func 
	/// @return 
	bool pop(function_capture_t& func) noexcept {
		lock_t lock { mutex };

		while (queue.empty() and not finished){
			ready.wait(lock);
		} 

		if (queue.empty()) {
			return false;
		}

		func = std::move(queue.front());
		queue.pop_front();
		return true;
	}

	
	template<class Function>
	bool try_push(Function && func) noexcept {
		{ 
			lock_t lock{ mutex, std::try_to_lock };

			if (not lock) {
				return false;
			}

			queue.emplace_back(std::forward<Function>(func));
		}

		ready.notify_one();
		return true;
	}

	template<class Function>
	void push(Function&& func) noexcept {
		lock_t lock{ mutex };
		queue.emplace_back(std::forward<Function>(func));
	}

	void done() noexcept {
		{
			lock_t lock{ mutex };
			finished = true;
		}
		ready.notify_all();
	}
};


class task_system {
	// *** task_system type vocabulary *** //
	using thread_container_t   = std::vector<std::thread>;
	using notifications_t      = std::vector<notification_queue>;
	using atomic_index_t       = std::atomic<unsigned>;

	const unsigned	    count{std::thread::hardware_concurrency()};
	const unsigned      k_bound {48};
	atomic_index_t	    index{0};
	notifications_t		notifications{count};
	thread_container_t  threads;

	/// @brief 
	/// @param i 
	void run(unsigned i) noexcept {

		while (true) {
			function_capture_t func;

			for (unsigned n = 0; n != count; ++n) {
				if (notifications[(i + n) % count].try_pop(func)) {
					break;
				}
			}

			if (not func and not notifications[i].pop(func)) {
                break;
            }

			func();
		}
	}

public:

	/// @brief Contructs a Task System based on a thread pool.
	///
	task_system () noexcept {
		for (unsigned n = 0; n != count; ++n) {
			threads.emplace_back([&, n] { run(n); });
		}
	}
	
	/// @brief Contructs a Task System based on a thread pool.
	///
	/// @param k is a bound on the number iterations
	/// before trying to push to a task onto a queue.
	///
	explicit task_system (unsigned k) noexcept : k_bound{ k } {
		for (unsigned n = 0; n != count; ++n) {
			threads.emplace_back([&, n] { run(n); });
		}
	}

	/// @brief The destructor destories all the threads and 
	/// notification queues.
	///
	~task_system() noexcept {
		for (auto& ns : notifications) ns.done();
		for (auto& ts : threads)       ts.join();
	}

	/// @brief This function takes as tasks and assisgnes it to 
	/// a notifaction queue that perfroms work.
	///
	/// @tparam Function Any callable type that can be used in std::function.
	/// @param work Any function that no return peretemer.
	///
	template<class Function>
	void async(Function&& work) noexcept {
		const auto i = index++;
		for (unsigned n = 0; n != count * k_bound; ++n) {
			if(notifications[(i + n) % count].try_push(std::forward<Function>(work))) {
				return;
			} 
		}
		notifications[i % count].push(std::forward<Function>(work));
	}
};
