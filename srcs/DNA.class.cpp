#include "DNA.class.hpp"
#include "Parser.class.hpp"

Gene::Gene(int actualCycle, int timeElapsed, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock, bool addProcess)
    : actualCycle(actualCycle), timeElapsed(timeElapsed), vProcess(vProcess), currentStock(currentStock), mutating(true) {

    std::vector<Process> doableProcess = std::vector<Process>();
	std::vector<Process> &vParserProcess = Parser::instance->getProcess();

	if (actualCycle != 0) {
		refreshProcessDelay();
	}
	initialStock = this->currentStock;
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
					// std::cerr << process.name << " -> " << z++ << ", Doable -> " << maxDoable << ", Random -> " << random << std::endl;
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
	std::cout << "ActualCycle : " << actualCycle << std::endl;
	std::cout << "TimeElapsed : " << timeElapsed << std::endl;
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
	if (hash)
		std::cout << "Process hash : " << vProcessHash << std::endl;
	std::cout << "Initial stock :" << std::endl;
	for (const auto &stock : initialStock) {
		std::cout << "	" <<  stock.first << " : " << stock.second << std::endl;
	}
	std::cout << "Current stock :" << std::endl;
	for (const auto &stock : currentStock) {
		std::cout << "	" <<  stock.first << " : " << stock.second << std::endl;
	}
	if (hash)
		std::cout << "Stock hash : " << currentStockHash << std::endl;
	std::cout << "_____________________________________________" << std::endl;
}

DNA::DNA() : fitness(0), vGene (std::vector<Gene>()) {
	vGene.push_back(Gene(0, 0, std::map<std::string, std::vector<int> >(), Parser::instance->getStartStock(), true));
	int timeElapsed;
	for (int i = 0; 1;) {
		timeElapsed = firstEndedProcess(vGene.back().vProcess);
		i += timeElapsed;
		if (timeElapsed == std::numeric_limits<int>::max() || vGene.size() == DNA_SIZE) {
			break ;
		}
		vGene.push_back(Gene(i, timeElapsed, vGene.back().vProcess, vGene.back().currentStock, true));
		if (std::find(vHash.begin(), vHash.end(), vGene.back().currentStockHash + vGene.back().vProcessHash) != vHash.end()) {
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
	}
	Gene tmpGeneA = first.getGeneCpy()[geneA];
	int i;
	for (const auto &process : tmpGeneA.vProcess) {
		for (i = 0; i < (int)process.second.size(); i++) {
			if (process.second[i] < 0)
				break ;
		}
		if (i < (int)process.second.size())
			tmpGeneA.vProcess[process.first].erase(tmpGeneA.vProcess[process.first].begin() + i, tmpGeneA.vProcess[process.first].end());
	}

	std::vector<Gene> vGeneB = second.getGeneCpy();
	Gene tmpGeneB = vGeneB[geneB];
	for (const auto &process : tmpGeneB.vProcess) {
		i = 0;
		for (; i < (int)process.second.size(); i++) {
			if (process.second[i] < 0)
				break ;
		}
		if (i > 0)
			tmpGeneB.vProcess[process.first].erase(tmpGeneB.vProcess[process.first].begin(), tmpGeneB.vProcess[process.first].begin() + i);
	}

	for (auto processA : tmpGeneA.vProcess) {
		tmpGeneB.vProcess[processA.first].insert(tmpGeneB.vProcess[processA.first].begin(), processA.second.begin(), processA.second.end());
	}
	tmpGeneB.actualCycle = tmpGeneA.actualCycle;
	tmpGeneB.initialStock = tmpGeneA.initialStock;
	tmpGeneB.currentStock = tmpGeneA.initialStock;
	for (auto process : tmpGeneB.vProcess) {
		int count = 0;
		for (i = 0; i < (int)process.second.size(); i++) {
			if (process.second[i] < 0)
				count++;
		}
		if (count > 0)
			tmpGeneB.applyProcessToStock(process.first, &(tmpGeneB.currentStock), count);
	}


	// 2) Add create genes fro
	tmpGeneB.updateHash();
	vGene.push_back(tmpGeneB);
	int timeElapsed;
	int actualCycle = tmpGeneB.actualCycle;
	int originelTimeElapsed = -1;

	for (size_t i = geneB + 1; i < vGeneB.size(); i++) {
		timeElapsed = firstEndedProcess(vGene.back().vProcess);
		if (timeElapsed == std::numeric_limits<int>::max())
			break ;
		actualCycle += timeElapsed;
		if (i < second.getGene().size() - 1 && second.getGene()[i + 1].timeElapsed == originelTimeElapsed) {
			originelTimeElapsed = -1;
			tmpGeneB = vGeneB[i];
			tmpGeneB.actualCycle = actualCycle;
			tmpGeneB.timeElapsed = timeElapsed;
			tmpGeneB.vProcess = vGene.back().vProcess;
			tmpGeneB.currentStock = vGene.back().currentStock;
			tmpGeneB.refreshProcessDelay();
			tmpGeneB.updateHash();
			vGene.push_back(tmpGeneB);
		}
		else if (i < second.getGene().size() - 1 && second.getGene()[i + 1].timeElapsed > timeElapsed) {
			originelTimeElapsed  = second.getGene()[i + 1].timeElapsed;
			vGene.push_back(Gene(actualCycle, timeElapsed, vGene.back().vProcess, vGene.back().currentStock, false));
			i--;
		}
		else {
			tmpGeneB = vGeneB[i];
			tmpGeneB.actualCycle = actualCycle;
			tmpGeneB.timeElapsed = timeElapsed;
			tmpGeneB.vProcess = vGene.back().vProcess;
			tmpGeneB.currentStock = vGene.back().currentStock;
			tmpGeneB.refreshProcessDelay();
			tmpGeneB.updateHash();
			vGene.push_back(tmpGeneB);
		}
		// At each gene, check if we dont need to put some of the excessProcess (or even create new Gene if delay is shorter)

		// vGene.back().description();
	}
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

void DNA::description(bool hash) {
	std::cout << "*********************************************" << std::endl;
	for (const auto &gene : vGene) {
		gene.description(hash);
	}
	std::cout << "DNA size      : " << vGene.size() << std::endl;
	std::cout << "Fitness value : " << fitness << std::endl;
}

bool DNA::compareGenes(Gene &first, Gene &second) {
	if (!DNA::compareCurrentStock(first.initialStock, second.initialStock))
		return false;
	if (!DNA::compareCurrentProcess(first.vProcess, second.vProcess))
		return false;
	return true;
}

bool DNA::compareCurrentProcess(std::map<std::string, std::vector<int>> first, std::map<std::string, std::vector<int>> second) {
	for (const auto &f : first) {
		for (size_t i = 0; i < f.second.size(); i++) {
			if (second[f.first].size() > i && f.second[i] != second[f.first][i] && f.second[i] > 0 && second[f.first][i] > 0)
				return false;
		}
	}
	return true;
}

bool DNA::compareCurrentStock(std::map<std::string, int> &first, std::map<std::string, int> &second) {
	for (const auto &f : first) {
		if (f.second > second[f.first])
			return false;
	}
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
