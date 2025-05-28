#ifndef ACO_H
#define ACO_H

#include <vector>
#include <random>
#include <utility>

#include "utils.h"

class ACO {
public:
    ACO(int numAnts, double evaporationRate, double alpha, double beta,
        int capacity, const std::vector<Item>& items, int maxIterations, unsigned int seed);

    std::pair<std::vector<int>, int> solve();
    const std::vector<int>& getBestValueHistory() const;

private:
    int numAnts_;
    double evaporationRate_;
    double alpha_;
    double beta_;
    int capacity_;
    std::vector<Item> items_;
    int maxIterations_;

    std::vector<std::vector<double>> pheromones_;
    std::mt19937 rng_;
    unsigned int seed_;

    int bestValueGlobal_;
    std::vector<int> bestSolutionGlobal_;

    std::vector<int> bestValuePerIteration_;

    void initializePheromones();
    std::vector<int> constructSolution();
    double calculateProbability(int itemIndex, int option, int currentWeight);
    void updatePheromones(const std::vector<std::vector<int>>& solutions);
    int calculateValue(const std::vector<int>& solution) const;
    int calculateWeight(const std::vector<int>& solution) const;
    bool isFeasible(const std::vector<int>& solution) const;
    double getHeuristicInformation(int itemIndex) const;
};

#endif // ACO_H
