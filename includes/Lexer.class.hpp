#include <iostream>
#include <fstream>

class Lexer {

public:
	Lexer(std::ifstream *cFileStream);
	~Lexer();

private:
	// std::ifstream *_cFileStream;
};
