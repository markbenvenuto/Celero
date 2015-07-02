#include <random>

#include <celero/Celero.h>
#ifndef _WIN32
#include <sys/time.h>
#else
#include <Windows.h>
#endif

///
/// This is the main(int argc, char** argv) for the entire celero program.
/// You can write your own, or use this macro to insert the standard one into the project.
///
CELERO_MAIN

#define DO_SINGLE 1

#ifdef _WIN32

static unsigned long long getPerfCounter() {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}

static unsigned long long curTimeMicros64PP() {
    return getPerfCounter();
}

unsigned long long curTimeMicros64_GTC() {
    return GetTickCount64();
}

unsigned long long curTimeMicros64_QUIT() {
    ULONGLONG t;
    QueryUnbiasedInterruptTime(&t);
    return t;
}

#else

unsigned long long getTimeOfDay() {
    timeval tp;
    struct timezone tzp;
    gettimeofday(&tp, &tzp);

    return tp.tv_sec * 1000000 + tp.tv_usec;
}

#endif

#ifdef DO_SINGLE
#define SAMPLES_S 10
#define ITERATIONS_S 100000

// Shortened up the name from "StackOverflowSimpleComparison"
BASELINE(DemoSimple, Baseline, 10, ITERATIONS_S)
{
    sin(3.14159265);
}

std::random_device RandomDevice;
std::uniform_int_distribution<int> UniformDistribution(0, 1024);


BENCHMARK(DemoSimple, SinRandomw, 10, ITERATIONS_S)
{
    sin(UniformDistribution(RandomDevice));
}

// name, func
#ifdef _WIN32

#define CLOCK_FUNCTIONS(DECL) \
    DECL(getPerfCounter, curTimeMicros64PP) \
    DECL(GetCurrentTime, GetCurrentTime) \
    DECL(GetTickCount64, curTimeMicros64_GTC) \
    DECL(QueryUnbiasedInterruptTime, curTimeMicros64_QUIT) 

#elif defined(__sunos__)

unsigned long long gethrtime2() {
    hrtime_t t = getthrtime();
    return t;
}

#define CLOCK_FUNCTIONS(DECL) \
    DECL(getTimeOfDay, getTimeOfDay) \
    DECL(gethrtime,gethrtime2) 

#else
#define CLOCK_FUNCTIONS(DECL) \
    DECL(getTimeOfDay, getTimeOfDay) 

#endif

#define BENCH_CLOCK_DECL(name, func) \
    BENCHMARK(DemoSimple, name, 10, ITERATIONS_S) { \
        func(); \
    } 

CLOCK_FUNCTIONS(BENCH_CLOCK_DECL);

// More platform specific functions

#if defined(__linux__) || defined(__sunos__)

#ifdef __sunos__

#define POSIX_CLOCK_FUNCTIONS(DECL) \
    DECL(REALTIME0, __CLOCK_REALTIME0) \
    DECL(VIRTUAL, CLOCK_VIRTUAL) \
    DECL(THREAD_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID) \
    DECL(REALTIME, CLOCK_REALTIME) \
    DECL(MONOTONIC, CLOCK_MONOTONIC) \
    DECL(PROCESS_CPUTIME_ID, CLOCK_PROCESS_CPUTIME_ID) \
    DECL(HIGHRES, CLOCK_HIGHRES) \
    DECL(PROF, CLOCK_PROF)

#else

#define POSIX_CLOCK_FUNCTIONS(DECL) \
    DECL(REALTIME, CLOCK_REALTIME) \
    DECL(MONOTONIC, CLOCK_MONOTONIC) \
    DECL(PROCESS_CPUTIME_ID, CLOCK_PROCESS_CPUTIME_ID) \
    DECL(THREAD_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID) \
    DECL(MONOTONIC_RAW, CLOCK_MONOTONIC_RAW) \
    DECL(REALTIME_COARSE, CLOCK_REALTIME_COARSE) \
    DECL(MONOTONIC_COARSE, CLOCK_MONOTONIC_COARSE) \
    DECL(BOOTTIME, CLOCK_BOOTTIME) \
    DECL(REALTIME_ALARM, CLOCK_REALTIME_ALARM) \
    DECL(BOOTTIME_ALARM, CLOCK_BOOTTIME_ALARM) \
    DECL(TAI, CLOCK_TAI)

#endif

#define BENCH_CLOCK_DECL2(name, clock) \
    BENCHMARK(DemoSimple, name, 10, ITERATIONS_S) { \
       timespec tp; \
        clock_gettime(clock, &tp); \
    }

POSIX_CLOCK_FUNCTIONS(BENCH_CLOCK_DECL2);

#endif

#ifdef __APPLE_

#define APPLE_CLOCK_FUNCTIONS(DECL) \
    DECL(machAbsoluteTime, mach_absolute_time) 

APPLE_CLOCK_FUNCTIONS(BENCH_CLOCK_DECL);

#endif


#else // DO_MULTI

class ClockFixture : public celero::ThreadTestFixture {
public:
    int counter;

    void setUp(int64_t) override {
        this->counter = 0;
    }
};

#define SAMPLES_T 10
#define ITERATIONS_T 1000000
#define THREADS_T 12

BASELINE_T(DemoMultithread, Baseline, ClockFixture, SAMPLES_T, ITERATIONS_T, THREADS_T) {
    ++counter;
}

#ifdef _WIN32
BENCHMARK_T(DemoMultithread, getPerfCounter, ClockFixture, SAMPLES_T, ITERATIONS_T, THREADS_T) {
    curTimeMicros64PP();
}

BENCHMARK_T(DemoMultithread, GetCurrentTime, ClockFixture, SAMPLES_T, ITERATIONS_T, THREADS_T) {
    GetCurrentTime();
}

BENCHMARK_T(DemoMultithread, GetTickCount64, ClockFixture, SAMPLES_T, ITERATIONS_T, THREADS_T) {
    curTimeMicros64_GTC();
}

BENCHMARK_T(DemoMultithread, QueryUnbiasedInterruptTime, ClockFixture, SAMPLES_T, ITERATIONS_T, THREADS_T) {
    curTimeMicros64_QUIT();
}
#else
BENCHMARK_T(DemoMultithread, GetTimeOfDay, ClockFixture, SAMPLES_T, ITERATIONS_T, THREADS_T) {
    getTimeOfDay();
}
#endif

#endif // DO_MULTI
