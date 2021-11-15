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
			elapsed_time = clock_end - clock_start;
		}
		void Clear()
		{
			elapsed_time = timer::duration::zero();
		}
		void Pause()
		{
			clock_end = timer::now();
			elapsed_time += clock_end - clock_start;
		}
		template<class units> long int ElapsedTime()
		{			
			return static_cast<long int>(std::chrono::duration_cast<units>(elapsed_time).count());
		}
		float ElapsedSeconds()
		{
			return float(ElapsedTime<nanosec>()) / 1e9f;
		}
		float ElapsedMilliseconds()
		{
			return float(ElapsedTime<nanosec>()) / 1e6f;
		}

	protected:

		timer::time_point clock_start;
		timer::time_point clock_end;
		timer::duration	elapsed_time;
	};

	// Returns random integer in interval <min, max>
	inline int RandomInt(int min, int max)
	{
		max = max + 1;
		return std::rand() % (max - min) + min;
	}
	inline float RandomFloat_0_1()
	{
		return (float)std::rand() / (float)RAND_MAX;
	}
	inline float RandomFloat(float min, float max)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(min, max);
		return dis(gen);
	}
}