#pragma once

#include "Krpsim.hpp"
#include "Parser.hpp"

struct Stock;
struct Process;

class Agent {
public:
	Agent() {};
	Agent(std::vector<Stock> &vStock, std::map<std::string, int> startStock);
	~Agent() {};

	void describe();
	void produceStock(Stock *stock, int currentCycle, std::string processName = "", std::map<std::string, Process *> processMap = std::map<std::string, Process *>(), int nbr = 1);
	bool selectProcess(Stock *stock, std::string &process);
	bool canBeProduce(Process *process);
	void productStockProcess(Process *process, int currentCycle, std::string processName, int nbr);
	void applyProcess(Process *process, int currentCycle);
	void addNewStockFromEndedProcess();
	void addStock(std::string processName);
	void searchDouableProcess();
	bool nothingToDo();
	int processInProgress();
	void upgrade();

	std::map<std::string, std::map<std::string, int>> stockMultProdWayCnt; // stockMultProdWayCnt[stockName][processName]
	std::map<std::string, std::map<std::string, int>> stockMultProdWayProb; // stockMultProdWayProb[stockName][processName]
	std::map<std::string, int> stockDoableMultProdWayProb; // stockDoableMultProdWayProb[processName]

	std::map<std::string, std::vector<int> > currentProcess;
	std::map<int, std::vector<std::string>> processHistory;
	std::map<std::string, int> currentStock;
	std::map<std::string, int> inProgressStock;
	std::map<std::string, Process *> mapProcess;
	std::map<std::string, bool> doableProcess;
	std::map<std::string, bool> processNotDoable;
	int noMoreProcessToRun;
	int lastCycle = 0;

};
