#include <pthread.h>
#include <unistd.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

std::atomic<uint64_t> sum(0);
std::mutex mtx;
uint64_t relax_sum = 0;
int nproc;

uint64_t* avg_per_iteration_time_nsec;

uint64_t now_timeticks_usec() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

// Function that each thread will execute
void atomic_thread_func(int threadId, uint64_t iterations, int core_id) {
  // --- Set CPU Affinity ---
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);                          // Clear the CPU set
  CPU_SET(core_id, &cpuset);                  // Add the desired core to the set
  pthread_t current_thread = pthread_self();  // Get current thread ID
  int result =
      pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
  if (result != 0) {
    std::cerr << "Error setting thread affinity for thread " << threadId
              << " to core " << core_id << ": errno = " << result << "\n";
    return;
  }
  std::cerr << "Thread " << threadId << " running on core " << core_id << "\n";

  uint64_t start = now_timeticks_usec();
  for (uint64_t i = 0; i < iterations; ++i) {
    sum += 1;
  }
  uint64_t end = now_timeticks_usec();

  avg_per_iteration_time_nsec[threadId] = (end - start) * 1e3 / iterations;
}

void mutex_thread_func(int threadId, uint64_t iterations, int core_id) {
  // --- Set CPU Affinity ---
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);                          // Clear the CPU set
  CPU_SET(core_id, &cpuset);                  // Add the desired core to the set
  pthread_t current_thread = pthread_self();  // Get current thread ID
  int result =
      pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
  if (result != 0) {
    std::cerr << "Error setting thread affinity for thread " << threadId
              << " to core " << core_id << ": errno = " << result << "\n";
    return;
  }
  std::cerr << "Thread " << threadId << " running on core " << core_id << "\n";

  uint64_t local_sum = 0;

  uint64_t start = now_timeticks_usec();
  for (uint64_t i = 0; i < iterations; ++i) {
    local_sum += 1;
  }
  uint64_t end = now_timeticks_usec();

  avg_per_iteration_time_nsec[threadId] = (end - start) * 1e3 / iterations;

  std::lock_guard<std::mutex> l(mtx);
  sum += local_sum;
}

void naive_mutex_thread_func(int threadId, uint64_t iterations, int core_id) {
  // --- Set CPU Affinity ---
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);                          // Clear the CPU set
  CPU_SET(core_id, &cpuset);                  // Add the desired core to the set
  pthread_t current_thread = pthread_self();  // Get current thread ID
  int result =
      pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
  if (result != 0) {
    std::cerr << "Error setting thread affinity for thread " << threadId
              << " to core " << core_id << ": errno = " << result << "\n";
    return;
  }
  std::cerr << "Thread " << threadId << " running on core " << core_id << "\n";

  uint64_t start = now_timeticks_usec();
  for (uint64_t i = 0; i < iterations; ++i) {
    std::lock_guard<std::mutex> l(mtx);
    relax_sum += 1;
  }
  uint64_t end = now_timeticks_usec();

  avg_per_iteration_time_nsec[threadId] = (end - start) * 1e3 / iterations;
}

void usage(const char* cmd) {
  std::cerr << "-- Synchronized Sum --\n\n";
  std::cerr << "Spawns N threads, each of which conducts M iterations of\n"
               "add-by-1. The result is aggregated across all threads. The\n"
               "summation results are somehow synchronized across threads and\n"
               "the result is guaranteed to be M * N.\n\n";
  std::cerr
      << "This is a simple program to demonstrate different synchronization\n"
         "mechenisms and their implications on the runtime performance. An\n"
         "interesting toy code to help understand atomic operations, mutex,\n"
         "cache line contention, etc.\n";
  std::cerr << "ref: https://www.youtube.com/watch?si=MP24hf_E0pi1elIC\n\n";
  std::cerr << "Usage: " << cmd << " <iterations> <number_of_threads> <mode>\n";
  std::cerr << "mode:\n";
  std::cerr << " 0 - mutex: all threads sum with local variable, and use\n";
  std::cerr << "            mutex lock once for final aggregation.\n";
  std::cerr << " 1 - atomic: all threads share a global atomic var for sum\n";
  std::cerr << " 2 - naive mutex: all threads share a global mutex to access\n";
  std::cerr << "                  a non-atomic global sum\n";
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    usage(argv[0]);
    return 1;
  }

  uint64_t iterations = std::stoull(argv[1]);
  int num_threads = std::stoi(argv[2]);
  int mode = std::stoi(argv[3]);

  std::cerr << "Running "
            << (mode == 0 ? "mutex" : (mode == 1 ? "atomic" : "naive_mutex"))
            << " mode: threads - " << num_threads << ", iterations - "
            << iterations << std::endl;

  // Get the number of available processors
  int nproc = sysconf(_SC_NPROCESSORS_ONLN);
  std::cerr << "Number of available processors: " << nproc << std::endl;

  std::vector<std::thread> threads;
  threads.reserve(num_threads);

  avg_per_iteration_time_nsec =
      (uint64_t*)calloc(num_threads, sizeof(uint64_t));

  // Create and start the threads
  for (int i = 0; i < num_threads; ++i) {
    int core_id = i % nproc;
    // int core_id = (i + nproc - 1) % nproc;
    if (mode == 0) {
      threads.emplace_back(mutex_thread_func, i, iterations, core_id);
    } else if (mode == 1) {
      threads.emplace_back(atomic_thread_func, i, iterations, core_id);
    } else {
      threads.emplace_back(naive_mutex_thread_func, i, iterations, core_id);
    }
  }

  // Wait for all threads to finish
  for (auto& thread : threads) {
    thread.join();
  }

  std::cerr << "All threads finished.\n";

  std::cerr << "sum = " << sum << ", relax_sum = " << relax_sum << std::endl;

  uint64_t min_time = 0xFFFFFFFF, max_time = 0, avg_time = 0;

  for (int i = 0; i < num_threads; ++i) {
    if (min_time > avg_per_iteration_time_nsec[i]) {
      min_time = avg_per_iteration_time_nsec[i];
    }

    if (max_time < avg_per_iteration_time_nsec[i]) {
      max_time = avg_per_iteration_time_nsec[i];
    }

    avg_time += avg_per_iteration_time_nsec[i];
  }

  std::cerr << "avg per iteration time (nsec): min " << min_time << ", max "
            << max_time << ", avg " << avg_time / num_threads << std::endl;

  free(avg_per_iteration_time_nsec);

  return 0;
}