#pragma once

#include "Krpsim.hpp"


struct Gene {
public:
    static bool loopDirection;
    Gene(int actualCycle, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock);
    ~Gene() {};

    void doableProcessGene(int index, std::vector<std::string> doableProcess);
	int doableProcessNbr(std::string name, std::map<std::string, int> tmpCurrentStock);
	bool applyProcessToStock(std::string name, std::map<std::string, int> * stock);

    int actualCycle;
    std::map<std::string, std::vector<int> > vProcess;
    std::map<std::string, int> currentStock;
};

class DNA {
public:
    DNA();
    // DNA(std::vector<ProcessInfo> genes, std::map<std::string, int> currentStock, int fitness)
    //     : genes(genes), currentStock(currentStock), fitness(fitness) {};
    ~DNA() {};

    int getFitness() { return fitness; };
	void evalFitness();
    // void createGenesSequence(std::vector<Gene> startingGenes, std::map<std::string, int> startStock);

private:
    int fitness;
    std::vector<Gene> vGene;
};
