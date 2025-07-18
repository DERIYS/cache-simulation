#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <string>
#include <vector>
#include "../../include/structs/default.h"
#include "legacy.h"

class Application {
private:

	const std::string title = R"(
			  ______      ___        ______  __    __   _______ 
			 /      |    /   \      /      ||  |  |  | |   ____|
			|  ,----'   /  ^  \    |  ,----'|  |__|  | |  |__   
			|  |       /  /_\  \   |  |     |   __   | |   __|  
			|  `----. /  _____  \  |  `----.|  |  |  | |  |____ 
			\______|/__ /     \__\ \______| |__|  |__| |_______|
		)";

	const std::string startButton = "Start Simulation";

	const std::string configureButton = "Configure";

	const std::string exitButton = "Exit";

	const std::string cursor = ">";

	const std::string inputConf = "Input file: ";

	const std::string tracefileConf = "Trace file: ";

	const std::string cyclesConf = "Cycles: ";

	const std::string numCacheLevelsConf = "Number of cache levels: ";

	const std::string cacheLineSizeConf = "Cache line size: ";

	const std::string numLinesL1Conf = "Line number for L1 cache: ";

	const std::string numLinesL2Conf = "Line number for L2 cache: ";

	const std::string numLinesL3Conf = "Line number for L3 cache: ";

	const std::string latencyL1Conf = "Latency L1: ";

	const std::string latencyL2Conf = "Latency L2: ";

	const std::string latencyL3Conf = "Latency L3: ";

	const std::string mappingStratConf = "Mapping strategy: ";

	const std::string debugConf = "Debug mode: ";

	const std::string back = "Back";


	enum class MainMenuOption {
		Start,
		Configure,
		Exit,
		Count
	};

	enum class ConfigOption {
		Input,
		Tracefile,
		Cycles,
		NumberCacheLevels,
		CacheLineSize,
		NumberLinesL1,
		NumberLinesL2,
		NumberLinesL3,
		LatencyL1,
		LatencyL2,
		LatencyL3,
		MappingStrat,
		Debug,
		Back,
		Count
	};

	uint32_t  cycles = CYCLES;
	uint32_t  cachelineSize = CACHE_LINE_SIZE;
	uint32_t  numLinesL1 = NUM_LINES_L1;
	uint32_t  numLinesL2 = NUM_LINES_L2;
	uint32_t  numLinesL3 = NUM_LINES_L3;
	uint32_t  latencyCacheL1 = LATENCY_CACHE_L1;
	uint32_t  latencyCacheL2 = LATENCY_CACHE_L2;
	uint32_t  latencyCacheL3 = LATENCY_CACHE_L3;
	uint8_t   numCacheLevels = NUM_CACHE_LEVELS;
	bool   	  mappingStrategy = MAPPING_STRATEGY == 1 ? true : false;
	bool 	  tracefile       = false;
	bool	  debugMode = false;
	const char* tracefileName = "tracefile"; 

	MainMenuOption currentMOption;
	ConfigOption currentCOption;

	bool isRunning, isMainMenu;

	void printMenu();
	void enterOption();
	void incrementConfigOptionValue(ConfigOption option);
	void decrementConfigOptionValue(ConfigOption option);
	void runCacheSimulation();


	template <typename OptionEnum>
	void readOptionInput(OptionEnum& currentOption);

	void readInput();

public:

	Application();
	~Application() = default;

	void run();
};

#endif // APLICATION_HPP