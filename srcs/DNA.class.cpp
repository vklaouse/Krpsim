#include "DNA.class.hpp"
#include "Parser.class.hpp"

Gene::Gene(int actualCycle, int timeElapsed, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock, bool addProcess)
    : actualCycle(actualCycle), timeElapsed(timeElapsed), vProcess(vProcess), currentStock(currentStock) {

    std::vector<Process> doableProcess = std::vector<Process>();
	std::vector<Process> &vParserProcess = Parser::instance->getProcess();

	initialStock = this->currentStock;
	if (actualCycle != 0) {
		refreshProcessDelay();
		initialStock = this->currentStock;

		// Check if gene is end of DNA
		bool hasSelfMaintainedProduction = true;
		for (const auto stock : this->initialStock) {
			if (stock.second < Parser::instance->getStartStock()[stock.first]) {
				hasSelfMaintainedProduction = false;
				break;
			}
		}
		// Check that all goals have been produced at least once
		if (hasSelfMaintainedProduction) {
			for (const auto &goal : Parser::instance->getGoal()) {
				if (this->initialStock[goal.name] <= 0) {
					hasSelfMaintainedProduction = false;
					break;
				}
			}
			if (hasSelfMaintainedProduction) {
				addProcess = false;
			}
		}
	}
	if (addProcess) {
		size_t maxDoable;
		size_t tmpDoable;
		size_t random;
		bool firstCheck;
		bool canAddProcess = false;
		bool hasAddedProcess = false;
		do {
			for (const auto &process : vParserProcess) {
				firstCheck = true;
				for (const auto &needStock : process.neededStock) {
					tmpDoable = this->currentStock[needStock.first] / needStock.second;
					if (firstCheck || tmpDoable < maxDoable) {
						firstCheck = false;
						maxDoable = tmpDoable;
						if (maxDoable == 0)
							break;
					}
				}
				if (maxDoable > 0) {
					canAddProcess = true;
					random = Parser::instance->getRandom() % (maxDoable + 1);
					if (random > 0) {
						hasAddedProcess = true;
						std::vector<int> vDelay (random, (process.delay * -1));
						applyProcessToStock(process.name, &(this->currentStock), random);
						if (actualCycle != 0)
							this->vProcess[process.name].insert(this->vProcess[process.name].end(), vDelay.begin(), vDelay.end());
						else
							this->vProcess[process.name] = vDelay;
					}
				}
			}
		} while (canAddProcess && !hasAddedProcess);
	}
	updateHash();
}

void Gene::updateHash() {
	for (const auto &CS : this->currentStock) {
		currentStockHash += CS.first + std::to_string(CS.second);
	}
	for (const auto &process : this->vProcess) {
		if (process.second.size() > 0) {
			vProcessHash += process.first;
			vProcessHash += ":";
			for (const auto &delay : process.second) {
				vProcessHash += std::to_string(delay) + ";";
			}
		}
	}
}

