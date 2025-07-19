#include "../include/application.hpp"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <sys/wait.h>
#include "application.hpp"

int getch() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

Application::Application(){
	isRunning = true;
	isMainMenu = true;
	currentMOption = MainMenuOption::Start;
	currentCOption = ConfigOption::Input;
}

void Application::printMenu()
{
	system("clear");

	std::cout << title << "\n";

	if (isMainMenu){
		std::cout << "\t\t\t" << (currentMOption == MainMenuOption::Start ? cursor : " ") << startButton << "\n";
		std::cout << "\t\t\t" << (currentMOption == MainMenuOption::Configure ? cursor : " ") << configureButton << "\n";
		std::cout << "\t\t\t" << (currentMOption == MainMenuOption::Exit ? cursor : " ") << exitButton << "\n";
	}
	else // Configure 
	{
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::Input ? cursor : " ") << inputConf << "requests.csv" << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::Tracefile ? cursor : " ") << tracefileConf << (tracefile ? "tracefile.tf" : " ") << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::Cycles ? cursor : " ") << cyclesConf << cycles << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::NumberCacheLevels ? cursor : " ") << numCacheLevelsConf << static_cast<uint32_t>(numCacheLevels) << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::CacheLineSize ? cursor : " ") << cacheLineSizeConf << cachelineSize << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::NumberLinesL1 ? cursor : " ") << numLinesL1Conf << numLinesL1 << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::NumberLinesL2 ? cursor : " ") << numLinesL2Conf << numLinesL2 <<"\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::NumberLinesL3 ? cursor : " ") << numLinesL3Conf << numLinesL3 << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::LatencyL1 ? cursor : " ") << latencyL1Conf << latencyCacheL1 << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::LatencyL2 ? cursor : " ") << latencyL2Conf << latencyCacheL2 << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::LatencyL3 ? cursor : " ") << latencyL3Conf << latencyCacheL3 << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::MappingStrat ? cursor : " ") << mappingStratConf << (mappingStrategy ? "Fully associative" : "Direct-mapped") << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::Debug ? cursor : " ") << debugConf << (debugMode ? "ON" : "OFF") << "\n";
		std::cout << "\t\t\t" << (currentCOption == ConfigOption::Back ? cursor : " ") << back << "\n";
	}
}


void Application::enterOption()
{
	if (isMainMenu) {
		switch(currentMOption) {
		case MainMenuOption::Start:
			runCacheSimulation();
			break;
		case MainMenuOption::Configure:
			isMainMenu = false;
			break;
		case MainMenuOption::Exit:
			isRunning = false;
			break;

		}
	}
	else {
		switch(currentCOption){
			case ConfigOption::Back:
				isMainMenu = true;
				break;
		}
	}
}

void Application::incrementConfigOptionValue(ConfigOption option)
{
	switch (option) {
        case ConfigOption::Cycles:              cycles += 100; break;
		case ConfigOption::Tracefile:			tracefile = !tracefile; break;
        case ConfigOption::NumberCacheLevels:   if (numCacheLevels < 3) numCacheLevels++; break;
        case ConfigOption::CacheLineSize:       cachelineSize *= 2; break;
        case ConfigOption::NumberLinesL1:       numLinesL1 *= 2; break;
        case ConfigOption::NumberLinesL2:       numLinesL2 *= 2; break;
        case ConfigOption::NumberLinesL3:       numLinesL3 *= 2; break;
        case ConfigOption::LatencyL1:           latencyCacheL1 += 1; break;
        case ConfigOption::LatencyL2:           latencyCacheL2 += 1; break;
        case ConfigOption::LatencyL3:           latencyCacheL3 += 1; break;
        case ConfigOption::MappingStrat:        mappingStrategy = !mappingStrategy; break;
        case ConfigOption::Debug:               debugMode = !debugMode; break;
        default: break;
    }	
}

void Application::decrementConfigOptionValue(ConfigOption option)
{
	switch (option) {
        case ConfigOption::Cycles:              if (cycles > 100) cycles -= 100; break;
		case ConfigOption::Tracefile:			tracefile = !tracefile; break;
        case ConfigOption::NumberCacheLevels:   if (numCacheLevels > 1) numCacheLevels--; break;
        case ConfigOption::CacheLineSize:       if (cachelineSize > 1) cachelineSize /= 2; break;
        case ConfigOption::NumberLinesL1:       if (numLinesL1 > 1) numLinesL1 /= 2; break;
        case ConfigOption::NumberLinesL2:       if (numLinesL2 > 1) numLinesL2 /= 2; break;
        case ConfigOption::NumberLinesL3:       if (numLinesL3 > 1) numLinesL3 /= 2; break;
        case ConfigOption::LatencyL1:           if (latencyCacheL1 > 1) latencyCacheL1 -= 1; break;
        case ConfigOption::LatencyL2:           if (latencyCacheL2 > 1) latencyCacheL2 -= 1; break;
        case ConfigOption::LatencyL3:           if (latencyCacheL3 > 1) latencyCacheL3 -= 1; break;
        case ConfigOption::MappingStrat:        mappingStrategy = !mappingStrategy; break;
        case ConfigOption::Debug:               debugMode = !debugMode; break;
        default: break;
    }
}

