#include "ThreadPool.h"

CThreadPool::CThreadPool() : m_state(State::Stopped)
{}

CThreadPool::~CThreadPool()
{
	Stop();
}

void CThreadPool::Initialize(int numThreads)
{
	m_maxThreads = numThreads;
	m_threads.reserve(m_maxThreads);

	for (int i = 0; i < m_maxThreads; i++)
		m_threads.push_back(new std::thread(ProcessTask, this));

	m_state = State::Running;
}

void CThreadPool::QueueTask(ThreadPoolTask t)
{
	if (IsStopping())
		return;

	//TODO: expand new thread when pool run out

	std::unique_lock<std::mutex> lock(m_mutex);
	m_queue.push(t);

	m_condVariable.notify_one();
}

void CThreadPool::Stop()
{
	if (m_state == State::Running)
	{
		m_state = State::Stopping;
		m_condVariable.notify_all();

		for (auto t : m_threads)
		{
			try
			{
				t->join();
				delete t;
			}
			catch (std::runtime_error&)
			{
				throw std::runtime_error("Error stopping a thread from pool");
			}
		}

		m_state = State::Stopped;
	}
}

void CThreadPool::Execute()
{
	ThreadPoolTask task;

	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_condVariable.wait(lock, [this] {
			return !m_queue.empty() || IsStopping();
		});

		if (!m_queue.empty())
		{
			task = m_queue.front();
			m_queue.pop();
		}
	}

	if (task)
	{
		//TODO: increment and decrement
		task();
	}
}

void CThreadPool::ProcessTask(CThreadPool* pool)
{
	while (true)
	{
		if (pool->IsStopping() && pool->IsQueueEmpty())
			break;

		pool->Execute();
	}
}