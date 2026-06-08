#include "threadsafe_queue.hpp"
#include <future>
#include <vector>
#include <thread>
#include <functional>

class threadpool {
    threadsafe_queue<std::packaged_task<void()>> q;
    std::vector<std::thread> threads;
    std::atomic<bool> done;
    unsigned int n_threads;

    void thread_work() {
        while(!done) {
            std::packaged_task<void()> task;
            if(q.try_pop(task)) task();
            else std::this_thread::yield(); 
        }
    }

public:

    threadpool(unsigned int n) : done(false), n_threads(n) {
        for(unsigned int i = 0; i<n_threads; i++) {
            threads.push_back(
                std::thread(&threadpool::thread_work, this)
            );
        }
    }
    
    template <typename func>
    auto submit(func&& f) {
        std::packaged_task<void()> task(f);
        auto fut = task.get_future();
        q.push(std::move(task));
        return fut;
    }

    ~threadpool() {
        done = true;
        for(auto &it:threads) it.join();
    }
};