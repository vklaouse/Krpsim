#include "Parser.class.hpp"

Parser *Parser::instance;

Parser::Parser(std::vector<Token> &tokens) : myRandom(rand()) {
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
    if (tmpVal == 0) {
		errors.push_back("Parser Error: Numbers must be strictly positives");
		return false;
	}
	*myInt = static_cast<int>(tmpVal);
	return true;
}

void Parser::runSimlation(int lifeTime) {
    clock_t killTime = clock() + (lifeTime * CLOCKS_PER_SEC);
    (void)killTime;

    createGoodsLeaderboard();
    setProcessScores();
    // std::cout << "+++++++++++++++++++++++++" << std::endl;
    // for (auto const &proc : vProcess) {
    //     std::cout << proc.name << std::endl;
    // }
    std::sort(vProcess.begin(), vProcess.end(), &Parser::sortProcessFunction);
    std::cout << "++++++++++ Process Leaderboard +++++++++++++++" << std::endl;
    for (auto const &proc : vProcess) {
        std::cout << proc.name << " " << proc.score << std::endl;
    }

    // Find best path using genetic algo
    // 1) Create Initial population
    createFirstGen();
	size_t totalFit;

    int gen = 1;
    while (clock() < killTime) {
        std::cout << "__new-gen__" << gen << std::endl; 
		totalFit = 1;
        size_t totalSize = 0;
        // 2) Rank solutions
		for (auto &dna : actualGen) {
            totalSize += dna.getGene().size();
			totalFit += dna.getFitness();
	    }
        std::cout << "Avg gen size: " << (totalSize / actualGen.size()) << std::endl;
        std::cout << "Avg fitness: " << (totalFit / actualGen.size()) << std::endl;


        // 3) Cross Over
		for (size_t i = 0; i < POPULATION_SIZE; i++) {
			crossOver(totalFit);
            // 3bis) Mutation
            mutation();

		}
        actualGen = childGen;
        gen++;
    }

    // Print best path
    DNA *bestDNA = &(actualGen[0]);
    for (auto &dna : actualGen) {
        if (dna.evalFitness() > bestDNA->getFitness()) {
            bestDNA = &dna;
        }
    }
    bestDNA->description();
    std::cout << "TotGen: " << gen << std::endl;
}

void Parser::mutation() {
    int idx = rand() % (childGen.back().getGene().size() * 100);
    childGen.back().justMutation(idx);
}

void Parser::crossOver(size_t totalFit) {
    size_t somePartOfTotalFit = rand() % totalFit;
    size_t somePartOfTotalFit2 = rand() % totalFit;
    int idxParentA = 0;
    int idxParentB = 0;
    for (size_t i = 0; i < POPULATION_SIZE; i++) {
        if (somePartOfTotalFit > 0) {
            if (somePartOfTotalFit < actualGen[i].getFitness()) {
                // parent A found !
                idxParentA = i;
                somePartOfTotalFit = 0;
            }
            else {
                somePartOfTotalFit -= actualGen[i].getFitness();
            }
        }

        if (somePartOfTotalFit2 > 0) {
            if (somePartOfTotalFit2 < actualGen[i].getFitness()) {
                // parent B found !
                idxParentB = i;
                somePartOfTotalFit2 = 0;
            }
            else {
                somePartOfTotalFit2 -= actualGen[i].getFitness();
            }
        }

        if (somePartOfTotalFit == 0 && somePartOfTotalFit2 == 0)
            break ;
    }
    // If same parent has been chosen, then no crossOver will happen
    if (idxParentA == idxParentB) {
	    childGen.push_back(actualGen[idxParentA]);
        return;
    }


    // Find on which genes the crossover can happen
	std::map<int, std::vector<int>> possibleCrossOver = std::map<int, std::vector<int>>();
	compareDNAForCrossOver(actualGen[idxParentA], actualGen[idxParentB], &possibleCrossOver);

    if (possibleCrossOver.size() == 0) {
        // handle parents that cannot crossover except for start gene
	    childGen.push_back(actualGen[idxParentA]);
        childGen.back().justMutation(rand() % childGen.back().getGene().size());
        return;
    }

    int random = rand() % possibleCrossOver.size();
	size_t geneA;
    for (const auto &mapValues : possibleCrossOver) {
        if (random == 0) {
            geneA = mapValues.first;
            break;
        }
        random--;
    }
    random = rand() % possibleCrossOver[geneA].size();
	size_t geneB = possibleCrossOver[geneA][random];
	childGen.push_back(DNA(actualGen[idxParentA], actualGen[idxParentB], geneA, geneB));
}

