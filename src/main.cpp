#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;
using namespace std;

// ---------------- ThreadPool ----------------
class ThreadPool {
	vector<thread> workers;
	queue<function<void()>> tasks;
	mutex queue_mutex;
	condition_variable cv;
	bool stop = false;

public:
	ThreadPool(size_t threads = thread::hardware_concurrency()) {
		for (size_t i = 0; i < threads; ++i) {
			workers.emplace_back([this] {
				while (true) {
					function<void()> task;
					{
						unique_lock<mutex> lock(queue_mutex);
						cv.wait(lock, [this] { return stop || !tasks.empty(); });
						if (stop && tasks.empty()) return;
						task = move(tasks.front());
						tasks.pop();
					}
					task();
				}
				});
		}
	}

	void enqueue(function<void()> f) {
		{
			unique_lock<mutex> lock(queue_mutex);
			tasks.push(move(f));
		}
		cv.notify_one();
	}

	~ThreadPool() {
		{
			unique_lock<mutex> lock(queue_mutex);
			stop = true;
		}
		cv.notify_all();
		for (auto& w : workers) w.join();
	}
};

// ---------------- ECS DAG ----------------
class System {
public:
	string name;
	function<void()> update;
	System(string n, function<void()> f) : name(move(n)), update(move(f)) {}
};

class TaskGraph {
	unordered_map<string, unique_ptr<System>> systems;
	unordered_map<string, vector<string>> adj;
	unordered_map<string, int> indeg;

public:
	void add_system(const string& name, function<void()> func) {
		systems[name] = make_unique<System>(name, func);
		if (!indeg.count(name)) indeg[name] = 0;
	}

	void add_dependency(const string& before, const string& after) {
		adj[before].push_back(after);
		indeg[after]++;
	}

	void execute_parallel() {
		ThreadPool pool;
		unordered_map<string, int> indeg_copy = indeg;
		mutex mtx;
		condition_variable cv;
		size_t remaining = systems.size();

		function<void(string)> run_task = [&](string sysName) {
			cout << "[Run] " << sysName << "\n";
			systems[sysName]->update();

			{
				lock_guard<mutex> lk(mtx);
				for (auto& v : adj[sysName]) {
					if (--indeg_copy[v] == 0) {
						pool.enqueue([&, v] { run_task(v); });
					}
				}
				remaining--;
			}
			cv.notify_all();
			};

		for (auto& [name, deg] : indeg_copy) {
			if (deg == 0) {
				pool.enqueue([&, name] { run_task(name); });
			}
		}

		unique_lock<mutex> lk(mtx);
		cv.wait(lk, [&] { return remaining == 0; });
	}
};

int main() {
	TaskGraph graph;

	graph.add_system("Physics", [] {
		this_thread::sleep_for(200ms);
		cout << "  Physics update\n";
		});
	graph.add_system("AI", [] {
		this_thread::sleep_for(100ms);
		cout << "  AI update\n";
		});
	graph.add_system("Animation", [] {
		cout << "  Animation update\n";
		});
	graph.add_system("Render", [] {
		cout << "  Render update\n";
		});
	graph.add_system("Audio", [] {
		cout << "  Audio update\n";
		});

	graph.add_dependency("Physics", "AI");
	graph.add_dependency("AI", "Animation");
	graph.add_dependency("Animation", "Render");

	graph.execute_parallel();
}