#include "Lexer.class.hpp"
#include "Parser.class.hpp"

int verifyLifeTime(std::string sLifeTime) {
	double tmpLifetime;

	for (size_t i = 0; sLifeTime.size() > i; i++) {
		if (isdigit(sLifeTime[i]) == false)
			return -1;
	}
	try {
		tmpLifetime = std::stod(sLifeTime);
	}
	catch (std::exception & e) {
		return -1;
	}
	if (tmpLifetime >= std::numeric_limits<int>::max() || tmpLifetime <= 0)
		return -1;
	return static_cast<int>(tmpLifetime);
}

void usage() {
	std::cerr << "./krpsim file_path lifetime" << std::endl;
}

int main(int ac, char **av) {
	if (ac == 3)
	{
		srand(clock());
		int lifeTime = verifyLifeTime(av[2]);
		if (lifeTime == -1) {
			std::cerr << "Lifetime must be positive int" << std::endl;
			usage();
			return 0;
		}
		Lexer tokenizer = Lexer(av[1]);
		std::vector<std::string> &errors = tokenizer.getErrors();
		std::vector<Token> &tokens = tokenizer.getTokens();
		if (errors.size() != 0) {
			for (auto it = errors.begin(); it != errors.end(); it++) {
				std::cerr << (*it) << std::endl;
			}
			return 0;
		}
		Parser parser = Parser(tokens);
		errors = parser.getErrors();
		if (errors.size() != 0) {
			for (auto it = errors.begin(); it != errors.end(); it++) {
				std::cerr << (*it) << std::endl;
			}
			return 0;
		}
		// std::cout << "=====Stock=====" << std::endl;
		// for (size_t i = 0; i < parser.getStock().size(); i++) {
		// 	std::cout << "Name : " << parser.getStock()[i].name << " -> Quantity : " << parser.getStock()[i].quantity << std::endl;
		// }
		// std::cout << "=====Process=====" << std::endl;
		// for (size_t i = 0; i < parser.getProcess().size(); i++) {
		// 	std::cout << "Name : " << parser.getProcess()[i].name << " -> Delay : " << parser.getProcess()[i].delay << std::endl;
		// }
		// std::cout << "=====Goals=====" << std::endl;
		// for (size_t i = 0; i < parser.getGoal().size(); i++) {
		// 	std::cout << "Name : " << parser.getGoal()[i].name << " -> Optimize time : " << parser.getGoal()[i].optimizeTime << std::endl;
		// }
		parser.runSimlation(lifeTime);
	}
	else {
		std::cerr << "Wrong number of parameters" << std::endl;
		usage();
	}
	return 0;
}
