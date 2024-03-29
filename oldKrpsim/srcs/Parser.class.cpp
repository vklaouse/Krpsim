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
	startStock = std::map<std::string, int>();
    for (auto it = vStock.begin(); it != vStock.end(); it++) {
        startStock[it->name] = it->quantity;
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
        vGoal.push_back(Goal(tokens[i].info, optimizeTime, false, 1));
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
	// *myInt = static_cast<int>(tmpVal);
	return true;
}

void Parser::runSimlation(int lifeTime, bool verboseOption) {
    this->verboseOption = verboseOption;
	this->_lifeTime = lifeTime;
    if (verboseOption) {
        std::cerr << VERBOSE_SECTION_START << std::endl;
    }

    clock_t killTime = clock() + (_lifeTime * CLOCKS_PER_SEC);
    // (void)killTime;

    // Set goods and process score in order to know fitness of DNAs
    createGoodsLeaderboard();
    setProcessScores();
    // Sorting the process pushes Genes to prioritize most valuable ones
    std::sort(vProcess.begin(), vProcess.end(), &Parser::sortProcessFunction);
	// size_t idx = 0;
	// for (idx = 0; idx < vProcess.size(); idx++) {
	// 	if (vProcess[idx].score == 0)
	// 		break;
	// }
	// if (idx != vProcess.size()) {
	// 	vProcess.erase(vProcess.begin() + idx, vProcess.end());
	// }
    if (verboseOption) {
        std::cerr << " ----- Process score -----" << std::endl;
        for (auto const &proc : vProcess) {
            std::cerr << proc.name << ": " << proc.score << std::endl;
        }
    }

    // Create Initial population
    createFirstGen();

	size_t totalFit;
    int gen = 1;
    // Main loop for genetic algorithm
    while (clock() < killTime) {
        totalFit = getGenerationFitness(gen);

		for (size_t i = 0; i < POPULATION_SIZE; i++) {
			crossOver(totalFit);
            mutation();
		}

        actualGen = childGen;
        gen++;
    }
    totalFit = getGenerationFitness(gen);

    // Find best path
    DNA *bestDNA = &(actualGen[0]);
    for (auto &dna : actualGen) {
        if (dna.evalFitness() > bestDNA->getFitness()) {
            bestDNA = &dna;
        }
    }
    int maxDelay = vProcess[0].delay;
    for (const auto &process : vProcess) {
        if (process.delay > maxDelay)
            maxDelay = process.delay;
    }
    bestDNA->createFollowingGenes(bestDNA->getGene().size() + maxDelay, false);
	bool tmp = bestDNA->getHasSelfMaintainedProduction();
	finishSolution(bestDNA);
	bestDNA->setHasSelfMaintainedProduction(tmp);
    // Print solution
    // bestDNA->description();
    bestDNA->printSolution();
    if (verboseOption) {
        std::cerr << VERBOSE_SECTION_END << std::endl;
    }
}

void Parser::concatGeneVector(std::vector<Gene> *myGene, std::vector<Gene> initialGene, int i) {
	size_t fusion = 0;
	if (verboseOption)
		std::cerr << "Duplicating best cycle for the " << i << " time" << std::endl;
	fusion = myGene->size();
	myGene->insert(myGene->end(), initialGene.begin(), initialGene.end());
	for (const auto &stk : myGene->front().initialStock) {
		int solution = (myGene->back().initialStock[stk.first] - stk.second) * i;
		myGene->at(myGene->size() - 1).initialStock[stk.first] += solution;
		myGene->at(myGene->size() - 1).currentStock[stk.first] += solution;
	}
	for (size_t i = fusion; i < myGene->size(); i++) {
		myGene->at(i).actualCycle += myGene->at(fusion - 1).actualCycle;
	}
}

