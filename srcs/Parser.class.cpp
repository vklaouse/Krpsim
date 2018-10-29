#include "Parser.class.hpp"

Parser::Parser(std::vector<Token> &tokens) {
	int quantity;
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].type == stock) {
			if (saveStrInInt(tokens[i + 1].info, &quantity) == false)
				continue;
            addToStock(tokens[i].info, quantity);
            ++i;
        }
        else if (tokens[i].type == operation) {
            i = addProcess(tokens, i);
        }
        else if (tokens[i].type == optimize) {
		    i = addGoal(tokens, i);
        }
    }

    // that process required stock can actually be produced
    for (auto it = vProcess.begin(); it != vProcess.end(); it++) {
        for (auto it2 = it->neededStock.begin(); it2 != it->neededStock.end(); it2++) {
            bool stockExist = false;
            for (auto it3 = vStock.begin(); it3 != vStock.end(); it3++) {
                if (it3->name.compare(it2->name) == 0) {
                    stockExist = true;
                    break;
                }
            }
            if (!stockExist) {
                errors.push_back("Parser Error: No way to have at least one '" + it2->name + "' in '" + it->name + "' process !");
            }
        }
    }
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

size_t Parser::addProcess(std::vector<Token> &tokens, size_t i) {
    std::string name = tokens[i].info;
    for (auto it = vProcess.begin(); it != vProcess.end(); it++) {
        if (it->name.compare(name) == 0) {
            errors.push_back("Parser Error: Process with name '" + name + "' given twice !");
			return i;
        }
    }

    ++i;
    std::string stockName;
    int quantity;
    std::vector<Stock> neededStock = std::vector<Stock>();
    while (tokens[i].type == needed_stock) {
        stockName = tokens[i].info;
        ++i;
		if (saveStrInInt(tokens[i].info, &quantity) == false)
			return i;
        ++i;

        // Check that stock is not already inside
        for (auto it = neededStock.begin(); it != neededStock.end(); it++) {
            if (it->name.compare(stockName) == 0) {
                errors.push_back("Parser error: Stock with name '" + stockName + "' given twice in '" + name + "' process !");
				return i;
            }
        }
        if (quantity == 0) // skip if stock is not actually involved in process
            continue;

        neededStock.push_back(Stock(stockName, quantity));
    }

    std::vector<Stock> resultStock = std::vector<Stock>();
    while (tokens[i].type == result_stock) {
        stockName = tokens[i].info;
        ++i;
		if (saveStrInInt(tokens[i].info, &quantity) == false)
			return i;
        ++i;

        // Check that stock is not already inside
        for (auto it = resultStock.begin(); it != resultStock.end(); it++) {
            if (it->name.compare(stockName) == 0) {
                errors.push_back("Parser Error: Stock with name '" + stockName + "' produced twice in '" + name + "' process !");
				return i;
            }
        }
        if (quantity == 0) // skip if stock is not actually involved in process
            continue;

        resultStock.push_back(Stock(stockName, quantity));
    }

	int delay;
	if (saveStrInInt(tokens[i].info, &delay)) {
        Process newProcess = Process(name, neededStock, resultStock, delay);
        vProcess.push_back(newProcess);

        for (auto it = newProcess.resultStock.begin(); it != newProcess.resultStock.end(); it++) {
            addProcessReferenceToStock(it->name, &newProcess);
        }
    }
    return i;
}


void Parser::addProcessReferenceToStock(std::string stockName, Process *newProcess) {
    for (auto it = vStock.begin(); it != vStock.end(); it++) {
        if (it->name.compare(stockName) == 0) {
            it->waysToProduce.push_back(newProcess);
            return;
        }
    }
    Stock newStock = Stock(stockName, 0);
    newStock.waysToProduce.push_back(newProcess);
    vStock.push_back(newStock);
}

size_t Parser::addGoal(std::vector<Token> &tokens, size_t i) {
	bool optimizeTime = tokens[i].info.compare(TIME_KEYWORD) == 0;

    if (optimizeTime) {
        ++i;
        if (i == tokens.size()) {
            errors.push_back(std::string("Parser Error: Cannot end optimize params with '") + TIME_KEYWORD + "' keyword !");
            return i;
        }
        if (tokens[i].info.compare(TIME_KEYWORD) == 0) {
            errors.push_back(std::string("Parser Error: '") + TIME_KEYWORD + "' keyword cannot be followed by another '" + TIME_KEYWORD + "' !");
            return i;
        }
    }

    // Check that stock is not already inside optimize
    for (auto it = vGoal.begin(); it != vGoal.end(); it++) {
        if (it->name.compare(tokens[i].info) == 0) {
            errors.push_back("Parser Error: '" + tokens[i].info + "' stock given twice in optimize !");
            return i;
        }
    }

    // Check if goal exist
    bool stockExist = false;
    for (auto it = vStock.begin(); it != vStock.end(); it++) {
        if (it->name.compare(tokens[i].info) == 0) {
            stockExist = true;
            break;
        }
    }
    if (!stockExist) {
        errors.push_back("Parser Error: No way to have at least one '" + tokens[i].info + "' in your stocks !");
    }
    else
        vGoal.push_back(Goal(tokens[i].info, optimizeTime));
    return i;
}

bool Parser::saveStrInInt(std::string &str, int *myInt) {
	double tmpVal;
	try {
	 	tmpVal = std::stod(str);
	}
	catch (std::exception & e) {
		errors.push_back("Parser Error: Overflow detected (" + str + ")");
		return false;
	}
	if (tmpVal >= std::numeric_limits<int>::max()) {
		errors.push_back("Parser Error: Overflow detected (" + str + ")");
		return false;
	}

	*myInt = static_cast<int>(tmpVal);
	return true;
}
