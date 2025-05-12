#include "aco.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>

ACO::ACO(int numAnts, double evaporationRate, double alpha, double beta,
        int capacity, const std::vector<Item>& items, int maxIterations)
    : numAnts_(numAnts), evaporationRate_(evaporationRate), alpha_(alpha), beta_(beta),
      capacity_(capacity), items_(items), numItems_(items.size()), maxIterations_(maxIterations),
      iterationCount_(0), pheromone_(numItems_, std::vector<double>(2, 0.1)), // [item][0=not_in, 1=in]
      heuristicValue_(numItems_), bestValue_(0) {
    for (int i = 0; i < numItems_; ++i) {
        heuristicValue_[i] = static_cast<double>(items_[i].value) / items_[i].weight;
    }
    bestSolution_.resize(numItems_, 0);
}

std::pair<std::vector<int>, int> ACO::solve() {
    std::random_device rd;
    std::mt19937 rng(rd());

    for (iterationCount_ = 0; iterationCount_ < maxIterations_; ++iterationCount_) {
        std::vector<std::vector<int>> antSolutions(numAnts_);
        for (int i = 0; i < numAnts_; ++i) {
            antSolutions[i] = constructSolution(rng);
            int currentValue = calculateValue(antSolutions[i]);
            if (currentValue > bestValue_) {
                bestValue_ = currentValue;
                bestSolution_ = antSolutions[i];
            }
        }
        updatePheromones(antSolutions);
    }
    return {bestSolution_, bestValue_};
}

std::vector<int> ACO::constructSolution(std::mt19937& rng) {
    std::vector<int> solution(numItems_, 0);
    std::vector<bool> availableItems(numItems_, true);
    int currentWeight = 0;

    while (true) {
        std::vector<double> probabilities;
        double totalProbability = 0.0;

        for (int i = 0; i < numItems_; ++i) {
            if (availableItems[i] && currentWeight + items_[i].weight <= capacity_) {
                double pheromone = pheromone_[i][1]; // Pheromone for including the item
                double heuristic = heuristicValue_[i];
                double probability = std::pow(pheromone, alpha_) * std::pow(heuristic, beta_);
                probabilities.push_back(probability);
                totalProbability += probability;
            } else {
                probabilities.push_back(0.0);
            }
        }

        if (totalProbability <= 0.0) {
            break; // No more items can be added
        }

        std::uniform_real_distribution<> dist(0.0, totalProbability);
        double randomValue = dist(rng);
        double cumulativeProbability = 0.0;
        int chosenItemIndex = -1;

        for (int i = 0; i < numItems_; ++i) {
            if (availableItems[i] && currentWeight + items_[i].weight <= capacity_) {
                cumulativeProbability += probabilities[i];
                if (randomValue <= cumulativeProbability) {
                    chosenItemIndex = i;
                    break;
                }
            }
        }

        if (chosenItemIndex != -1) {
            solution[chosenItemIndex] = 1;
            currentWeight += items_[chosenItemIndex].weight;
            availableItems[chosenItemIndex] = false;
        } else {
            break; // Should not happen if totalProbability > 0, but for safety
        }
    }
    return solution;
}

bool ACO::isFeasible(const std::vector<int>& solution) {
    return calculateWeight(solution) <= capacity_;
}

int ACO::calculateValue(const std::vector<int>& solution) {
    int totalValue = 0;
    for (size_t i = 0; i < solution.size(); ++i) {
        if (solution[i] == 1) {
            totalValue += items_[i].value;
        }
    }
    return totalValue;
}

int ACO::calculateWeight(const std::vector<int>& solution) {
    int totalWeight = 0;
    for (size_t i = 0; i < solution.size(); ++i) {
        if (solution[i] == 1) {
            totalWeight += items_[i].weight;
        }
    }
    return totalWeight;
}

void ACO::updatePheromones(const std::vector<std::vector<int>>& antSolutions) {
    // Evaporation
    for (int i = 0; i < numItems_; ++i) {
        pheromone_[i][1] *= (1.0 - evaporationRate_);
        if (pheromone_[i][1] < 0.001) pheromone_[i][1] = 0.001; // Avoid too small values
    }

    // Pheromone deposition
    for (const auto& solution : antSolutions) {
        int solutionValue = calculateValue(solution);
        if (isFeasible(solution)) {
            for (size_t i = 0; i < solution.size(); ++i) {
                if (solution[i] == 1) {
                    pheromone_[i][1] += (double)solutionValue / bestValue_; // Deposit proportional to solution quality
                }
            }
        }
    }
}

std::vector<int> ACO::getBestSolution() const {
    return bestSolution_;
}

int ACO::getBestValue() const {
    return bestValue_;
}

int ACO::getIterationCount() const {
    return iterationCount_;
}