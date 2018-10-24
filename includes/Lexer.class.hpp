#pragma once

#include "Krpsim.hpp"

class Lexer {

public:
	Lexer(char *filePath);
	~Lexer();

	std::vector<std::string> &getErrors() { return errors; };
	std::vector<Token> &getTokens() { return tokens; };

private:
	void createStock(std::string str, TokenType tokenType, size_t i);
	bool tokenizeStock(std::string *toParse, size_t i, TokenType tokenType);
	void tokenizeGroupStock(size_t i, std::string groupStock, TokenType tokenType);
	void addError(size_t index, std::string msg);
	bool strIsInt(std::string str, size_t i);
	bool strIsAlNum(std::string str, size_t i);
	void addOptimizeToken(std::string str, size_t i);
	void tokenizeOptimize(size_t i, std::string str);

	// bool succes;
	std::ifstream *cFileStream;
	std::vector<Token> tokens;
	std::vector<std::string> errors;
};
