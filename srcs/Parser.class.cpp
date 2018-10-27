#include "Parser.class.hpp"

Parser::Parser(std::vector<Token> &tokens) {
	int quantity;
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].type == stock) {
			if (saveStrInInt(tokens[i].info, &quantity) == false)
				continue;
            addToStock(tokens[i].info, quantity);
            ++i;
        }
        else if (tokens[i].type == operation) {
            addProcess(tokens, &i);
        }
        else if (tokens[i].type == optimize) {
            // TODO
		    addGoal(tokens, i);
			i = tokens.size();
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
            errors.push_back("Parser Error: Process with name '" + name + "' given twice !");
			return;
        }
    }

    *i = *i + 1;
    std::string stockName;
    int quantity;
    std::vector<Stock> neededStock = std::vector<Stock>();
    while (tokens[*i].type == needed_stock) {
        stockName = tokens[*i].info;
        *i = *i + 1;
		if (saveStrInInt(tokens[*i].info, &quantity) == false)
			return;
        *i = *i + 1;

        // Check that stock is not already inside
        for (auto it = neededStock.begin(); it != neededStock.end(); it++) {
            if (it->name.compare(stockName) == 0) {
                errors.push_back("Parser error: Stock with name '" + stockName + "' given twice in '" + name + "' process !");
				return;
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
		if (saveStrInInt(tokens[*i].info, &quantity) == false)
			return;
        *i = *i + 1;

        // Check that stock is not already inside
        for (auto it = resultStock.begin(); it != resultStock.end(); it++) {
            if (it->name.compare(stockName) == 0) {
                errors.push_back("Parser Error: Stock with name '" + stockName + "' produced twice in '" + name + "' process !");
				return;
            }
        }
        if (quantity == 0) // skip if stock is not actually involved in process
            continue;

        resultStock.push_back(Stock(stockName, quantity));
    }

	int delay;
	if (saveStrInInt(tokens[*i].info, &delay) == false)
		return;
    // *i = *i + 1;
    vProcess.push_back(Process(name, neededStock, resultStock, delay));
}

void Parser::addGoal(std::vector<Token> &tokens, size_t i) {
	bool optimizeTime = false;
	while (i < tokens.size()) {
		std::cout << "CHECK: " << tokens[i].info << std::endl;
		if (tokens[i].info.compare(TIME_KEYWORD) == 0) {
			if (optimizeTime) {
                errors.push_back(std::string("Parser Error: '") + TIME_KEYWORD + "' keyword cannot be followed by another '" + TIME_KEYWORD + "' !");
				return;
			}
			optimizeTime = true;
		}
		else {
	        // Check that stock is not already inside
	        for (auto it = vGoal.begin(); it != vGoal.end(); it++) {
	            if (it->name.compare(tokens[i].info) == 0) {
	                errors.push_back("Parser Error: '" + tokens[i].info + "' stock given twice in optimize !");
					return;
	            }
	        }
			vGoal.push_back(Goal(tokens[i].info, optimizeTime));
			optimizeTime = false;
		}
		++i;
	}
	if (optimizeTime) {
		errors.push_back(std::string("Parser Error: Cannot end optimize params with '") + TIME_KEYWORD + "' keyword !");
		return;
	}
	vGoal.push_back(Goal(tokens[i - 1].info, optimizeTime));
}

bool Parser::saveStrInInt(std::string &str, int *myInt) {
	double tmpVal;
	try {
	 	tmpVal = std::stod(str);
	}
	catch (std::exception & e) {
		errors.push_back("Parser Error: Overflow detected");
		return false;
	}
	if (tmpVal >= std::numeric_limits<int>::max()) {
		errors.push_back("Parser Error: Overflow detected");
		return false;
	}

	*myInt = static_cast<int>(tmpVal);
	return true;
}
