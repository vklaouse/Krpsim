#include "Lexer.class.hpp"
#include "Parser.class.hpp"

bool printErrors(std::vector<std::string> &errors) {
	if (errors.size() != 0) {
		for (auto it = errors.begin(); it != errors.end(); it++) {
			std::cerr << (*it) << std::endl;
		}
		return true;
	}
	return false;
}

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
	std::cerr << "./krpsim [-v] file_path lifetime" << std::endl;
}

int main(int ac, char **av) {
	if (ac == 3 || ac == 4)
	{
		bool verboseOption;
		if (ac == 4) {
			std::cout << av[1] << std::endl;
			if (std::strcmp(av[1], "-v") == 0)
				verboseOption = true;
			else {
				std::cerr << "Invalid option given !" << std::endl;
				usage();
				return 0;
			}
		}
		else
			verboseOption = false;

		srand(clock());
		int lifeTime = verifyLifeTime(av[ac - 1]);
		if (lifeTime == -1) {
			std::cerr << "Lifetime must be positive int" << std::endl;
			usage();
			return 0;
		}
		Lexer tokenizer = Lexer(av[ac - 2]);
		std::vector<std::string> &errors = tokenizer.getErrors();
		std::vector<Token> &tokens = tokenizer.getTokens();
		if (printErrors(errors))
			return 0;
		Parser parser = Parser(tokens);
		errors = parser.getErrors();
		if (printErrors(errors))
			return 0;
		// TODO: fix infinite loop with increasing starting stock in recre
		// TODO: fix infinite loop with increasing starting stock in pomme
		// TODO: do not allow process that counter one another
		// TODO: detect infinite loop in recre -> get best loop
		// TODO: better eval of prcess score
		// TODO: size of DNA must never be greater than ActualCycle
		// TODO: optimize for 'time' keyword 
		// TODO: pomme without any euros at start
		parser.runSimlation(lifeTime, verboseOption);
	}
	else {
		std::cerr << "Wrong number of parameters" << std::endl;
		usage();
	}
	return 0;
}
