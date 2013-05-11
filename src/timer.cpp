#if defined(_WIN32)
// windows

#include <windows.h>

bool qpc;
static unsigned int timer_base_32;
static __int64 timer_base_64;

#elif defined(__APPLE__)
// os x / mac

#include <mach/mach_time.h>

static uint64_t timer_base;
#define get_raw_time mach_absolute_time

#else
// linux, unix, etc.
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

static uint64_t timer_base;
static bool monotonic;
static uint64_t get_raw_time(void)
{
#if defined(CLOCK_MONOTONIC)
    if (monotonic) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (uint64_t)ts.tv_sec * (uint64_t)1e9 + (uint64_t)ts.tv_nsec;
    } else
#endif
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (uint64_t)tv.tv_sec * (uint64_t)1e6 + (uint64_t)tv.tv_usec;
    }
}
#endif

static double resolution;

void init_time()
{
#if defined(_WIN32)
    __int64 freq;

    if (QueryPerformanceFrequency((LARGE_INTEGER*)&freq)) {
        qpc = true;
        resolution = 1.0 / (double)freq;
        QueryPerformanceCounter((LARGE_INTEGER*)&timer_base_64);
    } else {
        qpc = false;
        resolution = 1e-3;
        timer_base_32 = timeGetTime();
    }
#elif defined(__APPLE__)
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    resolution = (double)info.numer / (info.denom * 1e9);
    timer_base = get_raw_time();
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        monotonic = true;
        resolution = 1e-9;
    } else
        resolution = 1e-6;
    timer_base = get_raw_time();
#endif
}

double get_time()
{
#ifdef _WIN32
    double t;

    if (qpc) {
        __int64 t_64;
        QueryPerformanceCounter((LARGE_INTEGER*)&t_64);
        t = (double)(t_64 - timer_base_64);
    } else
        t = (double)(timeGetTime() - timer_base_32);
    return t * resolution;
#else
    return (get_raw_time() - timer_base) * resolution;
#endif
}
