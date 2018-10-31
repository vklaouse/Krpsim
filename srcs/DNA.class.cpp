#include "DNA.class.hpp"
#include "Parser.class.hpp"

bool Gene::loopDirection = true;

Gene::Gene(int actualCycle, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock)
    : actualCycle(actualCycle), vProcess(vProcess), currentStock(currentStock), mutating(true) {

    std::vector<Process> doableProcess = std::vector<Process>();
	std::vector<Process> &vParserProcess = Parser::instance->getProcess();

	if (actualCycle != 0) {
		refreshProcessDelay();
	}
	for (size_t i = 0; i < vParserProcess.size(); i++) {
		bool doable = true;
		for (const auto &needStock : vParserProcess[i].neededStock) {
			if (this->currentStock[needStock.first] < needStock.second) {
                doable = false;
                break ;
            }
        }
        if (doable)
            doableProcess.push_back(vParserProcess[i]);
    }
	if (doableProcess.size() > 0)
		doableProcessGene((Parser::instance->getRandom() % doableProcess.size()), doableProcess);
}

void Gene::refreshProcessDelay() {
	size_t count = 0;
	for (auto &process : vProcess) {
		count = 0;
		while (count <  process.second.size()) {
			if (process.second[count] > actualCycle) {
				break;
			}
			addNewStock(process.first);
			count++;
		}
		process.second.erase(process.second.begin(), process.second.begin() + count);
	}
}

void Gene::addNewStock(std::string processName) {
	for (const auto &process : Parser::instance->getProcess()) {
		if (process.name.compare(processName) == 0) {
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
	for (int i = index; 1; Gene::loopDirection ? i-- : i++) {
		doable = doableProcessNbr(doableProcess[i].name, currentStock);
		if (doable > 0) {
			// std::cout << "New rand: " << *myRandom << std::endl;
			random = Parser::instance->getRandom() % doable;
			// random = doable;
			std::vector<int> vDelay (random, doableProcess[i].delay + actualCycle);
			applyProcessToStock(doableProcess[i].name, &currentStock, random);
			vProcess[doableProcess[i].name].insert(vProcess[doableProcess[i].name].end(), vDelay.begin(), vDelay.end());
		}
		if (exitFromFor && index == i)
			break;
		exitFromFor = true;
		if (i <= 0 && Gene::loopDirection)
			i = doableProcess.size();
		if (i >= (int)doableProcess.size() - 1 && !Gene::loopDirection)
			i = -1;
	}
	Gene::loopDirection = !Gene::loopDirection;
}

bool Gene::applyProcessToStock(std::string name, std::map<std::string, int> * stock, int nbrOfApply) {
	for (const auto &process : Parser::instance->getProcess()) {
		if (process.name == name) {
			for (const auto &neededStock : process.neededStock) {
				stock->at(neededStock.first) -= neededStock.second * nbrOfApply;
				if (stock->at(neededStock.first) < 0) {
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
		if (!applyProcessToStock(name, &tmpCurrentStock, 1))
			break ;
		nbr += 1;
	}
	return nbr;
}

void Gene::description() const{
	std::cout << "ActualCycle : " << actualCycle << std::endl;
	std::cout << "Current active process : " << std::endl;
	for (const auto &process : vProcess) {
		if (process.second.size() > 0) {
			std::cout << "	Process name : " << process.first << " -> " << process.second.size() << std::endl;
			std::cout << "	Delay : [";
			for (const auto &delay : process.second) {
				std::cout << " " << delay << " ";
			}
			std::cout << "]\n" << std::endl;
		}
	}
	std::cout << "Current stock :" << std::endl;
	for (const auto &stock : currentStock) {
		std::cout << "	" <<  stock.first << " : " << stock.second << std::endl;
	}
}

DNA::DNA() : fitness(0), vGene (std::vector<Gene>()) {
	// std::cout << "y" << std::endl;
	vGene.push_back(Gene(0, std::map<std::string, std::vector<int> >(), Parser::instance->getStartStock()));

	for (int i = 0; 1;) {
		i = firstEndedProcess(vGene.back().vProcess);
		if (i == std::numeric_limits<int>::max() || vGene.size() == GEN_LENGTH) {
			break ;
		}
		vGene.push_back(Gene(i, vGene.back().vProcess, vGene.back().currentStock));
	}
	evalFitness();
}

int DNA::firstEndedProcess(std::map<std::string, std::vector<int> > vProcess) {
	int min = std::numeric_limits<int>::max();
	for (const auto &process : vProcess) {
		if (process.second.size() > 0)
			min = min < process.second[0] ? min : process.second[0];
	}
	return min;
}

void DNA::description() {
	std::cout << "*********************************************" << std::endl;
	for (const auto &gene : vGene) {
		gene.description();
		std::cout << "_____________________________________________" << std::endl;
	}
	std::cout << "DNA size      : " << vGene.size() << std::endl;
	std::cout << "Fitness value : " << fitness << std::endl;
}

bool DNA::compareGenes(Gene first, Gene second) {
	(void)first;
	(void)second;
	// for (const auto &f : first) {
	// 	for (const auto &s : second) {
	// 		Parser::map_compare(f.vProcess, s.vProcess);
	// 	}
	// }
	return true;
}

size_t DNA::evalFitness() {
	// std::cout << std::endl << "--- " << std::endl;
	fitness = 0;
	for (const auto &stock : Parser::instance->getWantedGoods()) {
	// for (const auto &stock : vGene.back().currentStock) {
		fitness += vGene.back().currentStock[stock.first] * stock.second;
	}
	// std::map<std::string, int> currentStock = vGene.back().currentStock;
	// Give lots of points if an optimize product is in stock

	// TODO: Give some points if intermediary products are in stock (the closest the product is to the goal, the higher the points)

	// Score based on current stock * rarity

	// TODO: Give some points for active process

	// TODO: Give some points if initial stock is here but not goals

	// TODO: Give max points if initial stock AND goals are here => we are in a positive loop!

	// Make fitness exponential so that little improvements weight more
	// fitness = pow(fitness, 2);
	// std::cout << "Fitness " << fitness << std::endl;

	return fitness;
}
