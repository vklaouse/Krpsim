#include "Agent.hpp"

// TODO:
// - Remove needed == result stock from Graphe
// - Find a way to give a % chance to a process to be done
// - Start run simulation

Agent::Agent(std::vector<Stock> &vStock) {
	std::cout << "New Agent" << std::endl;
	for (auto &stock : vStock) {
		if (stock.waysToProduce.size() > 1) {
			for (auto &process : stock.processForProduce) {
				std::cout << stock.name << " " << process.first << std::endl;
				stockMultProdWayCnt[stock.name][process.first] = 0;
				// stockMultProdWayProb
			}
		}
	}
	std::cout << stockMultProdWayCnt.size() << std::endl;

	return ;
}
