#include "Lexer.class.hpp"
#include "Parser.class.hpp"

void usage() {
	std::cout << "./krpsim file_path" << std::endl;
}

int main(int ac, char **av) {
	if (ac == 2)
	{
		Lexer tokenizer = Lexer(av[1]);
		std::vector<std::string> &errors = tokenizer.getErrors();
		std::vector<Token> &tokens = tokenizer.getTokens();
		if (errors.size() != 0) {
			for (auto it = errors.begin(); it != errors.end(); it++) {
				std::cerr << (*it) << std::endl;
			}
			return 0;
		}
		// do parse
		// for (auto it = tokens.begin(); it != tokens.end(); it++) {
		// 	std::cout << (*it).type << " " << (*it).info << std::endl;
		// }
		Parser parser = Parser(tokens);
		std::cout << "=====Stock=====" << std::endl;
		for (size_t i = 0; i < parser.getStock().size(); i++) {
			std::cout << "Name : " << parser.getStock()[i].name << " -> Quantity : " << parser.getStock()[i].quantity << std::endl;
		}
		std::cout << "=====Process=====" << std::endl;
		for (size_t i = 0; i < parser.getProcess().size(); i++) {
			std::cout << "Name : " << parser.getProcess()[i].name << " -> Delay : " << parser.getProcess()[i].delay << std::endl;
		}
	}
	else
		usage();
	return 0;
}
