#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>

void* worker (void *) {
  return 0;
}

int main() {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);

  if (sched_getaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
    perror("sched_getaffinity");
    exit(-1);
  }

  std::vector<pthread_t> thread_ids;
  int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

  for (size_t i = 0; i < num_cores; i++) {
    pthread_t worker_thread;
    thread_ids.push_back(worker_thread);
    pthread_create(&thread_ids[i], NULL, worker, nullptr);
    pthread_setaffinity_np(worker_thread, sizeof(cpu_set_t), &cpuset);
  }
}
