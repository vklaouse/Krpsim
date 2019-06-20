#pragma once

#include "Krpsim.hpp"
#include "Agent.hpp"

#define POPULATION_SIZE 10
#define GENERATION_NBR 10

#define VERBOSE_SECTION_START "-------------- Verbose --------------"
#define VERBOSE_SECTION_END "------------ End Verbose ------------"

class Agent;
struct Process;

struct Stock {
public:
    Stock(std::string name, int quantity) : name(name), quantity(quantity) {};
    ~Stock() {};
    std::string name;
    int quantity;
	std::vector<std::string> waysToProduce;
    std::map<std::string, Process *> processForProduce;
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
	std::map<std::string, Stock *> stockNeeded;
};

struct Goal {
public:
	Goal(std::string name, bool optimizeTime)
		: name(name), optimizeTime(optimizeTime) {} ;
	~Goal() {};
	std::string name;
	bool optimizeTime;
	Stock *stockToOptimize;
	std::map<std::string, Process *> possibleProcess;
};

class Parser {

public:
	static Parser *instance;

	Parser() {};
    Parser(std::vector<Token> &tokens);
    ~Parser();

    std::vector<Stock> getStock() { return vStock; };
    std::vector<Process> &getProcess() { return vProcess; };
    std::vector<Goal> &getGoal() { return vGoal; };
	std::vector<std::string> &getErrors() { return errors; };
	std::map<std::string, int> getStartStock() { return startStock; };

    void runSimlation(int lifeTime, bool verboseOption);

protected:
	void describe();
	void prepareGraphe(Stock &stock, Goal &goal);
    void addToStock(std::string name, int quantity);
    size_t addProcess(std::vector<Token> &tokens, size_t i);
    void addProcessReferenceToStock(std::string stockName, Process*newProcess);
    size_t addGoal(std::vector<Token> &tokens, size_t i);
	bool saveStrInInt(std::string &str, int *myInt);
	void eval(Agent &bestAgent);
	void writeSolution(Agent &agent);


    bool verboseOption;

    std::vector<Stock> vStock;
    std::vector<Process> vProcess;
    std::vector<Goal> vGoal;
	std::vector<Agent> vAgent;
	std::vector<std::string> errors;

    std::map<std::string, int> startStock;

	int bestVal = 0;
	int bestCycle = 2147483647;

	int _lifeTime;
};
