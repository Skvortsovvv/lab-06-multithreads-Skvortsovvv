#include <stdio.h>
#include <header.hpp>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <csignal>

Mine Miner;
int threads = 0;
std::string path;

#ifdef __linux__
void signal_callback_handler(int signum) {
	std::cout << "Stop process" << '\n';
	Miner.Save_Json(path);
	exit(signum);
}
#elif _WIN32
#include <windows.h>
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {

	switch (fdwCtrlType)
	{
	case CTRL_C_EVENT:
		std::cout << "Stop process" << '\n';
		Miner.Save_Json(path);
		return FALSE;
	default:
		return FALSE;
	}
}
#endif

int main(int argc, char* argv[]) {
	if (argc == 1) {
		std::cout << "Default situation" << '\n';
		threads = std::thread::hardware_concurrency();
		path = "found_hash.json";
	}
	else if (argc == 3) {
		threads = std::stoi(argv[1]);
		path = argv[2];
	}
	else {
		throw(std::invalid_argument("Error! Wrong number of arguments"));
	}

#ifdef __linux__
	signal(SIGINT, signal_callback_handler);
#elif _WIN32
	SetConsoleCtrlHandler(CtrlHandler, TRUE);
#endif
	Miner.logger();
	Miner.Mining(threads, path);
	return 0;

}
