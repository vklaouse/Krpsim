#include "DNA.class.hpp"
#include "Parser.class.hpp"

bool Gene::loopDirection = true;

Gene::Gene(int actualCycle, std::map<std::string, std::vector<int> > vProcess, std::map<std::string, int> currentStock)
    : actualCycle(actualCycle), vProcess(vProcess), currentStock(currentStock) {
    
    std::vector<std::string> doableProcess = std::vector<std::string>();
    // std::map<std::string, int> tmpCurrentStock = currentStock;

    for (auto process = Parser::instance->getProcess().begin(); process != Parser::instance->getProcess().end(); process++) {
        
        bool maxDoable = true;
        for (auto needStock = process->neededStock.begin(); needStock != process->neededStock.end(); needStock++) {
            if (currentStock[needStock->first] < needStock->second) {
                maxDoable = false;
                break ;
            }
            // TODO: update maxDoable
        }
        if (maxDoable) // TODO: take random number between 1 and maxDoable
            doableProcess.push_back(process->name);
    }
    doableProcessGene((rand() % doableProcess.size()), doableProcess);
}

void Gene::doableProcessGene(int index, std::vector<std::string> doableProcess) {
    std::map<std::string, int> tmpCurrentStock = currentStock;
    
    if (Gene::loopDirection) {
        for (int i = index; i != index; --i) {
            if (i <= 0)
                i = doableProcess.size();
            
            
        }
        Gene::loopDirection == false;
        return ;
    }
    Gene::loopDirection = true;
}

DNA::DNA() : fitness(0), vGene (std::vector<Gene>()) {
    Gene firstGene = Gene(0, std::map<std::string, std::vector<int> >(), Parser::instance->getStartStock());
}

// void DNA::createGenesSequence(std::vector<Gene> startingGenes, std::map<std::string, int> startStock) {
//     // genes = startingGenes;
//     // currentStock = startStock;
// }