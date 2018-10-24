#include "Lexer.class.hpp"

void usage() {
	std::cout << "./krpsim file_path" << std::endl;
}

int main(int ac, char **av) {
	if (ac == 2)
	{
		Lexer toto = Lexer(av[1]);
		std::vector<std::string> &errors = toto.getErrors();
		std::vector<Token> &tokens = toto.getTokens();
		if (errors.size() != 0) {
			for (auto it = errors.begin(); it != errors.end(); it++) {
				std::cerr << (*it) << std::endl;
			}
			// for (auto it = tokens.begin(); it != tokens.end(); it++) {
			// 	std::cout << (*it).type << " " << (*it).info << std::endl;
			// }
			return 0;
		}
		// do parse
		for (auto it = tokens.begin(); it != tokens.end(); it++) {
			std::cout << (*it).type << " " << (*it).info << std::endl;
		}
	}
	else
		usage();
	return 0;
}
