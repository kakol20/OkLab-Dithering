#pragma once

#include <chrono>
#include <string>

class Log {
public:
	Log() {};
	~Log() {};

	static void Write(const std::string input);
	static void WriteOneLine(const std::string input);

	static void EndLine();
	static void StartLine();

	static void Save(const std::string save = "console.log", const bool overwrite = true);

	static void StartTime();
	static bool CheckTime(const long long milliseconds);
	static bool CheckTimeSeconds(const double seconds);

	static std::string ToString(const bool value);
	static std::string ToString(const double value, const unsigned int precision = 6);
	static std::string ToString(const int value, const unsigned int precision = 0, const char lead = '0');
	static std::string ToString(const size_t value, const unsigned int width = 0, const char lead = '0');
	static std::string ToString(const unsigned int value, const unsigned int width = 0, const char lead = '0');

	static std::string LeadingCharacter(const std::string value, const unsigned int amount = 0, const char lead = ' ');

	static void Sound(const long long duration = 0);

	/// <summary>
	/// Does not automatically close console
	/// </summary>
	static void HoldConsole();

private:
	static std::string m_console;
	static std::chrono::steady_clock::time_point m_time;
};