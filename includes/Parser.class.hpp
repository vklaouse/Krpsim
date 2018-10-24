#pragma once

#include "Krpsim.hpp"

class Stock {

public:
    Stock(std::string name, int quantity) : name(name), quantity(quantity) {};
    ~Stock() {};
    std::string name;
    int quantity;

};

class Process {

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

class Parser {

public:
    Parser(std::vector<Token> &tokens);
    ~Parser();
    std::vector<Stock> getStock() { return vStock; };
    std::vector<Process> getProcess() { return vProcess; };
    
    void addToStock(std::string name, int quantity);

private:
    std::vector<Stock> vStock;
    std::vector<Process> vProcess;

};
