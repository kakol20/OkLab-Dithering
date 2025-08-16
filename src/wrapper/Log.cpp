#include "Log.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

std::string Log::m_console = "";
std::chrono::steady_clock::time_point Log::m_time = std::chrono::high_resolution_clock::now();

void Log::Write(const std::string input) {
	std::cout << input;
	Log::m_console += input;
}

void Log::WriteOneLine(const std::string input) {
	Log::StartLine();
	Log::Write(input);
	Log::EndLine();
}

void Log::EndLine() {
	std::cout << '\n';
	Log::m_console += "\n";
}

void Log::StartLine() {
	std::time_t now = std::time(0);

	std::tm tmnow{};

	localtime_s(&tmnow, &now);
	typedef std::chrono::system_clock Clock;
	auto c_now = Clock::now();

	std::string month = std::to_string(tmnow.tm_mon + 1);
	month = month.size() == 1 ? "0" + month : month;

	std::string day = std::to_string(tmnow.tm_mday);
	day = day.size() == 1 ? "0" + day : day;

	std::string hour = std::to_string(tmnow.tm_hour);
	hour = hour.size() == 1 ? "0" + hour : hour;

	std::string min = std::to_string(tmnow.tm_min);
	min = min.size() == 1 ? "0" + min : min;

	// combine seconds and milliseconds
	std::string sec = std::to_string(tmnow.tm_sec);
	sec = sec.size() == 1 ? "0" + sec : sec;

	// get milliseconds
	const auto c_seconds = std::chrono::time_point_cast<std::chrono::seconds>(c_now);
	const auto c_fraction = c_now - c_seconds;
	const time_t c_cnow = Clock::to_time_t(c_now);
	const auto c_millisec = std::chrono::duration_cast<std::chrono::milliseconds>(c_fraction);
	const int c_millCount = int(c_millisec.count());

	std::string mil = std::to_string(c_millCount);
	while (mil.size() < 3) {
		mil = "0" + mil;
	}

	std::string line = std::to_string(tmnow.tm_year + 1900) + "-" + month + "-" + day + " "
		+ hour + ":" + min + ":" + sec + "." + mil + " ";

	std::cout << line;
	Log::m_console += line;
}

void Log::Save(const std::string save, const bool overwrite) {
	std::fstream consoleLog;

	if (overwrite) {
		consoleLog.open(save, std::ios_base::out);
	} else {
		// checks if file exists
		consoleLog.open(save, std::ios_base::in);

		if (consoleLog.is_open()) {
			// file exist
			consoleLog.close();
			consoleLog.open(save, std::ios_base::app);
		} else {
			consoleLog.open(save, std::ios_base::out);
		}
	}

	consoleLog << Log::m_console;

	consoleLog.close();
}

void Log::StartTime() {
	Log::m_time = std::chrono::high_resolution_clock::now();
}

bool Log::CheckTime(const long long milliseconds) {
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - Log::m_time);
	return duration.count() >= milliseconds;
}

bool Log::CheckTimeSeconds(const double seconds) {
	return Log::CheckTime((long long)(std::ceil(seconds * 1000)));
}

std::string Log::ToString(const bool value) {
	return value ? "TRUE" : "FALSE";
}

std::string Log::ToString(const double value, const unsigned int precision) {
	std::stringstream out;
	out << std::fixed << std::setprecision(precision);
	out << value;
	return out.str();
}

std::string Log::ToString(const size_t value, const unsigned int precision, const char lead) {
	std::string out = std::to_string(value);

	if (precision > 0) {
		const int delta = (int)precision - (int)out.size();

		if (delta >= 1) {
			std::string leading = "";
			for (int i = 0; i < delta; i++) {
				leading += lead;
			}

			out = leading + out;
		}
	}

	return out;
}

std::string Log::ToString(const unsigned int value, const unsigned int precision, const char lead) {
	std::string out = std::to_string(value);

	if (precision > 0) {
		const int delta = (int)precision - (int)out.size();

		if (delta >= 1) {
			std::string leading = "";
			for (int i = 0; i < delta; i++) {
				leading += lead;
			}

			out = leading + out;
		}
	}

	return out;
}

std::string Log::LeadingCharacter(const std::string value, const unsigned int amount, const char lead) {
	std::string out = value;
	if (amount > 0) {
		const int delta = (int)amount - (int)out.size();

		if (delta >= 1) {
			std::string leading = "";
			for (int i = 0; i < delta; i++) {
				leading += lead;
			}

			out = leading + out;
		}
	}
	return out;
}

void Log::Sound(const long long duration) {
	std::cout << '\a';

	if (duration > 0) std::this_thread::sleep_for(std::chrono::seconds(duration));
}

void Log::HoldConsole() {
	std::cout << "\nPress any key to exit...";
	std::cin.ignore();
}
