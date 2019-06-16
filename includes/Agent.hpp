#pragma once

#include "Krpsim.hpp"
#include "Parser.hpp"

struct Stock;

class Agent {
public:
	Agent() {};
	Agent(std::vector<Stock> &vStock);
	~Agent() {};

	std::map<std::string, std::map<std::string, int>> stockMultProdWayCnt; // stockMultProdWayCnt[stockName][processName]
	std::map<std::string, std::map<std::string, int>> stockMultProdWayProb; // stockMultProdWayProb[stockName][processName]

	// std::map<std::string, std::vector<int> > vProcess;
	// std::map<std::string, int> currentStock;
};
