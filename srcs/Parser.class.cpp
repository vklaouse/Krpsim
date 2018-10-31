#include "Parser.class.hpp"

Parser *Parser::instance;

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
                if (it3->name.compare(it2->first) == 0) {
                    stockExist = true;
                    break;
                }
            }
            if (!stockExist) {
                errors.push_back("Parser Error: No way to have at least one '" + it2->first + "' in '" + it->name + "' process !");
            }
        }
    }

    if (errors.size() == 0) {
        Parser::instance = this;
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

void Parser::runSimlation(int lifeTime) {
    clock_t killTime = clock() + (lifeTime * CLOCKS_PER_SEC);
    size_t i = 0;

    // createGoodsLeaderboard();
    createGoodsLeaderboard2();

    // Find best path using genetic algo
    // 1) Create Initial population
    createFirstGen();
    while (clock() < killTime) {
        // 2) Rank solutions

        // 2.1) Keep best

        // 3) Cross Over
        // 3bis) Mutation

        // 4) Rank new childs
        // 4.1) Keep best childs and add to 2.1
    }

    // Print best path
    std::cout << i << std::endl;
}

void Parser::createFirstGen() {

    startStock = std::map<std::string, int>();

    for (auto it = vStock.begin(); it != vStock.end(); it++) {
        // if (it->quantity > 0) {
            startStock[it->name] = it->quantity;
        // }
    }

    // std::map<std::string, ProcessInfo> tmpProcess = std::map<std::string, ProcessInfo>();
    // std::map<std::string, int> tmpStock = std::map<std::string, int>();
    for (size_t i = 0; i < GEN_SIZE; i++) {
        DNA newDNA = DNA();
        // newDNA.createGenesSequence();

        // for (auto process = vProcess.begin(); process != vProcess.end(); process++) {
        //     int maxDoable = std::numeric_limits<int>::max();
        //     for (auto needStock = process->neededStock.begin(); needStock != process->neededStock.end(); needStock++) {
        //         if (startStock[needStock->first] < needStock->second) {
        //             maxDoable = 0;
        //             break ;
        //         }
        //         // TODO: update maxDoable
        //     }
        //     if (maxDoable > 0) // TODO: take random number between 1 and maxDoable
        //         tmpProcess[process->name] = ProcessInfo(maxDoable, process->delay);
        // }

        // CycleSnapshot firstPeople = CycleSnapshot(0, tmpProcess, startStock);
        // for (size_t j = 0; j < GEN_LENGTH; j++) {

        // }
        // actualGen.push_back(firstPeople);
    }
}

void Parser::createGoodsLeaderboard() {
    wantedGoods = std::map<std::string, int>();
	for (auto goal = vGoal.begin(); goal != vGoal.end(); goal++) {
		wantedGoods[goal->name] = 100;
	}
	// TODO: Give points to intermediary products
	for (auto goal = vGoal.begin(); goal != vGoal.end(); goal++) {
		for (auto stockInfo = vStock.begin(); stockInfo != vStock.end(); stockInfo++) {
			if (stockInfo->name.compare(goal->name) == 0) {
				for (auto processName = stockInfo->waysToProduce.begin(); processName != stockInfo->waysToProduce.end(); processName++) {
                    for (auto process = vProcess.begin(); process != vProcess.end(); process++) {
                        if (process->name.compare(*processName) == 0) {
                            // std::cout << "Check2A " << process->name << " size: " << process->neededStock.size() << std::endl;
                            for (const auto &neededGood : process->neededStock) {
                                // std::cout << "- " << neededGood.first << std::endl;
                                if (wantedGoods.find(neededGood.first) == wantedGoods.end()) {
                                    wantedGoods[neededGood.first] = 2;
                                }
                                else if (wantedGoods[neededGood.first] < 9)
                                    wantedGoods[neededGood.first] += 1;
                            }

                            break;
                        }
                    }
				}
			}
		}
	}
	// TODO: Malus for not useful stuff
	for (auto stockInfo = vStock.begin(); stockInfo != vStock.end(); stockInfo++) {
		if (wantedGoods.find(stockInfo->name) == wantedGoods.end()) {
			wantedGoods[stockInfo->name] = 0;
			// wantedGoods[stockInfo->name] = -5;
		}
	}
    std::cout << std::endl << "--- Goods Ratings" << std::endl;
	for (auto test = wantedGoods.begin(); test != wantedGoods.end(); test++) {
		std::cout << test->first << ": " << test->second << std::endl;
	}
}

void Parser::createGoodsLeaderboard2() {
    goodsTiers = std::vector<std::vector<GoodInfo> >();
    std::vector<GoodInfo> oldTier = std::vector<GoodInfo>();
    // Fill first tier
	for (auto goal = vGoal.begin(); goal != vGoal.end(); goal++) {
        oldTier.push_back(GoodInfo(goal->name));
    }
    goodsTiers.push_back(oldTier);
    std::vector<GoodInfo> newTier = std::vector<GoodInfo>();
    // Recursively create all other tiers
    while (true) {
        for (const auto &good : oldTier) {
            for (const auto &stockInfo : vStock) {
                if (stockInfo.name.compare(good.name) == 0) {
                    for (const auto &processName : stockInfo.waysToProduce) {
                        for (const auto &process : vProcess) {
                            if (process.name.compare(processName) == 0) {
                                for (const auto &neededGood : process.neededStock) {
                                    // Check if good is already in a tier or not
                                    bool found = false;
                                    // Look in current tier
                                    size_t idx = 0;
                                    for (const auto &newGood : newTier) {
                                        if (newGood.name.compare(neededGood.first) == 0) {
                                            found = true;
                                            // If process both requires and produces a good (eg. do_cook:(pan:1):(pan:1):1), then do not increase counter
                                            bool isSelfMaintained = false;
                                            for (const auto &producedGood : process.resultStock) {
                                                if (producedGood.first.compare(neededGood.first) == 0 && producedGood.second <= neededGood.second) {
                                                    isSelfMaintained = true;
                                                    break;
                                                }
                                            }
                                            if (!isSelfMaintained) {
                                                newTier[idx].timesNeededByHigherStock++;
                                            }
                                            break;
                                        }
                                        idx++;
                                    }
                                    if (found)
                                        continue;
                                    // Look in past tiers
                                    size_t tierIdx = 0;
                                    for (const auto &tier : goodsTiers) {
                                        idx = 0;
                                        for (const auto &savedGood : tier) {
                                            if (savedGood.name.compare(neededGood.first) == 0) {
                                                found = true;
                                                // If process both requires and produces a good (eg. do_cook:(pan:1):(pan:1):1), then do not increase counter
                                                bool isSelfMaintained = false;
                                                for (const auto &producedGood : process.resultStock) {
                                                    if (producedGood.first.compare(neededGood.first) == 0 && producedGood.second <= neededGood.second) {
                                                        isSelfMaintained = true;
                                                        break;
                                                    }
                                                }
                                                if (!isSelfMaintained) {
                                                    goodsTiers[tierIdx][idx].timesNeededByLowerStock++;
                                                }
                                                break;
                                            }
                                            idx++;
                                        }
                                        if (found)
                                            break;
                                        tierIdx++;
                                    }
                                    if (!found) {
                                        newTier.push_back(GoodInfo(neededGood.first));
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (newTier.size() == 0)
            break;
        goodsTiers.push_back(newTier);
        oldTier = newTier;
        newTier.clear();
    }

    std::cout << "--- Tiers ----------------------------------------" << std::endl;
    for (size_t tierIdx = 0; tierIdx < goodsTiers.size(); tierIdx++) {
        std::cout << "  --   T" << tierIdx << "   --" << std::endl;
        for (size_t idx = 0; idx < goodsTiers[tierIdx].size(); idx++) {
            std::cout << goodsTiers[tierIdx][idx].name << std::endl;
            std::cout << "  timesNeededByHigherStock: " << goodsTiers[tierIdx][idx].timesNeededByHigherStock << std::endl;
            std::cout << "  timesNeededByTierStock: " << goodsTiers[tierIdx][idx].timesNeededByTierStock << std::endl;
            std::cout << "  timesNeededByLowerStock: " << goodsTiers[tierIdx][idx].timesNeededByLowerStock << std::endl;
            std::cout << "  avgDelay: " << goodsTiers[tierIdx][idx].avgDelay << " score: " << goodsTiers[tierIdx][idx].score << std::endl;
        }
    }
}
