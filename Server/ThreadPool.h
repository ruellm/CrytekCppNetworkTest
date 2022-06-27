#pragma once
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <map>

using ThreadPoolTask = std::function<void()>;
using ThreadMap = std::map<int, std::thread*>;

class CThreadPool
{
public:
	enum class State
	{
		Stopped,
		Running,
		Stopping
	};

	struct SAtomicCounter {
		std::atomic<int> value;
		
		SAtomicCounter() : value(0) 
		{
			//...
		}

		void Increment() {
			++value;
		}

		void Decrement() {
			--value;
		}

		int Get() {
			return value.load();
		}
	};

	CThreadPool();
	~CThreadPool();
	void QueueTask(ThreadPoolTask t);
	void Stop();

	void Initialize(int numThreads, bool expand);
	
	inline bool IsStopping() const {
		return m_state == State::Stopping;
	}

	inline bool IsStopped() const{
		return m_state == State::Stopped;
	}

	inline bool IsQueueEmpty() const {
		return m_queue.empty();
	}

private:
	State m_state;
	int m_maxThreads;
	std::condition_variable m_condVariable;
	std::mutex m_mutex;
	std::mutex m_threadMutex;
	std::queue<ThreadPoolTask> m_queue;
	ThreadMap m_threads;
	std::vector<int> m_finished;
	SAtomicCounter m_counter;	// counter for busy thread
	
	int m_lastId;
	bool m_expand;

	bool Execute();
	void Mark(int index);
	
	static void ProcessTask(CThreadPool* pool, int index);
};