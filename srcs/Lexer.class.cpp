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
	for (size_t i = 1; std::getline(*cFileStream, sLine); i++) {
		// std::cout << i << ": " << sLine << std::endl;
		if (sLine.front() == '#') {
			continue;
		}

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
				continue ;
			tokens.push_back(Token(operation, operationName));

			sLine = sLine.substr(sLine.find(':'), sLine.size());
			// if sLine.fir
			if (!tokenizeStock(&sLine, i, needed_stock))
				continue;

			if (!tokenizeStock(&sLine, i, result_stock))
				continue;

			// tokenize process delay
			if (sLine.front() != ':') {
				addError(i, "Invalid char after first ':'");
				continue;
			}
			sLine = sLine.substr(1, sLine.size());
			if (!strIsInt(sLine, i))
				continue ;
			tokens.push_back(Token(delay, sLine));
		}
		else if (tokenType == operation && separatorCount == 1) {
			// optimize
			tokenType = optimize;
			tokenizeOptimize(i, sLine);
		}
		else {
			std::string phase = (tokenType == stock) ? "stock" : (tokenType == operation) ? "operation" : "optimize";
			addError(i, "Error when fetching '" + phase + "' data");
		}
	}
}

Lexer::~Lexer() {
	cFileStream->close();
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
	else if (toParse->at(1) == ':') {
		*toParse = toParse->substr(1, toParse->size());
	}
	else {
		addError(i, "Wrong char after ':' char");
		return false;
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
			createStock(groupStock, tokenType, i);
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
	if (str.compare(TIME_KEYWORD) == 0 || str.compare(OPTIMIZE_KEYWORD) == 0) {
		addError(i, "Wrong use of '" + str + "' keyword");
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
	// std::cout << "======================================" << std::endl;
	if (str.find(':') == std::string::npos) {
		addError(i, "Missing ':'");
		return ;
	}
	std::string strOptimize = str.substr(0, str.find(':'));
	if (strOptimize.compare(OPTIMIZE_KEYWORD) != 0) {
		addError(i, std::string("Missing '") + OPTIMIZE_KEYWORD + "' keyword");
		return ;
	}
	strOptimize = str.substr(str.find(':') + 1, str.size());
	if (strOptimize.size() < 3 || strOptimize.front() != '(' || strOptimize.back() != ')') {
		addError(i, std::string("Missing '") + OPTIMIZE_KEYWORD + "' parameters");
		return ;
	}

	strOptimize = strOptimize.substr(1, strOptimize.size() - 2); // Get everything between '(' and ')'
	while (1) {
		if (strOptimize.find(';') == std::string::npos) {
			addOptimizeToken(strOptimize, i);
			break;
		}
		// createStock(strOptimize.substr(0, strOptimize.find(';')), optimize, i);
		addOptimizeToken(strOptimize.substr(0, strOptimize.find(';')), i);
		strOptimize = strOptimize.substr(strOptimize.find(';') + 1, strOptimize.size());
	}
}

void Lexer::addOptimizeToken(std::string str, size_t i) {
	if (str.size() == 0) {
		addError(i, "Empty name");
		return ;
	}
	if (str.compare(OPTIMIZE_KEYWORD) == 0) {
		addError(i, std::string("Can not use or '") + OPTIMIZE_KEYWORD + "' as name");
		return ;
	}
	for (size_t index = 0; index < str.size(); index++) {
		if (str[index] != '_' && !std::isalnum(str[index])) {
			addError(i, "Must be an alnum");
			return ;
		}
	}

	tokens.push_back(Token(optimize, str));
}