void Parser::compareDNAForCrossOver(DNA &first, DNA &second, std::map<int, std::vector<int>> *possibleCrossOver) {
	for (size_t i = 0; i < first.getGene().size(); i++) {
		std::vector<int> fCrossover = std::vector<int>();
		for (size_t j = 0; j < second.getGene().size(); j++) {
            if (i == 0 && j == 0) {
                continue;
            }
			if (DNA::compareGenes(first.getGene()[i], second.getGene()[j])) {
				fCrossover.push_back(j);
            }
		}
		if (fCrossover.size() > 0)
			possibleCrossOver->insert(std::pair<int, std::vector<int>>(i, fCrossover));
	}
}

void Parser::createFirstGen() {
    startStock = std::map<std::string, int>();
    for (auto it = vStock.begin(); it != vStock.end(); it++) {
        startStock[it->name] = it->quantity;
    }
    actualGen = std::vector<DNA> (POPULATION_SIZE);
}

void Parser::createGoodsLeaderboard() {
    wantedGoods = std::map<std::string, size_t>();
    std::map<std::string, size_t> tmpWantedGoods = std::map<std::string, size_t>();
    goodsTiers = std::vector<std::vector<GoodInfo> >();
    std::vector<GoodInfo> oldTier = std::vector<GoodInfo>();
    // Fill first tier
	for (auto goal = vGoal.begin(); goal != vGoal.end(); goal++) {
        oldTier.push_back(GoodInfo(goal->name, 1));
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
                                // Skip process if its selfMaintained for current stockInfo
                                auto goodNeedsSelf = process.neededStock.find(good.name);
                                if (goodNeedsSelf != process.neededStock.end()) {
                                    int producedQuantity = process.resultStock.at(good.name);
                                    if (goodNeedsSelf->second >= producedQuantity) {
                                        break;
                                    }
                                }

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
                                                newTier[idx].timesNeededByHigherStock += process.resultStock.at(good.name);
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
                                                    if (tierIdx == goodsTiers.size() - 1)
                                                        goodsTiers[tierIdx][idx].timesNeededByTierStock += process.resultStock.at(good.name);
                                                    else
                                                        goodsTiers[tierIdx][idx].timesNeededByLowerStock += process.resultStock.at(good.name);
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
                                        newTier.push_back(GoodInfo(neededGood.first, process.resultStock.at(good.name)));
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

    // Give every good a score based on how much is used by other goods
    size_t score = 0;
    size_t tierScore = 0;
    size_t idx = 0;
    std::cout << "--- Tiers ----------------------------------------" << std::endl;
    for (const auto &tier : goodsTiers) {
        // Little bonus for higher tier
        std::cout << "  --   T" << idx << "   --" << std::endl;
        tierScore = pow(goodsTiers.size() - idx, 2);
        for (const auto &good : tier) {
            score = tierScore;
            score += good.timesNeededByHigherStock;
            score += good.timesNeededByTierStock / 2;
            score += sqrt(good.timesNeededByLowerStock);

            // Save scores in easier to handle way
            tmpWantedGoods[good.name] = score;

            // Debug
            std::cout << good.name << std::endl;
            std::cout << "  timesNeededByHigherStock: " << good.timesNeededByHigherStock << std::endl;
            std::cout << "  timesNeededByTierStock: " << good.timesNeededByTierStock << std::endl;
            std::cout << "  timesNeededByLowerStock: " << good.timesNeededByLowerStock << std::endl;
            std::cout << "  avgDelay: " << good.avgDelay << " score: " << score << std::endl;
        }
        idx++;
    }


    std::map<std::string, size_t> childsScore = std::map<std::string, size_t>();
    std::map<std::string, size_t> newWantedGoods = std::map<std::string, size_t>();
    idx = goodsTiers.size() - 1;
    size_t bestScore = 0;
    size_t tmpScore = 0;
    while (idx < goodsTiers.size()) {
        newWantedGoods.clear();
        for (const auto &good : goodsTiers[idx]) {
            // Retrieve value that we got before
            newWantedGoods[good.name] = tmpWantedGoods[good.name];

            // Search for goods that are used to make current one (except lowest tier)
            if (idx < goodsTiers.size() - 1) {
                childsScore.clear();
                bestScore = 0;
                for (const auto &stockInfo : vStock) {
                    if (stockInfo.name.compare(good.name) == 0) {
                        for (const auto &processName : stockInfo.waysToProduce) {
                            for (const auto &process : vProcess) {
                                if (process.name.compare(processName) == 0) {
                                    // Skip process if its selfMaintained for current stockInfo
                                    auto goodNeedsSelf = process.neededStock.find(good.name);
                                    if (goodNeedsSelf != process.neededStock.end()) {
                                        int producedQuantity = process.resultStock.at(good.name);
                                        if (goodNeedsSelf->second >= producedQuantity) {
                                            break;
                                        }
                                    }

                                    tmpScore = 0;
                                    for (const auto &neededGood : process.neededStock) {
                                        // Skip if neededGood is not actually used in process
                                        if (process.resultStock.find(neededGood.first) != process.resultStock.end() && neededGood.second >= process.resultStock.at(neededGood.first)) {
                                            continue;
                                        }

                                        // If needs good from same or higher tier then get its temporary value
                                        if (wantedGoods.find(neededGood.first) == wantedGoods.end()) {
                                            tmpScore += tmpWantedGoods[neededGood.first] * neededGood.second;
                                        }
                                        else { // Else get final value
                                            tmpScore += wantedGoods[neededGood.first] * neededGood.second;
                                        }

                                        // auto it = childsScore.find(neededGood.first);
                                        // if (it == childsScore.end()) {
                                        //     childsScore[neededGood.first] = neededGood.second;
                                        // }
                                        // else if (it->second > childsScore[neededGood.first]) {
                                        //     childsScore[neededGood.first] = it->second;
                                        // }
                                    }

                                    // tmpScore /= process.resultStock.at(good.name); // Divide score by how many of this good will be produced
                                    if (tmpScore > bestScore) {
                                        bestScore = tmpScore;
                                    }
                                    // else if (idx != 0 && tmpScore < bestScore) {
                                    //     bestScore = tmpScore;
                                    // }
                                    break;
                                }
                            }
                        }
                    }
                }

                newWantedGoods[good.name] += bestScore;
            }

            // Increase good score by mult with all of its childs
            // std::cout << "How to get " << good.name << " --> ";
            // for (const auto &childScore : childsScore) {
            //     // std::cout << " |" << childScore.first << " * " << childScore.second << "| ";
            //     newWantedGoods[good.name] += childScore.second * wantedGoods[childScore.first];
            // }
            // std::cout << std::endl;
        }
        // Add newWantedGoods to final map
        for (auto const &newInfo : newWantedGoods) {
            wantedGoods[newInfo.first] = newInfo.second;
        }
        if (idx == 0)
            break;
        idx--;
    }

    std::cout << std::endl << "--- Goods Ratings" << std::endl;
	for (auto test = wantedGoods.begin(); test != wantedGoods.end(); test++) {
		std::cout << test->first << ": " << test->second << std::endl;
	}
}


bool Parser::sortProcessFunction(Process const& lhs, Process const& rhs) {
    return lhs.score > rhs.score;
}


void Parser::setProcessScores() {

    // size_t tierScore = pow(10, goodsTiers.size() - idx);
    for (size_t processIdx = 0; processIdx < vProcess.size(); processIdx++) {
        vProcess[processIdx].score = 0;
        size_t idx = 0;
        for (; idx < goodsTiers.size(); idx++) {
            for (const auto &goodInTier : goodsTiers[idx]) {
                for (const auto &good : vProcess[processIdx].resultStock) {
                    if (good.first.compare(goodInTier.name) == 0) {
                        vProcess[processIdx].score = pow(10, goodsTiers.size() - idx);
                        break;
                    }
                }
                if (vProcess[processIdx].score != 0)
                    break;
            }
            if (vProcess[processIdx].score != 0)
                break;
        }
    }
}

size_t Parser::getProcessScore(Process const& process) {
    size_t score = 0;
    for (const auto &good : process.resultStock) {
        if (wantedGoods.find(good.first) != wantedGoods.end())
            score += wantedGoods[good.first] * good.second;
    }

    size_t subScore = 0;
    for (const auto &good : process.neededStock) {
        if (wantedGoods.find(good.first) != wantedGoods.end())
            subScore += wantedGoods[good.first] * good.second;
    }

    if (subScore > score)
        score = 0;
    else
        score -= subScore;

    return score;
}