void Parser::finishSolution(DNA *bestDNA) {
	Gene tmpGene;
	// If we have enough stock to start a shortcut dont be fooled and generate resources in advance !!
	if (bestDNA->getHasSelfMaintainedProduction()) {
		for (const auto &shortcut : shortcutProcess) {
			for (const auto &process : shortcut.second) {
				int maxDelay = vProcess[0].delay;
				std::map<std::string, int> neededStock;
				for (const auto &process2 : vProcess) {
					if (process2.name.compare(process) == 0) {
						neededStock = process2.neededStock;
						for (const auto &needed : neededStock) {
							if (process2.resultStock.find(needed.first) != process2.resultStock.end() && process2.resultStock.at(needed.first) == needed.second) {
								neededStock[needed.first] = 0;
							}
						}
					}
			        if (process2.delay > maxDelay)
			            maxDelay = process2.delay;
			    }
				int i = 0;
				std::vector<Gene> tmpGeneInitial = bestDNA->getGeneCpy();
				std::map<std::string, int> createdStock;
				bool canBreak = false;
				while (!canBreak) {
					// concatGeneVector
					++i;
					if (i == 1000)
						break;
					concatGeneVector(bestDNA->getGenePtr(), tmpGeneInitial, i);
					// decrease neededStock, if all values < 0 can break loop
					canBreak = true;
					for (const auto & needed : neededStock) {
						if (neededStock[needed.first] <= 0)
							continue;
						if (neededStock[needed.first] > (bestDNA->getGene().back().initialStock[needed.first] - bestDNA->getGene().front().initialStock[needed.first])) {
							canBreak = false;
						}
					}
				}
			}
		}
	}
	// 'Normal' loop where we resources are generated along the way
	for (const auto &shortcut : shortcutProcess) {
		for (const auto &process : shortcut.second) {
			tmpGene = bestDNA->getGeneCpy().back();
			std::vector<Gene> tmpGeneInitial = bestDNA->getGeneCpy();
			int i = 0;
			while (!tmpGene.applyProcessToStock(process, &tmpGene.currentStock, 1)) {
				++i;
				std::cout << "hey 2" << std::endl;

				concatGeneVector(bestDNA->getGenePtr(), tmpGeneInitial, i);
				tmpGene = bestDNA->getGene().at(bestDNA->getGene().size() - 1);
			}
			int maxDelay = vProcess[0].delay;
		    for (const auto &process2 : vProcess) {
				if (process2.name.compare(process) == 0) {
					bestDNA->getGenePtr()->back().vProcess[process].push_back(process2.delay * -1);
				}
		        if (process2.delay > maxDelay)
		            maxDelay = process2.delay;
		    }
			bestDNA->getGenePtr()->back().applyProcessToStock(process, &bestDNA->getGenePtr()->back().currentStock, 1);
			bestDNA->createFollowingGenes(bestDNA->getGene().size() + maxDelay, false);
		}
	}
}

