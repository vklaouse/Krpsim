#pragma once

#include "Krpsim.hpp"
#include "Lexer.hpp"

class LexerVerif : public Lexer {

public:
	LexerVerif(char *filePath);
	~LexerVerif() {};

private:
	bool tokenizeOperation(std::string *toParse, size_t i, TokenType tokenType);

};