void Application::runCacheSimulation()
{
	pid_t pid = fork();

    if (pid == 0) {
        // Child process
		// Allocate all strings first
		char* cycles_str = uint32_to_string(cycles);
		char* cachelineSize_str = uint32_to_string(cachelineSize);
		char* numLinesL1_str = uint32_to_string(numLinesL1);
		char* numLinesL2_str = uint32_to_string(numLinesL2);
		char* numLinesL3_str = uint32_to_string(numLinesL3);
		char* latencyCacheL1_str = uint32_to_string(latencyCacheL1);
		char* latencyCacheL2_str = uint32_to_string(latencyCacheL2);
		char* latencyCacheL3_str = uint32_to_string(latencyCacheL3);
		char* numCacheLevels_str = uint8_to_string(numCacheLevels);
		char* mappingStrategy_str = uint8_to_string(mappingStrategy);

        std::vector<char*> args;
		args.push_back((char*)"../../project");

		if (debugMode)
		    args.push_back((char*)"-d");

		args.push_back((char*)"-c");
		args.push_back(cycles_str);

		if (tracefile){
			args.push_back((char*)"-f");
			args.push_back((char*)(tracefileName));
		}

		args.push_back((char*)"-C");
		args.push_back(cachelineSize_str);

		args.push_back((char*)"-L");
		args.push_back(numLinesL1_str);

		args.push_back((char*)"-M");
		args.push_back(numLinesL2_str);

		args.push_back((char*)"-N");
		args.push_back(numLinesL3_str);

		args.push_back((char*)"-l");
		args.push_back(latencyCacheL1_str);

		args.push_back((char*)"-m");
		args.push_back(latencyCacheL2_str);

		args.push_back((char*)"-n");
		args.push_back(latencyCacheL3_str);

		args.push_back((char*)"-e");
		args.push_back(numCacheLevels_str);

		args.push_back((char*)"-S");
		args.push_back(mappingStrategy_str);

		args.push_back((char*)"../../requests.csv");

		args.push_back(NULL);

        execvp(args[0], args.data());
        perror("execvp failed"); // If exec fails

		free(cycles_str);
		free(cachelineSize_str);
		free(numLinesL1_str);
		free(numLinesL2_str);
		free(numLinesL3_str);
		free(latencyCacheL1_str);
		free(latencyCacheL2_str);
		free(latencyCacheL3_str);
		free(numCacheLevels_str);
		free(mappingStrategy_str);

        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
		waitpid(pid, &status, 0);
        std::cout << "\nSimulation finished. Press ENTER to return to the menu...";
        std::cin.ignore(); // Wait for ENTER
        std::cin.get();   
        system("clear");  
    } else {
        perror("fork failed");
    }

}

template <typename OptionEnum>
void Application::readOptionInput(OptionEnum& currentOption) {
	int ch = getch();

   if (ch == 27) { // ESC or arrow start
        int next1 = getch();
        if (next1 == '[') {
            int next2 = getch();
            int currentIndex = static_cast<int>(currentOption);
            int count = static_cast<int>(OptionEnum::Count);

            switch (next2) {
                case 'A': // Up arrow
                    currentIndex = (currentIndex - 1 + count) % count;
                    system("clear");
                    break;
                case 'B': // Down arrow
                    currentIndex = (currentIndex + 1) % count;
                    system("clear");
                    break;
				case 'C': // Right - increase
        			if constexpr (std::is_same<OptionEnum, ConfigOption>::value)
            			incrementConfigOptionValue(currentOption);
        			break;
    			case 'D': // Left - decrease
        			if constexpr (std::is_same<OptionEnum, ConfigOption>::value)
            			decrementConfigOptionValue(currentOption);
        			break;
                default:
                    break;
        	}
			currentOption = static_cast<OptionEnum>(currentIndex);
		}
		else // ESC
		{
			isRunning = false;
		}
	}
	else if (ch == 10 || ch == 13) // Enter
	{
		system("clear");
		enterOption();
	}

	else {
		// nothing
	}
}

void Application::readInput() {
	if (isMainMenu) {
		readOptionInput(currentMOption);
	}
	else {
		readOptionInput(currentCOption);
	}
	

}

void Application::run(){

	while (isRunning) {
		printMenu();
		readInput();
	}
}