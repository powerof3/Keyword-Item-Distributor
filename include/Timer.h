#pragma once

class Timer
{
public:
	void start()
	{
		startTime = std::chrono::steady_clock::now();
	}

	void end()
	{
		endTime = std::chrono::steady_clock::now();
	}

	std::uint64_t duration_μs() const
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	}

	std::uint64_t duration_ms() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
	}

private:
	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point endTime;
};
