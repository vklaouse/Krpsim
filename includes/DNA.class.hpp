#pragma once

#include "Krpsim.hpp"

struct Process;

struct Gene {
public:
    Gene() {};
    Gene(int actualCycle, int timeElapsed, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock, bool addProcess);
    ~Gene() {};

	bool applyProcessToStock(std::string name, std::map<std::string, int> * stock, int nbrOfApply);
	void refreshProcessDelay();
	void addNewStock(std::string processName);
	void description(bool hash = false) const;
    void updateHash();

    int actualCycle;
	int timeElapsed;
	std::string currentStockHash = "";
	std::string vProcessHash = "";
    std::map<std::string, std::vector<int> > vProcess;
    std::map<std::string, int> currentStock;
	std::map<std::string, int> initialStock;
};

class DNA {
public:
    DNA();
	DNA(DNA &first, DNA &second, int geneA, int geneB);
    ~DNA() {};

	int firstEndedProcess(std::map<std::string, std::vector<int> > vProcess);
    size_t getFitness() { return fitness; };
	size_t evalFitness();
	void description(bool hash = false);
	std::vector<Gene> & getGene() { return vGene; };
	std::vector<Gene> getGeneCpy() { return vGene; };
	std::vector<Gene> * getGenePtr() { return &vGene; };
	bool getHasSelfMaintainedProduction() { return hasSelfMaintainedProduction; };
    void justMutation(int index);
    void createFollowingGenes(int size, bool addProcess = true);
	static bool compareGenes(Gene &first, Gene &second);
	static bool compareCurrentStock(std::map<std::string, int> &first, std::map<std::string, int> &second);
	static bool compareCurrentProcess(std::map<std::string, std::vector<int>> first, std::map<std::string, std::vector<int>> second);
	void printSolution();

private:
	bool isSelfMaintained();

    size_t fitness;
	bool hasSelfMaintainedProduction;
	std::vector<std::string> vHash;
    std::vector<Gene> vGene;
};
