#include "ThreadPool.h"
#include <iostream>

CThreadPool::CThreadPool() 
	: m_state(State::Stopped), m_lastId(0)
{}

CThreadPool::~CThreadPool()
{
	Stop();
}

void CThreadPool::Initialize(int numThreads, bool expand)
{
	m_maxThreads = numThreads;
	m_expand = expand;

	for (m_lastId = 0; m_lastId < m_maxThreads; m_lastId++)
		m_threads[m_lastId] = new std::thread(ProcessTask, this, m_lastId);

	m_state = State::Running;
}

void CThreadPool::QueueTask(ThreadPoolTask t)
{
	if (IsStopping())
		return;

	if (m_expand && m_counter.Get() >= m_maxThreads)
	{
		
		// check for finished thread and delete them before spawning new one
		for (int i = 0; i < m_finished.size(); i++)
		{
			int index = m_finished.at(i);
			std::cout << "[INFO] Deleting unused thread id " << index << "\n";

			m_threads[index]->join();

			delete m_threads[index];
			m_threads.erase(index);
		}

		int id = ++m_lastId;
		std::cout << "[INFO] Allocating new thread with id " << id << "\n";
		m_threads[id] = new std::thread(ProcessTask, this, id);

		std::lock_guard<std::mutex> lock(m_finishedMutex);
		m_finished.clear();
	}

	std::unique_lock<std::mutex> lock(m_mutex);
	m_queue.push(t);

	m_condVariable.notify_one();
}

void CThreadPool::Join()
{
	ThreadMap::iterator it = m_threads.begin();

	while (it != m_threads.end())
	{
		it->second->join();
	}
}

void CThreadPool::Stop()
{
	if (m_state == State::Running)
	{
		m_state = State::Stopping;
		m_condVariable.notify_all();

		ThreadMap::iterator it = m_threads.begin();

		while (it != m_threads.end())
		{
			try
			{
				it->second->join();
				delete it->second;
			}
			catch (std::runtime_error&)
			{
				throw std::runtime_error("Error stopping a thread from pool");
			}
		}

		m_state = State::Stopped;
	}
}


bool CThreadPool::Execute()
{
	ThreadPoolTask task;
	bool ret = true;

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
		m_counter.Increment();

		// when task is completed check if we exceed thread count
		// if it does, delete or end this thread to get back to normal count
		bool exceed = m_counter.Get() > m_maxThreads;
		ret = !exceed;

		task();

		m_counter.Decrement();
	}

	return ret;
}

void CThreadPool::Mark(int index)
{
	std::lock_guard<std::mutex> lock(m_finishedMutex);
	m_finished.push_back(index);
}

void CThreadPool::ProcessTask(CThreadPool* pool, int index)
{
	while (true)
	{
		if (pool->IsStopping() && pool->IsQueueEmpty())
			break;

		if (!pool->Execute())
		{
			std::cout << "[INFO] Marking a thread for deletion : " << index << "\n";
			pool->Mark(index);
			return;
		}
	}
}

