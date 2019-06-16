#include "LexerVerif.class.hpp"

LexerVerif::LexerVerif(char *filePath) {
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
		else if ((tokenType == operation || tokenType == stock) && separatorCount >= 2) {
			// operation
			tokenType = operation;
			std::string operationCycle = sLine.substr(0, sLine.find(':'));
			if (!strIsInt(operationCycle, i))
				continue ;
			tokens.push_back(Token(cycle, operationCycle));

			sLine = sLine.substr(sLine.find(':'), sLine.size());
			if (!tokenizeOperation(&sLine, i, operation))
				continue;
		}
		else if ((tokenType == operation || tokenType == stock) && separatorCount == 1) {
			// operation
			tokenType = operation;
			std::string operationCycle = sLine.substr(0, sLine.find(':'));
			if (!strIsInt(operationCycle, i))
				continue ;
			tokens.push_back(Token(cycle, operationCycle));

			sLine = sLine.substr(sLine.find(':'), sLine.size());
			if (!tokenizeOperation(&sLine, i, operation))
				continue;
		}
		else {
			std::string phase = (tokenType == stock) ? "stock" : (tokenType == operation) ? "operation" : "optimize";
			addError(i, "Error when fetching '" + phase + "' data");
		}
		// for (const auto &t : tokens) {
		// 	if (t.info.size() == 0)
		// 		addError(0, "Error lexer");
		// }
	}
}

bool LexerVerif::tokenizeOperation(std::string *toParse, size_t i, TokenType tokenType) {
	std::string delimiter = ":";
	size_t pos = 0;
	std::string token = "t";
	toParse->erase(0, pos + delimiter.length());
	while ((pos = toParse->find(delimiter)) != std::string::npos) {
		if (token.compare("") == 0) {
			addError(i, "Empty operation name");
			return false;
		}
	    token = toParse->substr(0, pos);
		if (strIsAlNum(token, i) && token.compare("") != 0)
			tokens.push_back(Token(tokenType, token));
		else
			addError(i, "Bad operation name");
	    toParse->erase(0, pos + delimiter.length());
	}
	if (toParse->size() > 0) {
		addError(i, "Wrong format");
		return false;
	}
	return true;
}
