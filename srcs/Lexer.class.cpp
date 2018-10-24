#include "Lexer.class.hpp"

Lexer::Lexer(char *filePath) {
	tokens = std::vector<Token>();
	errors = std::vector<std::string>();

	cFileStream = new std::ifstream(filePath);
	if (cFileStream->fail()) {
		std::cerr << "Wrong file path -> " << filePath << std::endl;
		return;
	}

	std::string sLine;
	TokenType tokenType = stock;
	// Parse stock
	std::cout << "--- Stock --------" << std::endl;
	for (size_t i = 0; std::getline(*cFileStream, sLine); i++) {
		if (sLine.front() == '#') {
			continue;
		}

		std::cout << sLine << std::endl;
		size_t separatorCount = std::count(sLine.begin(), sLine.end(), ':');
		if (tokenType == stock && separatorCount == 1) {
			// stock
			createStock(sLine, stock, i);
		}
		else if ((tokenType == operation || tokenType == stock) && separatorCount >= 3) {
			// operation
			tokenType = operation;
			std::string operationName = sLine.substr(0, sLine.find(':'));
			if (!strIsAlNum(operationName, i)) 
				return ;
			tokens.push_back(Token(operation, operationName));

			sLine = sLine.substr(sLine.find(':'), sLine.size());
			if (!tokenizeStock(&sLine, i, needed_stock))
				continue;		

			if (!tokenizeStock(&sLine, i, result_stock))
				continue;		

			// tokenize process delay
			if (sLine.front() != ':') {
				addError(i, "Invalid char after first ':'");
				continue;
			}
			if (!strIsInt(sLine, i)) 
				return ;
			tokens.push_back(Token(delay, sLine.substr(1, sLine.size())));
		}
		else if (tokenType == operation && separatorCount == 1) {
			// optimize
			tokenizeOptimize(i, sLine);
			tokenType = optimize;
		}
		else {
			std::string phase = (tokenType == stock) ? "stock" : (tokenType == operation) ? "operation" : "optimize";
			addError(i, "Error when fetching '" + phase + "' data");
		}
	}
}

Lexer::~Lexer() {
	delete cFileStream;
}

bool Lexer::tokenizeStock(std::string *toParse, size_t i, TokenType tokenType) {
	if (toParse->front() != ':') {
		addError(i, "Expected ':' char");
		return false;
	}
	if (toParse->at(1) == '(') {
		if (toParse->find(')') == std::string::npos) {
			addError(i, "No closing ')' in operation");
			return false;
		}
		tokenizeGroupStock(i, toParse->substr(2, toParse->find(')') - 2), tokenType);
		*toParse = toParse->substr(toParse->find(')') + 1, toParse->size());
	}
	return true;
}

void Lexer::tokenizeGroupStock(size_t i, std::string groupStock, TokenType tokenType) {
	if (groupStock.size() == 0) {
		addError(i, "Empty group");
		return ;
	}
	size_t neededCount = std::count(groupStock.begin(), groupStock.end(), ';');
	size_t neededCountCheck = std::count(groupStock.begin(), groupStock.end(), ':');
	if (neededCount == std::string::npos) { // No ';' char in needed stock
		if (neededCountCheck != 1) {
			addError(i, "Wrong number of ';' and ':'");
			return;
		}
	}
	else if (neededCount == std::string::npos || neededCountCheck != neededCount + 1) {
		addError(i, "Wrong number of ';' and ':'");
		return;
	}

	// All good, can tokenize each elem individuall
	while (1) {
		if (groupStock.find(';') == std::string::npos) {
			createStock(groupStock.substr(0, groupStock.size()), tokenType, i);
			break;
		}
		createStock(groupStock.substr(0, groupStock.find(';')), tokenType, i);
		groupStock = groupStock.substr(groupStock.find(';') + 1, groupStock.size());
	}
}

bool Lexer::strIsInt(std::string str, size_t i) {
	if (str.size() == 0) {
		addError(i, "Empty number");
		return false;
	}
	for (size_t index = 0; index < str.size(); index++) {
		if (!std::isdigit(str[index])) {
			addError(i, "Must be a positive int");
			return false;
		}
	}
	return true;
}

bool Lexer::strIsAlNum(std::string str, size_t i) {
	if (str.size() == 0) {
		addError(i, "Empty name");
		return false;
	}
	if (str.compare("time") == 0 || str.compare("optimize") == 0) {
		addError(i, "Can not use 'time' or 'optimize' as name");
		return false;
	}
	for (size_t index = 0; index < str.size(); index++) {
		if (str[index] != '_' && !std::isalnum(str[index])) {
			addError(i, "Must be an alnum");
			return false;
		}
	}
	return true;
}

void Lexer::createStock(std::string str, TokenType stockType, size_t i) {
	std::string stockName = str.substr(0, str.find(':'));
	std::string stockQnt = str.substr(str.find(':') + 1, str.size());
	if (!strIsAlNum(stockName, i)) 
		return ;
	if (!strIsInt(stockQnt, i)) 
		return ;
	tokens.push_back(Token(stockType, stockName));
	tokens.push_back(Token(quantity, stockQnt));
}

void Lexer::addError(size_t index, std::string msg) {
	std::stringstream str;

	str << "Line " << index << ": " << msg;

	errors.push_back(str.str());
}

void Lexer::tokenizeOptimize(size_t i, std::string str) {
	if (str.find(':') == std::string::npos) {
		addError(i, "Missing ':'");
		return ;
	}
	std::string strOptimize = str.substr(0, str.find(':'));
	if (strOptimize.compare("optimize") != 0) {
		addError(i, "Missing 'optimize' keyword");
		return ;
	}
	std::string strOptimize = str.substr(str.find(':') + 1, str.size());
	if (strOptimize.size() < 3 && strOptimize.front() != '(' && strOptimize.back() != ')') {
		addError(i, "Missing 'optimize' parameters");
		return ;
	}

	std::string strOptimize = str.substr(1, str.size() - 1); // Get everything between '(' and ')'
	std::cout << "Check vals: " << strOptimize << std::endl;
	while (1) {
		if (strOptimize.find(';') == std::string::npos) {
			// createStock(strOptimize.substr(0, strOptimize.size()), optimize, i);
			break;
		}
		// createStock(strOptimize.substr(0, strOptimize.find(';')), optimize, i);
		strOptimize = strOptimize.substr(strOptimize.find(';') + 1, strOptimize.size());
	}
}