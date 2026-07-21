#include <chrono>

class timer {
    std::chrono::high_resolution_clock::time_point start;

public:
    timer() {
        start = std::chrono::high_resolution_clock::now();
    }

    double elapsed_us() const {
        auto end = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    double elapsed_ms() const {
        auto end = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }

    double elapsed_ns() const {
        auto end = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }

    double elapsed_s() const {
        auto end = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    }

    double elapsed_min() const {
        auto end = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::minutes>(end - start).count();
    }
};
