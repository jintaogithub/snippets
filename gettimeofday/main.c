#include <stdio.h>
#include <sys/time.h>
#include <time.h>

int main() {
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  printf("Seconds since epoch (UTC): %ld\n", tv.tv_sec);
  printf("Microseconds: %ld\n", tv.tv_usec);

  // Convert time_t to broken-down time in UTC
  struct tm* utc_tm = gmtime(&(tv.tv_sec));
  if (utc_tm == NULL) {
    perror("gmtime");
    return 1;
  }

  printf("UTC Time: %s", asctime(utc_tm));

  // Convert to local time using localtime()
  time_t now = tv.tv_sec;
  struct tm* local_tm = localtime(&now);
  if (local_tm == NULL) {
    perror("localtime");
    return 1;
  }

  printf("Local Time: %s", asctime(local_tm));

  return 0;
}