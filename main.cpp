#include <ostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <ctime>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>

void* worker (void * runtime) {
  unsigned long counter = 0;
  time_t start = time(nullptr);
  time_t current = time(nullptr);
  while (difftime(current, start) < *(long *) runtime) {
    counter++;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    time(&current);
  }
  return NULL;
}

int main(int argc, char* argv[]) {
  if (argc == 1 || argc > 2) {
    std::cerr << "Correct syntax: benign [time in seconds]" << std::endl;
    exit(-1);
  }

  char* pEnd;
  long runtime = strtol(argv[1], &pEnd, 10);

  std::cout << "Starting a run for " << runtime << " seconds" << std::endl;

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);

  if (sched_getaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
    perror("sched_getaffinity");
    exit(-1);
  }

  int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

  // Get active cores by reading through the mask
  std::vector<int> active_cores;
  for (size_t i = 0; i < num_cores; i++) {
    if (CPU_ISSET(i, &cpuset)) {
      active_cores.push_back(i);
    }
  }

  std::vector<pthread_t> thread_ids(active_cores.size());

  for (size_t i = 0; i < active_cores.size(); i++) {
    pthread_t worker_thread;
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(active_cores[i], &mask);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &mask);
    pthread_create(&thread_ids[i], &attr, worker, &runtime);
    pthread_attr_destroy(&attr);
  }

  for (auto& thread : thread_ids) {
    pthread_join(thread, nullptr);
  }

  std::cout << "Run complete!" << std::endl;
}
