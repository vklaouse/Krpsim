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
	std::cerr << "--------- Last Cycle -> " << lastCycle << std::endl;

}

bool Agent::selectProcess(Stock *stock, std::string &processName) {
	int cnt = 0;
	bool fullChoice = true;
	processName = "";
	if (stockMultProdWayProb.find(stock->name) != stockMultProdWayProb.end()) {
		stockDoableMultProdWayProb.clear();
		for (auto &s : stockMultProdWayProb[stock->name]) {
			// if (doableProcess[s.first] && processName != s.first)
			if (doableProcess[s.first] && processName != s.first && processNotDoable.find(s.first) == processNotDoable.end())
				stockDoableMultProdWayProb[s.first] = s.second;
		}
		if (stockDoableMultProdWayProb.size() == 0) {
			for (auto &p : stockMultProdWayProb[stock->name]) {
				// if (processName != p.first)
				if (processName != p.first && processNotDoable.find(p.first) == processNotDoable.end())
					stockDoableMultProdWayProb[p.first] = stockMultProdWayProb[stock->name][p.first];
			}
		}
		int mod = 0;
		for (auto &s : stockDoableMultProdWayProb)
			mod += s.second;
		// if (stockDoableMultProdWayProb.size() < stockMultProdWayProb[stock->name].size())
		// 	fullChoice = false;
		// if (stockDoableMultProdWayProb.size() <= 1)
		// 	fullChoice = false;
		if (mod == 0)
			return false;
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

void Agent::productStockProcess(Process *process, int currentCycle, std::string processName, int nbr) {
	for (auto &stock : process->neededStock) {
		if (stock.second > currentStock[stock.first]) {
		// if (stock.second > currentStock[stock.first] && inProgressStock[stock.first] < nbr) {
			produceStock(process->stockNeeded[stock.first], currentCycle, processName, std::map<std::string, Process *>(), nbr);
		}
	}
}

void Agent::applyProcess(Process *process, int currentCycle) {
	for (auto &stock : process->neededStock)
		currentStock[stock.first] -= stock.second;
	for (auto &stock : process->resultStock)
		inProgressStock[stock.first] += stock.second;
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

void Agent::produceStock(Stock *stock, int currentCycle, std::string processName, std::map<std::string, Process *> processMap, int nbr) {
	std::string prevProcessName = processName;
	if (processMap.size())
		mapProcess = processMap;
	searchDouableProcess();
	bool fullChoice = selectProcess(stock, processName);
	if (stock->processForProduce.find(processName) != stock->processForProduce.end()) { // Process exist
		if (prevProcessName != "") {
			for (int i = 0; i < mapProcess[prevProcessName]->neededStock[stock->name]; i += mapProcess[processName]->resultStock[stock->name]) {
				if (canBeProduce(stock->processForProduce[processName])) { // Process can be produce
					if (fullChoice && stockMultProdWayCnt.find(stock->name) != stockMultProdWayCnt.end())
						stockMultProdWayCnt[stock->name][processName]++;
					applyProcess(stock->processForProduce[processName], currentCycle);
					noMoreProcessToRun++;
				}
				else { // Process can't be produce
					if (nothingToDo() || (processNotDoable.find(processName) != processNotDoable.end() && processNotDoable[processName]))
						return ;
					processNotDoable[processName] = true;
					productStockProcess(stock->processForProduce[processName], currentCycle, processName, nbr * mapProcess[prevProcessName]->neededStock[stock->name]);
					return ;
				}
			}
		}
		else {
			noMoreProcessToRun = 1;
			while (noMoreProcessToRun > 0) {
				noMoreProcessToRun = 0;
				processNotDoable.clear();
				searchDouableProcess();
				fullChoice = selectProcess(stock, processName);
				if (processName == "")
					return ;
				if (canBeProduce(stock->processForProduce[processName])) { // Process can be produce
					if (fullChoice)
						stockMultProdWayCnt[stock->name][processName]++;
					applyProcess(stock->processForProduce[processName], currentCycle);
					noMoreProcessToRun++;
				}
				else { // Process can't be produce
					if (nothingToDo() || (processNotDoable.find(processName) != processNotDoable.end() && processNotDoable[processName]))
						return ;
					processNotDoable[processName] = true;
					productStockProcess(stock->processForProduce[processName], currentCycle, processName, nbr);
					nbr++;
				}
			}
		}
	}
}

void Agent::addStock(std::string processName) {
	for (auto &stockName : mapProcess[processName]->resultStock) {
		currentStock[stockName.first] += stockName.second;
		inProgressStock[stockName.first] -= stockName.second;
	}
}

void Agent::addNewStockFromEndedProcess() {
	lastCycle++;
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

int Agent::processInProgress() {
	int res = 0;
	for (auto &process : currentProcess)
		res += process.second.size();
	return res;
}

void Agent::upgrade() {
	for (auto &stock : stockMultProdWayCnt) {
		int total = 0;
		for (auto &process : stock.second)
			total += process.second;
		if (total > 0)
			for (auto &process : stock.second)
				stockMultProdWayProb[stock.first][process.first] = 100.0f * ((float)process.second / (float)total);
	}
}
