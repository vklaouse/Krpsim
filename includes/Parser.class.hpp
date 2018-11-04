#pragma once

#include "Krpsim.hpp"
#include "DNA.class.hpp"

#define POPULATION_SIZE 50
#define DNA_SIZE 500

struct Process;

struct Stock {

public:
    Stock(std::string name, int quantity) : name(name), quantity(quantity) { std::vector<Process*> waysToProduce = std::vector<Process*>(); };
    ~Stock() {};
    std::string name;
    int quantity;
    std::vector<std::string> waysToProduce;
};

struct Process {

public:
    Process(std::string name, std::map<std::string, int> neededStock, std::map<std::string, int> resultStock, int delay) :
        name(name), neededStock(neededStock), resultStock(resultStock), delay(delay)
        {};
    ~Process() {};
    std::string name;
    std::map<std::string, int> neededStock;
    std::map<std::string, int> resultStock;
    int delay;
    size_t score;
};

struct Goal {
public:
	Goal(std::string name, bool optimizeTime) : name(name), optimizeTime(optimizeTime) {} ;
	~Goal() {};

	std::string name;
	bool optimizeTime;
};

struct GoodInfo {
public:
    GoodInfo(std::string name, int timesNeededByHigherStock)
        : name(name), timesNeededByHigherStock(timesNeededByHigherStock), timesNeededByLowerStock(0),
        timesNeededByTierStock(0), avgDelay(0) {};
    ~GoodInfo() {};
    std::string name;
    int timesNeededByHigherStock;
    int timesNeededByLowerStock;
    int timesNeededByTierStock;
    int avgDelay;
};

class Parser {

public:
    static Parser *instance;

	Parser() {};
    Parser(std::vector<Token> &tokens);
    ~Parser();

    std::vector<Stock> getStock() { return vStock; };
    std::vector<Process> &getProcess() { return vProcess; };
    std::vector<Goal> getGoal() { return vGoal; };
	std::vector<std::string> &getErrors() { return errors; };
	std::map<std::string, int> getStartStock() { return startStock; };
	std::map<std::string, size_t> &getWantedGoods() { return wantedGoods; };
	std::vector<std::vector<GoodInfo> > &getTierGoods() { return goodsTiers; };
    // size_t &getRandom() { myRandom = myRandom * 3 + 1; return (size_t)(rand()); }
    size_t getRandom() { return (size_t)(rand()); }

    void runSimlation(int lifeTime);

protected:
    void addToStock(std::string name, int quantity);
    size_t addProcess(std::vector<Token> &tokens, size_t i);
    void addProcessReferenceToStock(std::string stockName, Process*newProcess);
    size_t addGoal(std::vector<Token> &tokens, size_t i);
	bool saveStrInInt(std::string &str, int *myInt);
    void createFirstGen();
    void createGoodsLeaderboard();
    void setProcessScores();
    static bool sortProcessFunction(Process const& lhs, Process const& rhs);
    size_t getProcessScore(Process const& process);
	void crossOver(size_t totalFit);
    void mutation();
	void compareDNAForCrossOver(DNA &first, DNA &second, std::map<int, std::vector<int>> *possibleCrossOver);

    std::vector<Stock> vStock;
    std::vector<Process> vProcess;
    std::vector<Goal> vGoal;
	std::vector<std::string> errors;

    std::map<std::string, int> startStock;
    std::vector<std::vector<GoodInfo> > goodsTiers;
	std::map<std::string, size_t> wantedGoods;
    std::vector<DNA> actualGen;
    std::vector<DNA> childGen;

    size_t myRandom;
};
