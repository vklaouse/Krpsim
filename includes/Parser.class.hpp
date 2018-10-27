#pragma once

#include <stdexcept>
#include "Krpsim.hpp"

struct Stock {

public:
    Stock(std::string name, int quantity) : name(name), quantity(quantity) {};
    ~Stock() {};
    std::string name;
    int quantity;

};

struct Process {

public:
    Process(std::string name, std::vector<Stock> neededStock, std::vector<Stock> resultStock, int delay) :
        name(name), neededStock(neededStock), resultStock(resultStock), delay(delay), activeProcess(std::vector<int>()), priority(0)
        {};
    ~Process() {};
    std::string name;
    std::vector<Stock> neededStock;
    std::vector<Stock> resultStock;
    int delay;
    std::vector<int> activeProcess;
    int priority;

};

struct Goal {
public:
	Goal(std::string name, bool optimizeTime) : name(name), optimizeTime(optimizeTime) {} ;
	~Goal() {};

	std::string name;
	bool optimizeTime;
};

class Parser {

public:
    Parser(std::vector<Token> &tokens);
    ~Parser();

    std::vector<Stock> getStock() { return vStock; };
    std::vector<Process> getProcess() { return vProcess; };
    std::vector<Goal> getGoal() { return vGoal; };
	std::vector<std::string> &getErrors() { return errors; };

private:
    void addToStock(std::string name, int quantity);
    void addProcess(std::vector<Token> &tokens, size_t *i);
    void addGoal(std::vector<Token> &tokens, size_t i);
	bool saveStrInInt(std::string &str, int *myInt);

    std::vector<Stock> vStock;
    std::vector<Process> vProcess;
    std::vector<Goal> vGoal;
	std::vector<std::string> errors;

};
