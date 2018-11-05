#include "Lexer.class.hpp"
#include "LexerVerif.class.hpp"
#include "Parser.class.hpp"
#include "ParserVerif.class.hpp"

bool printErrors(std::vector<std::string> &errors) {
	if (errors.size() != 0) {
		for (auto it = errors.begin(); it != errors.end(); it++) {
			std::cerr << (*it) << std::endl;
		}
		return true;
	}
	return false;
}

void usage() {
	std::cerr << "./krpsim_verif file_path result_to_test" << std::endl;
}

int main(int ac, char **av) {
	if (ac == 3)
	{
		Lexer tokenizer = Lexer(av[1]);
		std::vector<std::string> &errors = tokenizer.getErrors();
		std::vector<Token> &tokens = tokenizer.getTokens();
		if (printErrors(errors))
			return 0;
		Parser parser = Parser(tokens);
		errors = parser.getErrors();
		if (printErrors(errors))
			return 0;

		std::cout << "-----------" << std::endl;

		LexerVerif lexerVerif = LexerVerif(av[2]);
		errors = lexerVerif.getErrors();
		tokens = lexerVerif.getTokens();
		if (printErrors(errors))
			return 0;
		ParserVerif parserVerif = ParserVerif(tokens);
		errors = parserVerif.getErrors();
		if (printErrors(errors))
			return 0;
		parserVerif.checker();

	}
	else {
		std::cerr << "Wrong number of parameters" << std::endl;
		usage();
	}
	return 0;
}
