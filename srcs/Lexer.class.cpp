#include "Lexer.class.hpp"

Lexer::Lexer(std::ifstream *cFileStream) {


	std::string sLine;
	int i = 0;

	while (std::getline(*cFileStream, sLine)) {
		std::cout << sLine << std::endl;
		i++;
	}

}

Lexer::~Lexer() {
}