void Gene::refreshProcessDelay() {
	size_t count = 0;
	for (auto &process : vProcess) {
		count = 0;
		for (size_t i = 0; i < process.second.size(); i++) {
			if (process.second[i] < 0)
				process.second[i] *= -1;
			process.second[i] -= timeElapsed;
			if (process.second[i] == 0) {
				count++;
			}
		}

		if (count > 0) {
			for (const auto &processInfo : Parser::instance->getProcess()) {
				if (processInfo.name.compare(process.first) == 0) {
					for (const auto &resStock : processInfo.resultStock) {
						currentStock[resStock.first] += resStock.second * count;
					}
					break ;
				}
			}

			vProcess[process.first].erase(vProcess[process.first].begin(), vProcess[process.first].begin() + count);
		}
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

void Gene::description(bool hash) const{
	std::cerr << "ActualCycle : " << actualCycle << std::endl;
	std::cerr << "TimeElapsed : " << timeElapsed << std::endl;
	std::cerr << "Current active process : " << std::endl;
	for (const auto &process : vProcess) {
		if (process.second.size() > 0) {
			std::cerr << "	Process name : " << process.first << " -> " << process.second.size() << std::endl;
			std::cerr << "	Delay : [";
			for (const auto &delay : process.second) {
				std::cerr << " " << delay << " ";
			}
			std::cerr << "]\n" << std::endl;
		}
	}
	if (hash)
		std::cerr << "Process hash : " << vProcessHash << std::endl;
	std::cerr << "Initial stock :" << std::endl;
	for (const auto &stock : initialStock) {
		std::cerr << "	" <<  stock.first << " : " << stock.second << std::endl;
	}
	std::cerr << "Current stock :" << std::endl;
	for (const auto &stock : currentStock) {
		std::cerr << "	" <<  stock.first << " : " << stock.second << std::endl;
	}
	if (hash)
		std::cerr << "Stock hash : " << currentStockHash << std::endl;
	std::cerr << "_____________________________________________" << std::endl;
}

DNA::DNA() : fitness(0), mutatingNbr(0), vGene (std::vector<Gene>()) {
	vGene.push_back(Gene(0, 0, std::map<std::string, std::vector<int> >(), Parser::instance->getStartStock(), true));
	createFollowingGenes(DNA_SIZE);
}

void DNA::createFollowingGenes(int size) {
	int timeElapsed;
	int i = vGene.back().actualCycle;
	while (true) {
		timeElapsed = firstEndedProcess(vGene.back().vProcess);
		i += timeElapsed;

		if (timeElapsed == std::numeric_limits<int>::max() || vGene.size() == (size_t)size) {
			isSelfMaintained();
			break ;
		}
		Gene newGene(i, timeElapsed, vGene.back().vProcess, vGene.back().currentStock, true);
		vGene.push_back(newGene);
		if (std::find(vHash.begin(), vHash.end(), vGene.back().currentStockHash + vGene.back().vProcessHash) != vHash.end()) {
			vGene.pop_back();
			break ;
		}
		vHash.push_back(vGene.back().currentStockHash + vGene.back().vProcessHash);
	}
	// description();
	evalFitness();
}

DNA::DNA(DNA &first, DNA &second, int geneA, int geneB) : fitness(0), vGene(std::vector<Gene>()) {
	for (int i = 0; i < geneA; i++) {
		vGene.push_back(first.getGeneCpy()[i]);
		vHash.push_back(vGene.back().currentStockHash + vGene.back().vProcessHash);
	}
	std::vector<Gene> vGeneB = second.getGeneCpy();
	Gene tmpGeneA = first.getGeneCpy()[geneA];
	Gene tmpGeneB = vGeneB[geneB];

	// Find excess stock from geneA
    std::map<std::string, int> excessStock;
	for (auto const &aStock : tmpGeneA.initialStock) {
		if (tmpGeneB.initialStock.find(aStock.first) == tmpGeneB.initialStock.end()) {
			excessStock[aStock.first] += aStock.second;
		}
		else {
			excessStock[aStock.first] += aStock.second - tmpGeneB.initialStock[aStock.first];
		}
	}


	tmpGeneB.actualCycle = tmpGeneA.actualCycle;
	tmpGeneB.initialStock = tmpGeneA.initialStock;
	for (auto const &excess : excessStock) {
		tmpGeneB.currentStock[excess.first] += excess.second;
	}

	tmpGeneB.updateHash();
	vGene.push_back(tmpGeneB);
	vHash.push_back(vGene.back().currentStockHash + vGene.back().vProcessHash);

	// 2) Add create genes from parent B
	int timeElapsed;
	int actualCycle = tmpGeneB.actualCycle;
	for (size_t i = geneB + 1; i < vGeneB.size(); i++) {
		timeElapsed = firstEndedProcess(vGene.back().vProcess);
		if (timeElapsed == std::numeric_limits<int>::max())
			break ;
		actualCycle += timeElapsed;

		tmpGeneB = vGeneB[i];
		tmpGeneB.actualCycle = actualCycle;
		tmpGeneB.timeElapsed = timeElapsed;


		for (auto const &excess : excessStock) {
			tmpGeneB.currentStock[excess.first] += excess.second;
			tmpGeneB.initialStock[excess.first] += excess.second;
		}

		tmpGeneB.updateHash();
		vGene.push_back(tmpGeneB);
		vHash.push_back(vGene.back().currentStockHash + vGene.back().vProcessHash);
	}
	evalFitness();
}

int DNA::firstEndedProcess(std::map<std::string, std::vector<int> > vProcess) {
	int min = std::numeric_limits<int>::max();
	for (auto &process : vProcess) {
		if (process.second.size() > 0 ) {
			if (process.second[0] < 0)
				process.second[0] *= -1;
			min = min < process.second[0] ? min : process.second[0];
		}
	}
	return min;
}

void DNA::justMutation(int idx) {
	if ((size_t)idx >= vGene.size()) {
        return ;
    }
	vGene.erase(vGene.begin() + idx, vGene.end());
	vHash.erase(vHash.begin() + idx, vHash.end());
	int size = vGene.size();
	if (idx == 0) {
		vGene.push_back(Gene(0, 0, std::map<std::string, std::vector<int> >(), Parser::instance->getStartStock(), true));
	}
	createFollowingGenes(vGene.size() + size);
	evalFitness();
}

void DNA::description(bool hash) {
	std::cerr << "*********************************************" << std::endl;
	for (const auto &gene : vGene) {
		gene.description(hash);
	}
	std::cerr << "DNA size      : " << vGene.size() << std::endl;
	std::cerr << "Fitness value : " << fitness << std::endl;
	std::cerr << "Is self maintained : " << hasSelfMaintainedProduction << std::endl;
}

void DNA::printSolution() {
	std::cout << "# Stock" << std::endl;
	for (const auto &s : vGene.back().currentStock) {
		std::cout << s.first << ":" << s.second << std::endl;
	}
	std::cout << "# Main Walk" << std::endl;
	for (const auto &s : vGene) {
		if (s.vProcess.size()) {
			std::cout << s.actualCycle << ":";
			for (const auto &sr : s.vProcess) {
				// int cnt = 0;
				for (const auto &srx : sr.second) {
					if (srx < 0)
						std::cout << sr.first << ":";
						// cnt++;
				}
				// if (cnt > 0)
				// 	std::cout << sr.first << ":" << cnt << ";";
			}
			std::cout << std::endl;
		}
	}
	
}


bool DNA::compareGenes(Gene &first, Gene &second) {
	if (!DNA::compareCurrentStock(first.initialStock, second.initialStock))
		return false;
	if (!DNA::compareCurrentProcess(first.vProcess, second.vProcess)) {
		return false;
	}
	return true;
}

bool DNA::compareCurrentProcess(std::map<std::string, std::vector<int>> first, std::map<std::string, std::vector<int>> second) {
	size_t i;
	for (auto f : first) {
		for (i = 0; i < f.second.size(); i++) {
			if (f.second[i] < 0)
				break ;
		}
		first[f.first].erase(first[f.first].begin() + i, first[f.first].end());
	}

	for (auto f : second) {
		for (i = 0; i < f.second.size(); i++) {
			if (f.second[i] < 0)
				break ;
		}
		second[f.first].erase(second[f.first].begin() + i, second[f.first].end());
	}

	for (const auto &f : first) {
		if (f.second.size() == 0) {
			if (second.find(f.first) != second.end() && second[f.first].size() != 0)
				return false;
		}
		else if (second.find(f.first) == second.end() || f.second != second[f.first])
			return false;
	}
	return true;
}

bool DNA::compareCurrentStock(std::map<std::string, int> &first, std::map<std::string, int> &second) {
	for (const auto &f : first) {
		if (f.second < second[f.first])
			return false;
	}
	return true;
}

size_t DNA::evalFitness() {
	fitness = 0;
	for (const auto &optimizeGood : Parser::instance->getTierGoods()[0]) {
		for (const auto &stock : vGene.back().currentStock) {
			if (optimizeGood.name.compare(stock.first) == 0)
				fitness += pow(10, Parser::instance->getTierGoods().size() + 1) * stock.second;
		}
	}
	// Give lots of points if an optimize product is in stock

	// TODO: Give some points if intermediary products are in stock (the closest the product is to the goal, the higher the points)

	// Score based on current stock * rarity

	// TODO: Give some points for active process

	for (const auto &startedProcess : vGene.back().vProcess) {
		for (const auto &process : Parser::instance->getProcess()) {
			if (process.name.compare(startedProcess.first) == 0) {
				fitness += process.score * startedProcess.second.size();
			}
		}
	}

	// TODO: Give some points if initial stock is here but not goals

	// TODO: Give max points if initial stock AND goals are here => we are in a positive loop!

	// Make fitness exponential so that little improvements weight more
	// fitness = pow(fitness, 2);
	// std::cout << "Fitness " << fitness << std::endl;

	// Give lots of point if selfMantained cycle has been found
	if (hasSelfMaintainedProduction) {
		fitness += pow(fitness, 2);
	}

	return fitness;
}

bool DNA::isSelfMaintained() {
	hasSelfMaintainedProduction = true;
	for (const auto stock : vGene.back().initialStock) {
		if (stock.second < vGene.front().initialStock[stock.first]) {
			hasSelfMaintainedProduction = false;
			break;
		}
	}
	// Check that all goals have been produced at least once
	if (hasSelfMaintainedProduction) {
		for (const auto &goal : Parser::instance->getGoal()) {
			if (vGene.back().initialStock[goal.name] <= 0) {
				hasSelfMaintainedProduction = false;
				break;
			}
		}
	}
	return hasSelfMaintainedProduction;
}