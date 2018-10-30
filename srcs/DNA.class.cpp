#include "DNA.class.hpp"
#include "Parser.class.hpp"

bool Gene::loopDirection = true;

Gene::Gene(int actualCycle, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock)
    : actualCycle(actualCycle), vProcess(vProcess), currentStock(currentStock) {

    std::vector<std::string> doableProcess = std::vector<std::string>();
    // std::map<std::string, int> tmpCurrentStock = currentStock;
	std::vector<Process> &vParserProcess = Parser::instance->getProcess();

    // for (auto process = Parser::instance->getProcess().begin(); process != Parser::instance->getProcess().end(); process++) {
	for (size_t i = 0; i < vParserProcess.size(); i++) {
		std::cout << "Name1: " << vParserProcess[i].name << std::endl;
		bool doable = true;
        for (auto needStock = vParserProcess[i].neededStock.begin(); needStock != vParserProcess[i].neededStock.end(); needStock++) {
            if (currentStock[needStock->first] < needStock->second) {
                doable = false;
                break ;
            }
            // TODO: update maxDoable
        }
        if (doable) // TODO: take random number between 1 and maxDoable
            doableProcess.push_back(vParserProcess[i].name);
    }
    doableProcessGene((rand() % doableProcess.size()), doableProcess);
}

void Gene::doableProcessGene(int index, std::vector<std::string> doableProcess) {
	// int doable;
	// int random;
    // if (Gene::loopDirection) {
        for (int i = index; i != index; --i) {
            if (i <= 0)
                i = doableProcess.size();
		// 	doable = doableProcessNbr(doableProcess[i], currentStock);
		// 	std::cout << doableProcess[i] << " : " << doable << std::endl;
		// 	if (doable > 0) {
		// 		random = rand() % doable;
		// 		for (int j = 0; j < random; j++) {
		// 			applyProcessToStock(doableProcess[i], &currentStock);
		// 		}
		// 	}
        }
        // Gene::loopDirection = false;
        // return ;
    // }
	// for (int i = index; i != index; --i) {
	// 	if (i <= 0)
	// 		i = doableProcess.size();
	// 	doable = doableProcessNbr(doableProcess[i], currentStock);
	// 	std::cout << doableProcess[i] << " : " << doable << std::endl;
	// 	if (doable > 0) {
	// 		random = rand() % doable;
	// 		for (int j = 0; j < random; j++) {
	// 			applyProcessToStock(doableProcess[i], &currentStock);
	// 		}
	// 	}
	// }
//     Gene::loopDirection = true;
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
	vGene.push_back(firstGene);
	evalFitness();
}

// void DNA::createGenesSequence(std::vector<Gene> startingGenes, std::map<std::string, int> startStock) {
//     // genes = startingGenes;
//     // currentStock = startStock;
// }

void DNA::evalFitness() {
	// std::cout << std::endl << "--- " << std::endl;
	fitness = 0;
	std::map<std::string, int> currentStock = vGene.back().currentStock;
	// Give lots of points if an optimize product is in stock
	// for (auto goal = Parser::instance->getGoal().begin(); goal != Parser::instance->getGoal().end(); goal++) {
	// 	if (currentStock[goal->name] > 0) {
	// 		auto it = Parser::instance->getStartStock().find(goal->name);
	// 		if (it != Parser::instance->getStartStock().end()) {
	// 			if (currentStock[goal->name] > it->second)
	// 				fitness += sqrt(currentStock[goal->name] - it->second) * 10;
	// 		}
	// 		else {
	// 			fitness += sqrt(currentStock[goal->name]) * 10; // give more points if goal is in stock 5 times and not only once, but with a slow curve
	// 		}
	// 	}
	// }
	// std::cout << "Score for optimize goals reached: " << fitness << std::endl;

	// TODO: Give some points if intermediary products are in stock (the closest the product is to the goal, the higher the points)
	// Create leaderboard for most wanted obj

	// Score based on current stock * rarity
	for (auto good = currentStock.begin(); good != currentStock.end(); good++) {
		// Skip good if dont have any or less than when started
		if (good->second <= Parser::instance->getStartStock()[good->first])
			continue;
		// Use sqrt so that producing 365 days is still less important than having one year
		fitness += sqrt(good->second) * Parser::instance->getWantedGoods()[good->first];
	}


	// TODO: Give some points for active process

	// TODO: Give some points if initial stock is here but not goals

	// TODO: Give max points if initial stock AND goals are here => we are in a positive loop!

	// Make fitness exponential so that little improvements weight more
	fitness = pow(fitness, 2);
	// std::cout << "Fitness " << fitness << std::endl;
}
