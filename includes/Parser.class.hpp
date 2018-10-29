#pragma once

#include <stdexcept>
#include "Krpsim.hpp"

#define GEN_SIZE 3
#define GEN_LENGTH 4

struct Process;

struct Stock {

public:
    Stock(std::string name, int quantity) : name(name), quantity(quantity) { std::vector<Process*> waysToProduce = std::vector<Process*>(); };
    ~Stock() {};
    std::string name;
    int quantity;
    std::vector<Process*> waysToProduce;
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
};

struct Goal {
public:
	Goal(std::string name, bool optimizeTime) : name(name), optimizeTime(optimizeTime) {} ;
	~Goal() {};

	std::string name;
	bool optimizeTime;
};

struct ProcessInfo {
public:
    ProcessInfo() {};
    ProcessInfo(int multiplier, int delay) : multiplier(multiplier), delay(delay) {};
    ~ProcessInfo() {};
    int multiplier;
    int delay;
};

struct CycleSnapshot {
public:
	CycleSnapshot(int actualCycle, std::map<std::string, ProcessInfo> vProcess, std::map<std::string, int> vStock)
     : actualCycle(actualCycle), activeProcess(vProcess), currentStock(vStock) {} ;
	~CycleSnapshot() {};
    int actualCycle;
    std::map<std::string, ProcessInfo> activeProcess;
    std::map<std::string, int> currentStock;
    CycleSnapshot *prev;  
};

class Parser {

public:
    Parser(std::vector<Token> &tokens);
    ~Parser();

    std::vector<Stock> getStock() { return vStock; };
    std::vector<Process> getProcess() { return vProcess; };
    std::vector<Goal> getGoal() { return vGoal; };
	std::vector<std::string> &getErrors() { return errors; };

    void runSimlation(int lifeTime);

private:
    void addToStock(std::string name, int quantity);
    size_t addProcess(std::vector<Token> &tokens, size_t i);
    void addProcessReferenceToStock(std::string stockName, Process*newProcess);
    size_t addGoal(std::vector<Token> &tokens, size_t i);
	bool saveStrInInt(std::string &str, int *myInt);
    void createFirstGen();

    std::vector<Stock> vStock;
    std::vector<Process> vProcess;
    std::vector<Goal> vGoal;
	std::vector<std::string> errors;

    std::vector<CycleSnapshot> actualGen;
    std::vector<CycleSnapshot> childGen;

};
