#include <vector>
#include <thread>
#include <numeric>

using Iterator = std::vector<unsigned long long>::iterator;

void accumulate_block(Iterator start, Iterator end, unsigned long long &result) {
    result = std::accumulate(start, end, 0ULL);
}

unsigned long long accumulate_parallel(Iterator start, Iterator end, unsigned long long init) {
    size_t length = std::distance(start, end);
    size_t min_per_thread = 50;
    size_t max_threads = (length+min_per_thread-1)/min_per_thread;
    size_t hardware_threads = std::thread::hardware_concurrency();
    size_t thread_num = hardware_threads!=0?std::min(hardware_threads, max_threads):2;
    size_t block_size = length/thread_num;
    std::vector<unsigned long long> results(thread_num);
    std::vector<std::thread> threads(thread_num-1);
    Iterator block_start = start, block_end = start;
    for(size_t i = 0; i<thread_num-1; i++) {
        std::advance(block_end, block_size);
        threads[i] = std::thread(accumulate_block, block_start, block_end, std::ref(results[i]));
        block_start = block_end;
    }
    accumulate_block(block_start, end, results[thread_num-1]);
    for(auto &it:threads) {it.join();}
    return std::accumulate(results.begin(), results.end(), init);
}