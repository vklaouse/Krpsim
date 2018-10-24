#include "Lexer.class.hpp"

void usage() {
	std::cout << "./krpsim file_path" << std::endl;
}

int main(int ac, char **av) {
	if (ac == 2)
	{
		std::ifstream *cFileStream;
		cFileStream = new std::ifstream(av[1]);

		if (cFileStream->fail()) {
			std::cerr << "Wrong file path -> " << av[1] << std::endl;
		}
		else {
			Lexer toto = Lexer(cFileStream);
			// delete toto;
			(void)toto;
		}
		delete cFileStream;
	}
	else
		usage();
	return 0;
}
