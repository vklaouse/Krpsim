#pragma once

#include "Krpsim.hpp"
#include "DNA.class.hpp"
#include "Parser.class.hpp"


class ParserVerif : public Parser {

public:

    ParserVerif(std::vector<Token> &tokens);
    ~ParserVerif() {};

	std::vector<std::string> getErrors() { return errors; };
	void checker();

private:
	void applyDelay();
	void saveProcessWithDelay(std::vector<std::string> vProcess);
	void applyToStock(bool addToStock, int nbrOfApply, std::string processName);
	void displayResult();

	std::map<std::string, int> mStock;
	std::map<std::string, int> mOriginStock;
    std::map<int, std::vector<std::string>> mProcess;
	std::map<std::string, std::vector<int>> mChecker;
	std::vector<std::string> errors;
	int endCycle;
};
