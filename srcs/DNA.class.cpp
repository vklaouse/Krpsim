#include "DNA.class.hpp"
#include "Parser.class.hpp"

bool Gene::loopDirection = true;

Gene::Gene(int actualCycle, int timeElapsed, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock)
    : actualCycle(actualCycle), timeElapsed(timeElapsed), vProcess(vProcess), currentStock(currentStock) {

    std::vector<Process> doableProcess = std::vector<Process>();
	std::vector<Process> &vParserProcess = Parser::instance->getProcess();

	if (actualCycle != 0) {
		std::cout << actualCycle << std::endl;
		// refreshProcessDelay();
	}

	for (size_t i = 0; i < vParserProcess.size(); i++) {
		bool doable = true;
        for (auto needStock = vParserProcess[i].neededStock.begin(); needStock != vParserProcess[i].neededStock.end(); needStock++) {
            if (currentStock[needStock->first] < needStock->second) {
                doable = false;
                break ;
            }
        }
        if (doable)
            doableProcess.push_back(vParserProcess[i]);
    }
	std::cout << doableProcess.size() << std::endl;
    doableProcessGene((rand() % doableProcess.size()), doableProcess);
}

void Gene::refreshProcessDelay() {
	for (auto &process : vProcess) {
		for (size_t i = 0; i < process.second.size(); i++) {
			process.second[i] -= timeElapsed;
			if (process.second[i] == 0) {
				addNewStock(process.first);
				process.second.erase(process.second.begin() + i);
			}
		}
		if (process.second.size() == 0)
			std::cout << process.first << "  is empty" << std::endl;
			// process.erase(process.first.begin() + i);
	}
}

void Gene::addNewStock(std::string processName) {
	for (const auto &process : Parser::instance->getProcess()) {
		if (process.name == processName) {
			for (const auto &resStock : process.resultStock) {
				currentStock[resStock.first] += resStock.second;
			}
			break ;
		}
	}
}

void Gene::doableProcessGene(int index, std::vector<Process> &doableProcess) {
	int doable;
	int random;
	bool exitFromFor = false;
    if (Gene::loopDirection) {
        for (int i = index; 1; i--) {
			exitFromFor = true;
			doable = doableProcessNbr(doableProcess[i].name, currentStock);
			if (doable > 0) {
				random = rand() % doable;
				std::vector<int> vDelay = std::vector<int>();
				for (int j = 0; j <= random; j++) {
					applyProcessToStock(doableProcess[i].name, &currentStock);
					vDelay.push_back(doableProcess[i].delay);
				}
				vProcess[doableProcess[i].name] = vDelay;
			}
			if (exitFromFor && index == i)
				break;
			if (i <= 0)
                i = doableProcess.size();
        }
        Gene::loopDirection = false;
        return ;
    }
	for (int i = index; 1; i++) {
		exitFromFor = true;
		doable = doableProcessNbr(doableProcess[i].name, currentStock);
		if (doable > 0) {
			random = rand() % doable;
			std::vector<int> vDelay = std::vector<int>();
			for (int j = 0; j <= random; j++) {
				applyProcessToStock(doableProcess[i].name, &currentStock);
				vDelay.push_back(doableProcess[i].delay);
			}
			vProcess[doableProcess[i].name] = vDelay;
		}
		if (exitFromFor && index == i)
			break;
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
	vGene.push_back(Gene(0, 0, std::map<std::string, std::vector<int> >(), Parser::instance->getStartStock()));
	int timeElapsed = 0;
	for (int i = 0; 1;) {
		timeElapsed = firstEndedProcess(vGene.back().vProcess);
		i += timeElapsed;
		if (i == std::numeric_limits<int>::max())
			break ;
		vGene.push_back(Gene(i, timeElapsed, vGene.back().vProcess, vGene.back().currentStock));
	}
	evalFitness();
}

int DNA::firstEndedProcess(std::map<std::string, std::vector<int> > vProcess) {
	int min = std::numeric_limits<int>::max();
	for (const auto &process : vProcess) {
		for (const auto &delay : process.second) {
			min = min < delay ? min : delay;
		}
	}
	return min;
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
