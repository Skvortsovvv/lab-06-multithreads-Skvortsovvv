// Copyright 2020 Your Name <your_email>

#ifndef INCLUDE_HEADER_HPP_
#define INCLUDE_HEADER_HPP_

#include <boost/log/trivial.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/current_thread_id.hpp>

#include <stdio.h>

#include <stdexcept>
#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include "picosha2.h"
#include "boost/log/trivial.hpp"
#include <random>
#include "nlohmann/json.hpp"
#include <ctime>
#include <mutex>
#include <fstream>
#include <atomic>

using json = nlohmann::json;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

class Mine {
public:
	std::mutex mute;
	std::ofstream file;
	json Array;
	std::atomic<unsigned int> hash_counter = 0;
	std::vector<std::thread> vector_thead;
	std::atomic<bool> flag = true;
public:

	Mine() {
		Array = json::array();
	}

	std::vector<char> Data_Generator() {
		size_t lenght = Random_Number_Generator() % 129;
		std::vector<char> data;
		for (size_t i = 0; i < lenght; ++i) {
			int index = Random_Number_Generator() % 3;
			char c;
			switch (index)
			{
			case 0:
				c = 'a' + Random_Number_Generator() % 26;
				data.push_back(c);
			case 1:
				c = 'A' + Random_Number_Generator() % 26;
				data.push_back(c);
			case 2:
				int num = Random_Number_Generator() % 10;
				char ch = '0' + num;
				data.push_back(ch);
			}
		}
		return data;
	}

	bool Check(std::string& hash_string) {
		if (hash_string[60] == '0') {
			if (hash_string[61] == hash_string[60]) {
				if (hash_string[62] == hash_string[61]) {
					if (hash_string[63] == hash_string[62]) {
						return true;
					}
				}
			}
		}
		return false;
	}

	std::string Get_Hash_Code(const std::vector<char>& data) {
		std::string hash_string = picosha2::hash256_hex_string(data);
		return hash_string;
	}


	void Write_Json(const std::string& timestamp, const std::string& hash, const std::string& data) {
		mute.lock();
		json j;
		std::cout << "time: " << timestamp << '\n' << "hash: " << hash << '\n' << "data: " << data << '\n' << "___________________________" << '\n';
		j["timestamp"] = std::stoi(timestamp);
		j["hash"] = hash;
		j["data"] = data;
		Array.push_back(j);
		mute.unlock();
	}

	void Fun() {
		while (flag.load()) {
			if (hash_counter.load() < 1) {
				std::vector<char> data = Data_Generator();
				std::string hash_string = Get_Hash_Code(data);
				std::string data_string;
				for (auto ch : data) {
					data_string += ch;
				}
				if (Check(hash_string)) {
					if (hash_counter.load() < 1) {
						++hash_counter;
						std::time_t time_string = std::time(nullptr);
						BOOST_LOG_TRIVIAL(info) << "Input: " << data_string << "Hash: " << hash_string;
						Write_Json(std::to_string(time_string), hash_string, data_string);
					}
				}
				else {
					BOOST_LOG_TRIVIAL(trace) << "Input: " << data_string << "Hash: " << hash_string;
				}
			}
			else {
				flag = false;
			}
		}
	}
	void logger() {
		boost::log::add_common_attributes();
		boost::shared_ptr<logging::core> core = logging::core::get();
		core->add_global_attribute("ThreadID", attrs::current_thread_id());
		boost::shared_ptr<sinks::text_file_backend > backend =
			boost::make_shared<sinks::text_file_backend>(
				keywords::file_name = "Logs/file_%5N.log",
				keywords::rotation_size = 5 * 1024 * 1024);

		typedef sinks::synchronous_sink
			<sinks::text_file_backend> sink_f; // file sink
		//typedef sinks::synchronous_sink
		//	<sinks::text_ostream_backend> sink_c; // console sink
		//boost::shared_ptr<sink_c> console_sink(new sink_c());
		boost::shared_ptr<sink_f> file_sink(new sink_f(backend));
		//boost::shared_ptr<std::ostream> stream(&std::cout, boost::null_deleter());
		//console_sink->locked_backend()->add_stream(stream);
		file_sink->set_formatter(expr::stream
			<< expr::format_date_time<boost::posix_time::ptime>
			("TimeStamp", "%Y-%m-%d %H:%M:%S")
			<< ": <" << logging::trivial::severity
			<< "> <" << expr::attr<attrs::current_thread_id::value_type>("ThreadID")
			<< "> " << expr::smessage);
		/*console_sink->set_formatter(expr::stream
			<< expr::format_date_time<boost::posix_time::ptime>
			("TimeStamp", "%Y-%m-%d %H:%M:%S")
			<< ": <" << logging::trivial::severity
			<< "> [" << expr::attr<attrs::current_thread_id::value_type>("ThreadID")
			<< "] " << expr::smessage);*/
		core->add_sink(file_sink);
		//core->add_sink(console_sink);
	}



	int Average_Distance(const std::vector<int>& permutation) {
		int distance_sum = 0;
		for (int i = 0; i < 1000; i++)
			distance_sum += abs(permutation[i] - i);

		return distance_sum / 1000;
	}

	int Random_Number_Generator() {
		std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
		std::vector<int> permutation(1000);

		for (int i = 0; i < 1000; i++)
			permutation[i] = i;
		shuffle(permutation.begin(), permutation.end(), rng);
		int part1 = Average_Distance(permutation);
		shuffle(permutation.begin(), permutation.end(), rng);
		int part2 = Average_Distance(permutation);
		int index = permutation[std::rand() % 1000] + 1;
		int a_ = part1 * part2;
		int b_ = a_;
		int result = b_ % index;
		return result;
	}

	void Mining(int amount, std::string path) {
		std::cout << "Start mining with " << amount << " threads" << '\n';
		for (int i = 0; i < amount; ++i) {
			vector_thead.push_back(std::thread(&Mine::Fun, this));
		}
		for (auto& th : vector_thead) {
			th.join();
		}
		file.open(path);
		file << Array;
		file.close();
		std::cout << "Done!" << '\n';
	}

	void Save_Json(const std::string& path) {
		file.open(path);
		file << Array;
		file.close();
		std::cout << "Saved!" << '\n';
	}

};


#endif // INCLUDE_HEADER_HPP_

