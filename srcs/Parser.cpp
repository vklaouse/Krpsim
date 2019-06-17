#include "Parser.hpp"

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
    for (const auto &it : vProcess) {
        for (const auto &it2 : it.neededStock) {
            bool stockExist = false;
            for (const auto &it3 : vStock) {
                if (it3.name.compare(it2.first) == 0) {
                    stockExist = true;
                    break;
                }
            }
            if (!stockExist) {
                errors.push_back("Parser Error: No way to have at least one '" + it2.first + "' in '" + it.name + "' process !");
            }
        }
    }

	if (vGoal.empty())
		errors.push_back("Parser Error: Give us a Goal!");

	startStock = std::map<std::string, int>();
    for (auto it = vStock.begin(); it != vStock.end(); it++)
        startStock[it->name] = it->quantity;
	for (auto &goal : vGoal) {
		// Check every goals
		for (auto &stock : vStock) {
			// Search stock related to the goal
			if (goal.name == stock.name) {
				goal.stockToOptimize = &stock;
				prepareGraphe(stock, goal);
			}
		}
	}


	for (size_t i = 0; i < POPULATION_SIZE; i++) {
		vAgent.push_back(Agent(vStock, startStock));
	}
}

void Parser::prepareGraphe(Stock &stock, Goal &goal) {
	for (auto &neededProcess : stock.waysToProduce) {
		// search the name of the process who can product the stock
		for (auto &process : vProcess) {
			// search the prpocess who can product the stock
			if (neededProcess == process.name
					&& stock.processForProduce.find(process.name)
					== stock.processForProduce.end()) {
				if (process.resultStock.find(stock.name) != process.resultStock.end()
						&& process.neededStock.find(stock.name) != process.neededStock.end()
						&& process.resultStock[stock.name] <= process.neededStock[stock.name])
					continue ;
				stock.processForProduce[process.name] = &process;
				if (goal.possibleProcess.find(process.name) == goal.possibleProcess.end())
					goal.possibleProcess[process.name] = &process;
				for (auto &processNeeded : process.neededStock) {
					// check needed stock to execute the process
					for (auto &neededStock : vStock) {
						// search in vStock
						if (neededStock.name == processNeeded.first) {
							if (process.stockNeeded.find(neededStock.name)
									!= process.stockNeeded.end())
								return ; // if already exist, stop
							process.stockNeeded[neededStock.name] = &neededStock;
							prepareGraphe(neededStock, goal);
							break ;
						}
					}
				}
				break ;
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
    std::map<std::string, int> neededStock = std::map<std::string, int>();
    while (tokens[i].type == needed_stock) {
        stockName = tokens[i].info;
        ++i;
		if (saveStrInInt(tokens[i].info, &quantity) == false)
			return i;
        ++i;

        // Check that stock is not already inside
        if (neededStock.find(stockName) != neededStock.end()) {
            errors.push_back("Parser error: Stock with name '" + stockName + "' given twice in '" + name + "' process !");
            return i;
        }
        if (quantity == 0) // skip if stock is not actually involved in process
            continue;
        neededStock[stockName] = quantity;
    }

    std::map<std::string, int> resultStock = std::map<std::string, int>();
    while (tokens[i].type == result_stock) {
        stockName = tokens[i].info;
        ++i;
		if (saveStrInInt(tokens[i].info, &quantity) == false)
			return i;
        ++i;
        // Check that stock is not already inside
        if (resultStock.find(stockName) != resultStock.end()) {
            errors.push_back("Parser Error: Stock with name '" + stockName + "' produced twice in '" + name + "' process !");
            return i;
        }
        if (quantity == 0) // skip if stock is not actually involved in process
            continue;


        resultStock[stockName] = quantity;
    }

    // Check if it's a process that makes you loose stock
    bool isLosingProcess = true;
    for (const auto &rStock : resultStock) {
        if (neededStock.find(rStock.first) != neededStock.end()) {
            if (neededStock[rStock.first] < rStock.second) {
                isLosingProcess = false;
                break;
            }
        }
        else {
            // If result is not in neededStock, then procces is not a loosing one
            isLosingProcess = false;
            break;
        }
    }
    if (isLosingProcess) {
        return i;
    }

	int delay;
    // Skip process if it doesnt produce anything or if delay value has errors
	if (resultStock.size() != 0 && saveStrInInt(tokens[i].info, &delay)) {
        Process newProcess = Process(name, neededStock, resultStock, delay);
        vProcess.push_back(newProcess);

        for (auto it = newProcess.resultStock.begin(); it != newProcess.resultStock.end(); it++) {
            addProcessReferenceToStock(it->first, &newProcess);
        }
    }
    return i;
}


void Parser::addProcessReferenceToStock(std::string stockName, Process *newProcess) {
    for (auto it = vStock.begin(); it != vStock.end(); it++) {
        if (it->name.compare(stockName) == 0) {
            it->waysToProduce.push_back(newProcess->name);
            return;
        }
    }
    Stock newStock = Stock(stockName, 0);
    newStock.waysToProduce.push_back(newProcess->name);
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
    if (!stockExist)
        errors.push_back("Parser Error: No way to have at least one '" + tokens[i].info + "' in your stocks !");
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
    if (tmpVal == 0) {
		errors.push_back("Parser Error: Numbers must be strictly positives");
		return false;
	}
	return true;
}

void Parser::runSimlation(int lifeTime, bool verboseOption) {
    this->verboseOption = verboseOption;
	this->_lifeTime = lifeTime;
    if (verboseOption) {
        std::cerr << VERBOSE_SECTION_START << std::endl;
		describe();
		if (!vAgent.empty())
			vAgent[0].describe();
	}

	for (int i = 0; i < _lifeTime; i++) { // Cycle nb choose by user
		std::cout << "---------- " << i << " ----------" << std::endl;
		for (auto &goal : vGoal) { // search solution from goal
			for (auto &agent : vAgent) {
				agent.addNewStockFromEndedProcess();
				agent.produceStock(goal.stockToOptimize, i, "", goal.possibleProcess);
				vAgent[0].describe();

			}
		}
	}

    if (verboseOption)
        std::cerr << VERBOSE_SECTION_END << std::endl;
}

void Parser::describe() {

	std::cerr << "-------------- vStocks --------------" << std::endl;
	for (const auto &stock : vStock) {
		std::cerr << "Stock -> " << stock.name << " Quantity -> " << stock.quantity << std::endl;
		for (const auto &way : stock.waysToProduce) {
			std::cerr << "Way -> " << way << std::endl;
		}
		std::cerr << "---- Graphe ---- " << stock.processForProduce.size() << std::endl;
		for (const auto &way : stock.processForProduce) {
			std::cerr << "Graphe -> " << way.first << " == " << way.second->name << std::endl;
		}
		std::cerr << "-------------------------------------" << std::endl;
	}

	std::cerr << "-------------- Process --------------" << std::endl;
	for (const auto &process : vProcess) {
		std::cerr << "Process -> " << process.name << " Delay -> " << process.delay << std::endl;
		for (const auto &way : process.neededStock) {
			std::cerr << "Needed -> " << way.first << " " << way.second << std::endl;
		}
		for (const auto &way : process.resultStock) {
			std::cerr << "Result -> " << way.first << " " << way.second << std::endl;
		}
		std::cerr << "---- Graphe ---- " << process.stockNeeded.size() << std::endl;
		for (const auto &way : process.stockNeeded) {
			std::cerr << "Graphe -> " << way.first << " == " << way.second->name << std::endl;
		}
		std::cerr << "-------------------------------------" << std::endl;
	}

	std::cerr << "------------- vOptimize -------------" << std::endl;
	for (const auto &goal : vGoal) {
		std::cerr << "Goal -> " << goal.name << " Time -> " << goal.optimizeTime << std::endl;
		std::cerr << "---- Graphe ----" << std::endl;
		std::cerr << "Graphe -> " << goal.stockToOptimize->name << std::endl;
		std::cerr << "-------------------------------------" << std::endl;
	}

	std::cerr << "------------ StartStocks ------------" << std::endl;
	for (const auto &ss : startStock) {
		std::cerr << "Stock -> " << ss.first << " Quantity -> " << ss.second << std::endl;
	}
}
