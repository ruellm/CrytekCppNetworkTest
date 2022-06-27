#pragma once
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

using ThreadPoolTask = std::function<void()>;

class CThreadPool
{
public:
	enum class State
	{
		Stopped,
		Running,
		Stopping
	};

	CThreadPool();
	~CThreadPool();
	void QueueTask(ThreadPoolTask t);
	void Stop();

	void Initialize(int numThreads);
	
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
	std::queue<ThreadPoolTask> m_queue;
	std::vector<std::thread*> m_threads;

	void Execute();

	static void ProcessTask(CThreadPool* pool);
};