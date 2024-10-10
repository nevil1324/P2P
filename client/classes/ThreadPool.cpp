#include "../headers.h"

/**
* @brief Constructs the ThreadPool and starts a specified number of worker threads.
* @param numThreads The number of worker threads to create.
*/
ThreadPool::ThreadPool(size_t numThreads) : m_stop(false), m_activeTasks(0) {
    for (size_t i = 0; i < numThreads; ++i) {
        m_workers.emplace_back([this] {
            while (true) {
                function<void()> task;

                {
                    unique_lock<mutex> lock(this->m_queueMutex);
                    this->m_condition.wait(lock, [this] {
                        return this->m_stop || !this->m_tasks.empty();
                    });

                    if (this->m_stop && this->m_tasks.empty()) return;

                    task = move(this->m_tasks.front());
                    this->m_tasks.pop();
                    m_activeTasks++;
                }

                try {
                    // Execute the task
                    task();
                } catch(const string& e) {
                    generalLogger.log("ERROR", "THREAD POOL ERROR!! Error: " + e);
                } catch (...) {
                    generalLogger.log("ERROR", "THREAD POOL ERROR!! Unknown exception occurred.");
                }

                m_activeTasks--;

                // Notify that a task has completed
                m_waitCondition.notify_one();
            }
        });
    }
}

/**
* @brief Destroys the ThreadPool by stopping all worker threads and joining them.
*/
ThreadPool::~ThreadPool() {
    {
        unique_lock<mutex> lock(m_queueMutex);
        m_stop = true;
    }
    m_condition.notify_all();
    for (thread &worker : m_workers) worker.join();
}

/**
* @brief Enqueues a task for execution by the thread pool.
* @param task The task to be executed. It is a callable object (function, lambda, etc.).
* @throws runtime_error If the thread pool is stopped and no more tasks can be enqueued.
*/
void ThreadPool::enqueueTask(function<void()> task) {
    {
        unique_lock<mutex> lock(m_queueMutex);
        if (m_stop) throw runtime_error("enqueue on stopped ThreadPool");
        m_tasks.emplace(move(task));
    }
    m_condition.notify_one();
}

/**
* @brief Blocks until all enqueued tasks have been completed.
*/
void ThreadPool::wait() {
    unique_lock<mutex> lock(m_queueMutex);
    m_waitCondition.wait(lock, [this] { return m_tasks.empty() && m_activeTasks == 0; });
}
