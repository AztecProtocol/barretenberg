#pragma once
#include <cstdio>
#include <string>
#include <sys/resource.h>
#include <sys/time.h>
#include <ctime>

class Timer {
  private:
    struct timespec _startTime;
    struct timespec _endTime;

    static constexpr int64_t NanosecondsPerSecond = 1000LL * 1000 * 1000;

  public:
    Timer()
        : _endTime({})
    {
        start();
    }

    void start() { clock_gettime(CLOCK_REALTIME, &_startTime); }

    void end() { clock_gettime(CLOCK_REALTIME, &_endTime); }

    [[nodiscard]] int64_t nanoseconds() const
    {
        struct timespec end;
        if (_endTime.tv_nsec == 0 && _endTime.tv_sec == 0) {
            clock_gettime(CLOCK_REALTIME, &end);
        } else {
            end = _endTime;
        }

        int64_t nanos = (end.tv_sec - _startTime.tv_sec) * NanosecondsPerSecond;
        nanos += (end.tv_nsec - _startTime.tv_nsec);

        return nanos;
    }

    [[nodiscard]] double seconds() const
    {
        int64_t nanos = nanoseconds();
        double secs = static_cast<double>(nanos) / NanosecondsPerSecond;
        return secs;
    }

    [[nodiscard]] std::string toString() const
    {
        double secs = seconds();
        return std::to_string(secs);
    }
};