#pragma once

#include "Krpsim.hpp"

struct Process;

struct Gene {
public:
    Gene(int actualCycle, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock);
    ~Gene() {};

    void doableProcessGene(std::vector<Process> &doableProcess);
	int doableProcessNbr(std::string name, std::map<std::string, int> tmpCurrentStock);
	bool applyProcessToStock(std::string name, std::map<std::string, int> * stock, int nbrOfApply);
	void refreshProcessDelay();
	void addNewStock(std::string processName);
	void description() const;

    int actualCycle;
    std::map<std::string, std::vector<int> > vProcess;
    std::map<std::string, int> currentStock;
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
	static bool compareCurrentStock(std::map<std::string, int> first, std::map<std::string, int> second);
	static bool compareCurrentProcess(std::map<std::string, std::vector<int>> first, std::map<std::string, std::vector<int>> second);

private:
    size_t fitness;
    std::vector<Gene> vGene;
};
