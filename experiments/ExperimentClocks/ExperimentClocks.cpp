#include <random>

#include <celero/Celero.h>
#include <sys/time.h>


///
/// This is the main(int argc, char** argv) for the entire celero program.
/// You can write your own, or use this macro to insert the standard one into the project.
///
CELERO_MAIN

// Shortened up the name from "StackOverflowSimpleComparison"
BASELINE(DemoSimple, Baseline, 10, 1000000)
{
    celero::DoNotOptimizeAway(static_cast<float>(sin(3.14159265)));
}

unsigned long long getTimeOfDay() {
     timeval tp;
     struct timezone tzp;
     gettimeofday(&tp, &tzp);

     return tp.tv_sec * 1000000 + tp.tv_usec;
}

std::random_device RandomDevice;
std::uniform_int_distribution<int> UniformDistribution(0, 1024);

BENCHMARK(DemoSimple, SinRandomw, 10, 1000000)
{
    celero::DoNotOptimizeAway(static_cast<float>(sin(UniformDistribution(RandomDevice))));
}

//BENCHMARK_T(DemoSimple, GetTimeOfDay2, 10, 1000000, 5)
//{
    //celero::DoNotOptimizeAway(getTimeOfDay());
//}
BENCHMARK(DemoSimple, GetTimeOfDay, 10, 1000000)
{
    celero::DoNotOptimizeAway(getTimeOfDay());
}
