#pragma once
#include <chrono>
#include <random>

namespace Helper
{
	using timer = std::chrono::system_clock;
	using millisec = std::chrono::milliseconds;
	using seconds = std::chrono::seconds;
	using microsec = std::chrono::microseconds;
	using nanosec = std::chrono::nanoseconds;

	class Stopwatch
	{
	public:
		Stopwatch() : clock_start(), clock_end(), elapsed_time(timer::duration::zero()) {}

		void Start()
		{
			clock_start = timer::now();
		}
		void Restart()
		{
			elapsed_time = timer::duration::zero();
			Start();
		}
		void Stop()
		{
			clock_end = timer::now();
		}
		template<class units>
		long int ElapsedTime()
		{
			elapsed_time += clock_end - clock_start;
			return static_cast<long int>(std::chrono::duration_cast<units>(elapsed_time).count());
		}

	protected:

		timer::time_point clock_start;
		timer::time_point clock_end;
		timer::duration	elapsed_time;

	};

	int RandomInteger(int min, int max)
	{
		return std::rand() % (max - min) + min;
	}
	float RandomFloat_0_1()
	{
		return (float)std::rand() / (float)RAND_MAX;
	}
	float RandomFloat(float min, float max)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(min, max);
		return dis(gen);
	}
}