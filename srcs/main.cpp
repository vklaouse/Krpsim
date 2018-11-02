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
		// TODO:: fix size of DNA that is always 100
		// TODO:: fix infinite loop woth benefice in pomme
		// TODO: do not allow process that counter one another
		// TODO: no crossover on gene 0
		// TODO: detect infinite loop in recre -> get best loop
		// TODO: better eval of prcess score
		parser.runSimlation(lifeTime);
	}
	else {
		std::cerr << "Wrong number of parameters" << std::endl;
		usage();
	}
	return 0;
}