size_t Parser::getGenerationFitness(int generationCycle) {
    size_t totalFit = 1;
    size_t totalSize = 0;
    size_t shortestBest = std::numeric_limits<size_t>::max();
    size_t shortestFitness = 0;
    // size_t bestIdx = -1;
    // 2) Rank solutions
    for (auto &dna : actualGen) {
        totalSize += dna.getGene().size();
        totalFit += dna.getFitness();
        if (dna.getHasSelfMaintainedProduction() && dna.getGene().size() < shortestBest) {
            shortestBest = dna.getGene().size();
            shortestFitness = dna.getFitness();
        }
    }
    if (verboseOption) {
        std::cerr << " -- Generation #" << generationCycle << " --" << std::endl;
        std::cerr << "Avg DNA size: " << (totalSize / actualGen.size()) << std::endl;
        std::cerr << "Avg fitness: " << (totalFit / actualGen.size()) << std::endl;
        if (shortestBest != std::numeric_limits<size_t>::max()) {
            std::cerr << "Shortest : " << shortestBest << std::endl;
            std::cerr << "Shortest fitness : " << shortestFitness << std::endl;
        }
    }

    return totalFit;
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
    // If only one of the two parents hasSelfMaintainedProduction then add this one
    if (actualGen[idxParentA].getHasSelfMaintainedProduction() && !actualGen[idxParentB].getHasSelfMaintainedProduction()) {
	    childGen.push_back(actualGen[idxParentA]);
        return;
    }
    if (!actualGen[idxParentA].getHasSelfMaintainedProduction() && actualGen[idxParentB].getHasSelfMaintainedProduction()) {
	    childGen.push_back(actualGen[idxParentB]);
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
	// childGen.push_back(actualGen[idxParentA]);
	// return ;
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

    // Simplify tiers if possile -> if there is only one way to produce an optimize, then the resources needed to do this good will become the optimize itself
    shortcutProcess = std::map<std::string, std::vector<std::string>>();
    std::map<std::string, int> neededForOptimize = std::map<std::string, int>();
    bool hasDoneChanges = true;
    bool canSkip;
    std::string savedProcessName;
    // int processScore; // TODO: use this to better optimize shortcuts
    while (oldTier.size() == 1 && hasDoneChanges) {
        hasDoneChanges = false;
        canSkip = false;
        // for (size_t i = 0; i < oldTier.size(); i++) {
            // std::cout << "Index: " << i << ", good: " << oldTier[0].name << std::endl;
            neededForOptimize.clear();
            canSkip = false;
            for (const auto &stockInfo : vStock) {
                if (stockInfo.name.compare(oldTier[0].name) == 0) {
                    for (const auto &processName : stockInfo.waysToProduce) {
                        for (const auto &process : vProcess) {
                            if (process.name.compare(processName) == 0) {
                                if (neededForOptimize.empty()) {
                                    // First time, save all needed resources
                                    savedProcessName = process.name;
                                    for (const auto &neededGood : process.neededStock) {
                                        bool skipGood = false;
                                        // Skip same tier needs
                                        for (const auto &check : oldTier) {
                                            if (check.name.compare(neededGood.first) == 0) {
                                                skipGood = true;
                                                break;
                                            }
                                        }
                                        if (skipGood)
                                            continue;
                                        // Skip if good is both in needed and result
                                        if (process.resultStock.find(neededGood.first) != process.resultStock.end() && process.resultStock.at(neededGood.first) == neededGood.second) {
                                            continue;
                                        }
                                        neededForOptimize[neededGood.first] = neededGood.second;
                                    }
                                }
                                else {
                                    // check that needed resources are exactly the ones needed in previous process
                                    for (const auto &neededGood : process.neededStock) {
                                        bool skipGood = false;
                                        // Skip same tier needs
                                        for (const auto &check : oldTier) {
                                            if (check.name.compare(neededGood.first) == 0) {
                                                skipGood = true;
                                                break;
                                            }
                                        }
                                        if (skipGood)
                                            continue;
                                        // Skip if good is both in needed and result
                                        if (process.resultStock.find(neededGood.first) != process.resultStock.end() && process.resultStock.at(neededGood.first) == neededGood.second) {
                                            continue;
                                        }
                                        if (neededForOptimize.find(neededGood.first) == neededForOptimize.end() || neededForOptimize[neededGood.first] != neededGood.second) {
                                            // Value not found or not the same as prev process, exit from loop and do not simplify
                                            canSkip = true;
                                            neededForOptimize.clear();
                                        }
                                    }
                                }
                                break;
                            }
                        }
                        if (canSkip)
                            break;
                    }
                    break;
                }
            }
            if (!neededForOptimize.empty()) {
                // Optimize good can be replaced !

                // Save process shortcut
				std::vector<std::string> newVProcess = std::vector<std::string>();
				if (shortcutProcess.find(oldTier[0].name) != shortcutProcess.end()) {
					newVProcess = shortcutProcess[oldTier[0].name];
				}
				newVProcess.insert(newVProcess.begin(), savedProcessName);

                // Erase from goals
                bool optimizeTime;
                bool isFirst = true;
                for (size_t j = 0; j < vGoal.size(); j++) {
                    if (vGoal[j].name.compare(oldTier[0].name) == 0) {

                        optimizeTime = vGoal[j].optimizeTime;
                        vGoal.erase(vGoal.begin() + j);
                        break;
                    }
                }

                int oldMultipier = oldTier[0].timesNeededByHigherStock; // useful if we have chain of simplify
                // Fill in new values
                for (const auto &newOptimize : neededForOptimize) {
                    if (isFirst) {
                        isFirst = false;
                        shortcutProcess[newOptimize.first] = newVProcess;
                    }
                    oldTier.push_back(GoodInfo(newOptimize.first, newOptimize.second * oldMultipier, true));
                    vGoal.push_back(Goal(newOptimize.first, optimizeTime, true, newOptimize.second * oldMultipier));
                }
                hasDoneChanges = true;


				shortcutProcess.erase(oldTier[0].name);
                oldTier.erase(oldTier.begin()); // Erase from tier
                // i--; // counters the fact that we erased the old value
            }
        // }
    };
    if (verboseOption) {
        std::cerr << "-------- Shortcut Process ---------" << std::endl;
        for (const auto &goodKey: shortcutProcess) {
            std::cerr << goodKey.first << " [";

            for (const auto &procName : goodKey.second) {
                std::cerr << " " << procName;
            }
            std::cerr << " ]" << std::endl;
        }
    }

    // Save optimize tier
    goodsTiers.push_back(oldTier);

    // Recursively create all other tiers
    std::vector<GoodInfo> newTier = std::vector<GoodInfo>();
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
                                    // Skip if good is both in needed and result
                                    // if (process.resultStock.find(neededGood.first) != process.resultStock.end() && process.resultStock.at(neededGood.first) == neededGood.second) {
                                    //     continue;
                                    // }
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
    if (verboseOption) {
        std::cerr << " ----- Tiers -----" << std::endl;
    }
    for (const auto &tier : goodsTiers) {
        // Little bonus for higher tier
        if (verboseOption) {
            std::cerr << "  --   T" << idx << "   --" << std::endl;
        }
        tierScore = pow(goodsTiers.size() - idx, 2);
        for (const auto &good : tier) {
            score = tierScore;
            score += good.timesNeededByHigherStock;
            score += good.timesNeededByTierStock / 2;
            score += sqrt(good.timesNeededByLowerStock);

            // Save scores in easier to handle way
            tmpWantedGoods[good.name] = score;

            // Debug
            if (verboseOption) {
                std::cerr << good.name << std::endl;
                std::cerr << "  timesNeededByHigherStock: " << good.timesNeededByHigherStock << std::endl;
                std::cerr << "  timesNeededByTierStock: " << good.timesNeededByTierStock << std::endl;
                std::cerr << "  timesNeededByLowerStock: " << good.timesNeededByLowerStock << std::endl;
                std::cerr << "  isShortcut: " << std::boolalpha << good.isShortcut << std::endl;
                std::cerr << "  score: " << score << std::endl;
            }
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
        }
        // Add newWantedGoods to final map
        for (auto const &newInfo : newWantedGoods) {
            wantedGoods[newInfo.first] = newInfo.second;
        }
        if (idx == 0)
            break;
        idx--;
    }

    if (verboseOption) {
        std::cerr << " ----- Goods score -----" << std::endl;
        for (auto test = wantedGoods.begin(); test != wantedGoods.end(); test++) {
            std::cerr << test->first << ": " << test->second << std::endl;
        }
    }
}


bool Parser::sortProcessFunction(Process const& lhs, Process const& rhs) {
    return lhs.score > rhs.score;
}


void Parser::setProcessScores() {
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
