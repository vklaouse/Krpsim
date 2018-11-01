#pragma once

#include "Krpsim.hpp"

struct Process;

struct Gene {
public:
	Gene() {};
    Gene(int actualCycle, int timeElapsed, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock);
    ~Gene() {};

	bool applyProcessToStock(std::string name, std::map<std::string, int> * stock, int nbrOfApply);
	void refreshProcessDelay();
	void addNewStock(std::string processName);
	void description() const;

    int actualCycle;
	int timeElapsed;
	std::string currentStockHash = "";
	std::string vProcessHash = "";
    std::map<std::string, std::vector<int> > vProcess;
    std::map<std::string, int> currentStock;
	bool mutating;
	std::map<std::string, int> initialStock;
};

class DNA {
public:
    DNA();
	DNA(std::vector<Gene> vGene, size_t join);
    ~DNA() {};

	int firstEndedProcess(std::map<std::string, std::vector<int> > vProcess);
    size_t getFitness() { return fitness; };
	size_t evalFitness();
	void description();
	std::vector<Gene> & getGene() { return vGene; };
	std::vector<Gene> getGeneCpy() { return vGene; };
	static bool compareGenes(Gene &first, Gene &second);
	static bool compareCurrentStock(std::map<std::string, int> &first, std::map<std::string, int> &second);
	static bool compareCurrentProcess(std::map<std::string, std::vector<int>> first, std::map<std::string, std::vector<int>> second);

private:
    size_t fitness;
	std::vector<std::string> vHash;
    std::vector<Gene> vGene;
};
