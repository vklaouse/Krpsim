#include "ParserVerif.hpp"

ParserVerif::ParserVerif(std::vector<Token> &tokens) {
	mStock = Parser::instance->getStartStock();
	mOriginStock = Parser::instance->getStartStock();
	endCycle = 0;
	for (size_t i = 0; i < tokens.size(); i++) {
		if (tokens[i].type == stock) {
			int myInt = 0;
			saveStrInInt(tokens[i + 1].info, &myInt);
			if (myInt >= 0) {
				mStock[tokens[i].info] = myInt;
				i++;
			}
			else
				errors.push_back("Quantity can't be negative");
		}
		else if (tokens[i].type == cycle) {
			std::vector<std::string> operations;
			int j = i + 1;

			for (; tokens[j].type == operation; j++) {
				if (tokens[j].info.size() <= 0)
					break ;
				bool exist = false;
				for (const auto &trueProcess : Parser::instance->getProcess()) {
					if (trueProcess.name.compare(tokens[j].info) == 0	) {
						exist = true;
						break ;
					}
				}
				if (!exist)
					errors.push_back(tokens[j].info + " doesn't exist");
				operations.push_back(tokens[j].info);
			}
			int myInt = 0;
			saveStrInInt(tokens[i].info, &myInt);
			mProcess[myInt] = operations;
			if (endCycle < myInt)
				endCycle = myInt;
			i = j - 1;
		}
	}
}

void ParserVerif::checker() {
	saveProcessWithDelay(mProcess[0]);
	if (endCycle == 0)
		displayResult();
	for (size_t i = 1; i <= (size_t)endCycle; i++) {
		applyDelay();
		saveProcessWithDelay(mProcess[i]);
		if (i == (size_t)endCycle - 1)
			displayResult();
	}
}

void ParserVerif::displayResult() {
	for (const auto &s : mOriginStock) {
		if (s.second != mStock[s.first]) {
			std::cerr << "Expected result : " << std::endl;
			for (const auto &s : mStock) {
				std::cerr  << "\t" << s.first << " " << s.second << std::endl;
			}
			std::cerr << "Verif result : " << std::endl;
			for (const auto &s : mOriginStock) {
				std::cerr  << "\t" << s.first << " " << s.second << std::endl;
			}
			return ;
		}
	}
	std::cout << "Stock after " << endCycle << " cycle :" << std::endl;
	for (const auto &s : mOriginStock) {
		std::cout << "\t" << s.first << " " << s.second << std::endl;
	}
}

void ParserVerif::applyToStock(bool addToStock, int nbrOfApply, std::string processName) {
	for (const auto &trueProcess : Parser::instance->getProcess()) {
		if (trueProcess.name == processName) {
			if (addToStock) {
				for (const auto &p : trueProcess.resultStock) {
					mOriginStock[p.first] += p.second * nbrOfApply;
				}
			}
			else {
				for (const auto &p : trueProcess.neededStock) {
					mOriginStock[p.first] -= p.second * nbrOfApply;
					if (mOriginStock[p.first] < 0)
						std::cerr << "A stock has a negative quantity" << std::endl;
				}
			}
			break ;
		}
	}
}

void ParserVerif::applyDelay() {
	int endDelay;

	for (auto process : mChecker) {
		endDelay = 0;
		for (size_t i = 0; i < process.second.size(); i++) {
			mChecker[process.first][i] -= 1;
			if (mChecker[process.first][i] == 0)
				endDelay++;
		}
		if (endDelay > 0) {
			mChecker[process.first].erase(mChecker[process.first].begin(), mChecker[process.first].begin() + endDelay);
			applyToStock(true, endDelay, process.first);
		}
	}
}

void ParserVerif::saveProcessWithDelay(std::vector<std::string> vProcessName) {
	for (const auto &trueProcess : Parser::instance->getProcess()) {
		for (const auto &p : vProcessName) {
			if (p == trueProcess.name) {
				mChecker[p].push_back(trueProcess.delay);
				applyToStock(false, 1, p);
			}
		}
	}
}
