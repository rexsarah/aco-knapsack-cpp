#ifndef ACO_H
#define ACO_H

#include <vector>
#include "utils.h"
#include <random>

class ACO {
public:
    ACO(int numAnts, double evaporationRate, double alpha, double beta,
        int capacity, const std::vector<Item>& items, int maxIterations);

    std::pair<std::vector<int>, int> solve();

    std::vector<int> getBestSolution() const;
    int getBestValue() const;
    int getIterationCount() const;

private:
    int numAnts_;
    double evaporationRate_;
    double alpha_;
    double beta_;
    int capacity_;
    std::vector<Item> items_;
    int numItems_;
    int maxIterations_;
    int iterationCount_;

    std::vector<std::vector<double>> pheromone_;
    std::vector<double> heuristicValue_;
    std::vector<int> bestSolution_;
    int bestValue_;

    std::vector<int> constructSolution(std::mt19937& rng);
    bool isFeasible(const std::vector<int>& solution);
    int calculateValue(const std::vector<int>& solution);
    int calculateWeight(const std::vector<int>& solution);
    void updatePheromones(const std::vector<std::vector<int>>& antSolutions);
};

#endif // ACO_H