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

// Shortened up the name from "StackOverflowSimpleComparison"
BASELINE(DemoSimple, Baseline, 10, 1000000)
{
    sin(3.14159265);
}

std::random_device RandomDevice;
std::uniform_int_distribution<int> UniformDistribution(0, 1024);


BENCHMARK(DemoSimple, SinRandomw, 10, 1000000)
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

#else
#define CLOCK_FUNCTIONS(DECL) \
    DECL(getTimeOfDay, getTimeOfDay) 

#endif

#define BENCH_CLOCK_DECL(name, func) \
    BENCHMARK(DemoSimple, name, 10, 1000000) { \
        func(); \
    } 

CLOCK_FUNCTIONS(BENCH_CLOCK_DECL);

// More platform specific functions

#ifdef __linux__

#define POSIX_CLOCK_FUNCTIONS(DECL) \
    DECL(Realtime, CLOCK_REALTIME)  \
    DECL(Realtime2, CLOCK_REALTIME_COARSE) \
    DECL(Realtime3, CLOCK_REALTIME_COARSE2) 

POSIX_CLOCK_FUNCTIONS(BENCH_CLOCK_DECL);

#define BENCH_CLOCK_DECL(name, clock) \
    BENCHMARK(DemoSimple, name, 10, 1000000) { \
       timespec tp; \    
        clock_gettime(clock, &tp); \
    } 

#define CLOCK_REALTIME			0
#define CLOCK_MONOTONIC			1
#define CLOCK_PROCESS_CPUTIME_ID	2
#define CLOCK_THREAD_CPUTIME_ID		3
#define CLOCK_MONOTONIC_RAW		4
#define CLOCK_REALTIME_COARSE		5
#define CLOCK_MONOTONIC_COARSE		6
#define CLOCK_BOOTTIME			7
#define CLOCK_REALTIME_ALARM		8
#define CLOCK_BOOTTIME_ALARM		9
#define CLOCK_SGI_CYCLE			10	/* Hardware specific */
#define CLOCK_TAI			11

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
