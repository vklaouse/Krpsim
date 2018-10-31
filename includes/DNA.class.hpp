#pragma once

#include "Krpsim.hpp"

struct Process;

struct Gene {
public:
    static bool loopDirection;
    Gene(int actualCycle, int timeElapsed, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock, bool endDNA);
    ~Gene() {};

    void doableProcessGene(int index, std::vector<Process> &doableProcess);
	int doableProcessNbr(std::string name, std::map<std::string, int> tmpCurrentStock);
	bool applyProcessToStock(std::string name, std::map<std::string, int> * stock);
	void refreshProcessDelay();
	void addNewStock(std::string processName);
	void description() const;

    int actualCycle;
	int timeElapsed;
    std::map<std::string, std::vector<int> > vProcess;
    std::map<std::string, int> currentStock;
	bool endDNA;
	bool mutating;
};

class DNA {
public:
    DNA();
    ~DNA() {};

	int firstEndedProcess(std::map<std::string, std::vector<int> > vProcess);
    size_t getFitness() { return fitness; };
	size_t evalFitness();
	void description();
	std::vector<Gene> & getGene() { return vGene; };
	static bool compareGenes(Gene first, Gene second);

private:
    size_t fitness;
    std::vector<Gene> vGene;
};
