#include "Agent.hpp"

Agent::Agent(std::vector<Stock> &vStock, std::map<std::string, int> startStock) : currentStock(startStock) {
	for (auto &stock : vStock) {
		if (stock.processForProduce.size() > 1) {
			for (auto &process : stock.processForProduce) {
				stockMultProdWayCnt[stock.name][process.first] = 0;
				stockMultProdWayProb[stock.name][process.first] = 100 / stock.processForProduce.size();
			}
		}
	}
}

void Agent::describe() {
	std::cerr << "--------- Agent description ---------" << std::endl;
	for (auto &stock : stockMultProdWayProb) {
		std::cerr << "- Stock " << stock.first << " process choice:" << std::endl;
		for (auto &process : stock.second) {
			std::cerr << "   " << process.first << " " << process.second << "%" << std::endl;
		}
	}
	std::cerr << "--------- Current Stock" << std::endl;
	for (auto &stock : currentStock) {
		std::cerr << stock.first << " " << stock.second << std::endl;
	}
	std::cerr << "--------- Current Process" << std::endl;
	for (auto &stock : currentProcess) {
		std::cerr << stock.first << " " << stock.second.size() << std::endl;
	}
}

bool Agent::selectProcess(Stock *stock, std::string &processName) {
	int cnt = 0;
	bool fullChoice = true;
	if (stockMultProdWayProb.find(stock->name) != stockMultProdWayProb.end()) {
		stockDoableMultProdWayProb.clear();
		for (auto &s : stockMultProdWayProb[stock->name]) {
			if (doableProcess[s.first] && processName != s.first)
				stockDoableMultProdWayProb[s.first] = s.second;
		}
		if (stockDoableMultProdWayProb.size() == 0) {
			for (auto &p : stockMultProdWayProb[stock->name]) {
				if (processName != p.first)
					stockDoableMultProdWayProb[p.first] = stockMultProdWayProb[stock->name][p.first];
			}
		}
		int mod = 0;
		for (auto &s : stockDoableMultProdWayProb)
			mod += s.second;
		// if (stockDoableMultProdWayProb.size() < stockMultProdWayProb[stock->name].size())
		// 	fullChoice = false;
		if (stockDoableMultProdWayProb.size() <= 1)
			fullChoice = false;
		int chosenProcess = rand() % mod;
		for (auto &processToChoose : stockDoableMultProdWayProb) {
			cnt += processToChoose.second;
			if (chosenProcess <= cnt) {
				processName = processToChoose.first;
				break ;
			}
		}
	}
	else
		for (auto &processToChoose : stock->processForProduce)
			processName = processToChoose.first;
	return fullChoice;
}

bool Agent::canBeProduce(Process *process) {
	for (auto &stock : process->neededStock) {
		if (stock.second > currentStock[stock.first])
			return false;
	}
	return true;
}

void Agent::productStockProcess(Process *process, int currentCycle, std::string processName) {
	for (auto &stock : process->neededStock) {
		if (stock.second > currentStock[stock.first]) {
			produceStock(process->stockNeeded[stock.first], currentCycle, processName);
		}
	}
}

void Agent::applyProcess(Process *process, int currentCycle) {
	for (auto &stock : process->neededStock)
		currentStock[stock.first] -= stock.second;
	currentProcess[process->name].push_back(process->delay);
	processHistory[currentCycle].push_back(process->name);
}

void Agent::searchDouableProcess() {
	for (auto &process : mapProcess)
		doableProcess[process.first] = canBeProduce(process.second);
}

bool Agent::nothingToDo() {
	bool ret;
	for (auto & p : doableProcess) {
		if (p.second) {
			ret =  false;
			for (auto & loop : doableProcess) {
				if (loop.second
						&& mapProcess[loop.first]->neededStock
						== mapProcess[p.first]->resultStock
						&& mapProcess[loop.first]->resultStock
						== mapProcess[p.first]->neededStock) {
					ret = true;
				}
			}
			if (!ret)
				return false;
		}
	}
	return true;
}

void Agent::produceStock(Stock *stock, int currentCycle, std::string processName, std::map<std::string, Process *> processMap) {
	if (processMap.size())
		mapProcess = processMap;
	searchDouableProcess();
	bool fullChoice = selectProcess(stock, processName);
	if (nothingToDo())
		return ;
	if (stock->processForProduce.find(processName) != stock->processForProduce.end()) { // Process exist
		if (canBeProduce(stock->processForProduce[processName])) { // Process can be produce
			std::cout << "Let's go " << processName << std::endl;
			if (fullChoice)
				stockMultProdWayCnt[stock->name][processName]++;
			applyProcess(stock->processForProduce[processName], currentCycle);
		}
		else { // Process can't be produce
			std::cout << "Take your time for " << processName << std::endl;
			productStockProcess(stock->processForProduce[processName], currentCycle, processName);
		}
	}
	else { // There is no way to produce this stock
		std::cout << "OVERWERK - Moments (alt) ->" << stock->name << " " << processName << std::endl;
	}
	std::cout << "Choosen process -> "<< processName << std::endl;

}

void Agent::addStock(std::string processName) {
	for (auto &stockName : mapProcess[processName]->resultStock) {
		currentStock[stockName.first] += stockName.second;
	}
}

void Agent::addNewStockFromEndedProcess() {
	for (auto &process : currentProcess) {
		for (size_t i = 0; i < process.second.size(); i++) {
			process.second[i]--;
			if (process.second[i] == 0) {
				addStock(process.first);
				process.second.erase(process.second.begin() + i);
				i--;
			}

		}
	}
}
