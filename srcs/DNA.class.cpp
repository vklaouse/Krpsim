#include "DNA.class.hpp"
#include "Parser.class.hpp"

bool Gene::loopDirection = true;

Gene::Gene(int actualCycle, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock)
    : actualCycle(actualCycle), vProcess(vProcess), currentStock(currentStock) {

    std::vector<std::string> doableProcess = std::vector<std::string>();
	std::vector<Process> &vParserProcess = Parser::instance->getProcess();

	for (size_t i = 0; i < vParserProcess.size(); i++) {
		bool doable = true;
        for (auto needStock = vParserProcess[i].neededStock.begin(); needStock != vParserProcess[i].neededStock.end(); needStock++) {
            if (currentStock[needStock->first] < needStock->second) {
                doable = false;
                break ;
            }
        }
        if (doable)
            doableProcess.push_back(vParserProcess[i].name);
    }
    doableProcessGene((rand() % doableProcess.size()), doableProcess);
}

void Gene::doableProcessGene(int index, std::vector<std::string> &doableProcess) {
	int doable;
	int random;
	bool exitFromFor = false;
    if (Gene::loopDirection) {
        for (int i = index; 1; i--) {
			if (exitFromFor && index == i)
				break;
			exitFromFor = true;
			doable = doableProcessNbr(doableProcess[i], currentStock);
			if (doable > 0) {
				random = rand() % doable;
				for (int j = 0; j < random; j++) {
					applyProcessToStock(doableProcess[i], &currentStock);
				}
			}
			if (i <= 0)
                i = doableProcess.size();
        }
        Gene::loopDirection = false;
        return ;
    }
	for (int i = index; 1; i++) {
		if (exitFromFor && index == i)
			break;
		exitFromFor = true;
		doable = doableProcessNbr(doableProcess[i], currentStock);
		if (doable > 0) {
			random = rand() % doable;
			for (int j = 0; j < random; j++) {
				applyProcessToStock(doableProcess[i], &currentStock);
			}
		}
		if (i >= (int)doableProcess.size() - 1)
			i = -1;
	}
    Gene::loopDirection = true;
}

bool Gene::applyProcessToStock(std::string name, std::map<std::string, int> * stock) {
	for (auto process = Parser::instance->getProcess().begin(); process != Parser::instance->getProcess().end(); process++) {
		if (process->name == name) {
			for (auto neededStock = process->neededStock.begin(); neededStock != process->neededStock.end(); neededStock++) {
				stock->at(neededStock->first) -= neededStock->second;
				if (stock->at(neededStock->first) < 0) {
					return false;
				}
			}
			break ;
		}
	}
	return true;
}

int Gene::doableProcessNbr(std::string name, std::map<std::string, int> tmpCurrentStock) {
	int nbr = 0;
	while (1) {
		if (!applyProcessToStock(name, &tmpCurrentStock))
			break ;
		nbr += 1;
	}
	return nbr;
}

DNA::DNA() : fitness(0), vGene (std::vector<Gene>()) {
	Gene firstGene = Gene(0, std::map<std::string, std::vector<int> >(), Parser::instance->getStartStock());
}

// void DNA::createGenesSequence(std::vector<Gene> startingGenes, std::map<std::string, int> startStock) {
//     // genes = startingGenes;
//     // currentStock = startStock;
// }

void DNA::evalFitness() {

}
