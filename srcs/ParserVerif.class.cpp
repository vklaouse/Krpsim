#include "ParserVerif.class.hpp"

ParserVerif::ParserVerif(std::vector<Token> &tokens) {
	mStock = Parser::instance->getStartStock();
	mOriginStock = Parser::instance->getStartStock();
	for (size_t i = 0; i < tokens.size(); i++) {
		if (tokens[i].type == stock) {
			int myInt = 0;
			if (saveStrInInt(tokens[i + 1].info, &myInt) && myInt != 0) {
				mStock[tokens[i].info] = myInt;
				i++;
			}
			else
				errors.push_back("Quantity can't be 0");
		}
		else if (tokens[i].type == cycle) {
			std::vector<std::string> operations;
			int j = i + 1;
			for (; tokens[j].type == operation; j++) {
				bool exist = false;
				for (const auto &trueProcess : Parser::instance->getProcess()) {
					if (trueProcess.name == tokens[j].info) {
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
			i += j - i - 1;
		}
	}
	// for (const auto &s : mStock) {
	// 	std::cout << s.first << " " << s.second << std::endl;
	// }
	// std::cout << " ============= " << std::endl;
	// for (const auto &s : mProcess) {
	// 	std::cout <<  s.first << " ";
	// 	for (const auto &sr : s.second) {
	// 		std::cout <<  sr << " ";
	// 	}
	// 	std::cout << std::endl;
	// }
}

void ParserVerif::checker() {
	saveProcessWithDelay(mProcess[0]);
	for (size_t i = 1; i < 10000; i++) {
		applyDelay();
		saveProcessWithDelay(mProcess[i]);
	}
	for (const auto &s : mOriginStock) {
		std::cout << s.first << " " << s.second << std::endl;
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
						std::cerr << "Stock can't become negative" << std::endl;
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

void ParserVerif::saveProcessWithDelay(std::vector<std::string> vProcess) {
	for (const auto &trueProcess : Parser::instance->getProcess()) {
		for (const auto &p : vProcess) {
			if (p == trueProcess.name) {
				mChecker[p].push_back(trueProcess.delay);
				applyToStock(false, 1, p);
			}
		}
	}
}
