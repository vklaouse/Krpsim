#include "Parser.class.hpp"

Parser::Parser(std::vector<Token> &tokens) {
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].type == stock) {
            addToStock(tokens[i].info, atoi(tokens[i + 1].info.c_str()));
            ++i;
        }
        else if (tokens[i].type == operation) {
            addProcess(tokens, &i);
        }
        else if (tokens[i].type == optimize) {
            // TODO
        }
    }
    return ;
}

Parser::~Parser() {
    return ;
}

void Parser::addToStock(std::string name, int quantity) {
    for (auto it = vStock.begin(); it != vStock.end(); it++) {
        if (it->name.compare(name) == 0) {
            it->quantity += quantity;
            return ;
        }
    }
    vStock.push_back(Stock(name, quantity)); 
}

void Parser::addProcess(std::vector<Token> &tokens, size_t *i) {
    std::string name = tokens[*i].info;
    for (auto it = vProcess.begin(); it != vProcess.end(); it++) {
        if (it->name.compare(name) == 0) {
            throw new std::runtime_error("Process with name '" + name + "' given twice !");
        }
    }

    *i = *i + 1;
    std::string stockName;
    int quantity;
    std::vector<Stock> neededStock = std::vector<Stock>();
    while (tokens[*i].type == needed_stock) {
        stockName = tokens[*i].info;
        *i = *i + 1;
        quantity = atoi(tokens[*i].info.c_str());
        *i = *i + 1;

        // Check that stock is not already inside
        for (auto it = neededStock.begin(); it != neededStock.end(); it++) {
            if (it->name.compare(stockName) == 0) {
                throw new std::runtime_error("Stock with name '" + stockName + "' needed twice in '" + name + "' process !");
            }
        }
        if (quantity == 0) // skip if stock is not actually involved in process
            continue;

        neededStock.push_back(Stock(stockName, quantity));
    }

    std::vector<Stock> resultStock = std::vector<Stock>();
    while (tokens[*i].type == result_stock) {
        stockName = tokens[*i].info;
        *i = *i + 1;
        quantity = atoi(tokens[*i].info.c_str());
        *i = *i + 1;

        // Check that stock is not already inside
        for (auto it = resultStock.begin(); it != resultStock.end(); it++) {
            if (it->name.compare(stockName) == 0) {
                throw new std::runtime_error("Stock with name '" + stockName + "' produced twice in '" + name + "' process !");
            }
        }
        if (quantity == 0) // skip if stock is not actually involved in process
            continue;

        resultStock.push_back(Stock(stockName, quantity));
    }

    int delay = atoi(tokens[*i].info.c_str());
    // *i = *i + 1;
    vProcess.push_back(Process(name, neededStock, resultStock, delay)); 
}