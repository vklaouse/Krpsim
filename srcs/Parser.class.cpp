#include "Parser.class.hpp"

Parser::Parser(std::vector<Token> &tokens) {
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].type == stock) {
            addToStock(tokens[i].info, atoi(tokens[i + 1].info.c_str()));
            ++i;
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