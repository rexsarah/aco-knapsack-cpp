#include "aco.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>

ACO::ACO(int numAnts, double evaporationRate, double alpha, double beta,
         int capacity, const std::vector<Item>& items, int maxIterations, unsigned int seed)
    : numAnts_(numAnts), evaporationRate_(evaporationRate), alpha_(alpha), beta_(beta),
      capacity_(capacity), items_(items), maxIterations_(maxIterations), seed_(seed),
      rng_(seed), bestValueGlobal_(0), bestSolutionGlobal_(items.size(), 0) {
    initializePheromones();
}

void ACO::initializePheromones() {
    pheromones_.resize(items_.size(), std::vector<double>(2, 1.0)); // para pegar ou não pegar
}

double ACO::getHeuristicInformation(int itemIndex) const {
    // Heurística simples: valor / peso
    return static_cast<double>(items_[itemIndex].value) / items_[itemIndex].weight;
}

double ACO::calculateProbability(int itemIndex, int option, int currentWeight) {
    // option: 0 = não pegar, 1 = pegar
    if (option == 1 && currentWeight + items_[itemIndex].weight > capacity_)
        return 0.0; // não pode pegar item que excede capacidade

    double tau = pheromones_[itemIndex][option];
    double eta = (option == 1) ? getHeuristicInformation(itemIndex) : 1.0;
    return std::pow(tau, alpha_) * std::pow(eta, beta_);
}

std::vector<int> ACO::constructSolution() {
    std::vector<int> solution(items_.size(), 0);
    int currentWeight = 0;

    // Decidir item a item
    for (size_t i = 0; i < items_.size(); ++i) {
        double probTake = calculateProbability(i, 1, currentWeight);
        double probNotTake = calculateProbability(i, 0, currentWeight);
        double sumProb = probTake + probNotTake;

        double randValue = std::uniform_real_distribution<>(0.0, 1.0)(rng_);

        if (sumProb > 0 && randValue < probTake / sumProb) {
            if (currentWeight + items_[i].weight <= capacity_) {
                solution[i] = 1;
                currentWeight += items_[i].weight;
            }
        } else {
            solution[i] = 0;
        }
    }
    return solution;
}

int ACO::calculateValue(const std::vector<int>& solution) const {
    int totalValue = 0;
    for (size_t i = 0; i < solution.size(); ++i)
        if (solution[i] == 1)
            totalValue += items_[i].value;
    return totalValue;
}

int ACO::calculateWeight(const std::vector<int>& solution) const {
    int totalWeight = 0;
    for (size_t i = 0; i < solution.size(); ++i)
        if (solution[i] == 1)
            totalWeight += items_[i].weight;
    return totalWeight;
}

bool ACO::isFeasible(const std::vector<int>& solution) const {
    return calculateWeight(solution) <= capacity_;
}

void ACO::updatePheromones(const std::vector<std::vector<int>>& solutions) {
    // Evaporação
    for (auto& pheromoneItem : pheromones_)
        for (auto& pheromoneVal : pheromoneItem)
            pheromoneVal *= (1 - evaporationRate_);

    // Adicionar feromônio proporcional à qualidade das soluções
    for (const auto& sol : solutions) {
        int val = calculateValue(sol);
        for (size_t i = 0; i < sol.size(); ++i) {
            int option = sol[i];
            pheromones_[i][option] += val;
        }
    }
}

std::pair<std::vector<int>, int> ACO::solve() {
    bestValueGlobal_ = 0;
    bestSolutionGlobal_ = std::vector<int>(items_.size(), 0);
    bestValuePerIteration_.clear();

    for (int iter = 0; iter < maxIterations_; ++iter) {
        std::vector<std::vector<int>> allSolutions;
        int bestValueIter = 0;
        std::vector<int> bestSolIter;

        for (int ant = 0; ant < numAnts_; ++ant) {
            auto sol = constructSolution();
            if (isFeasible(sol)) {
                int val = calculateValue(sol);
                if (val > bestValueIter) {
                    bestValueIter = val;
                    bestSolIter = sol;
                }
            }
            allSolutions.push_back(sol);
        }

        if (bestValueIter > bestValueGlobal_) {
            bestValueGlobal_ = bestValueIter;
            bestSolutionGlobal_ = bestSolIter;
        }

        updatePheromones(allSolutions);
        bestValuePerIteration_.push_back(bestValueGlobal_);
    }

    return {bestSolutionGlobal_, bestValueGlobal_};
}

const std::vector<int>& ACO::getBestValueHistory() const {
    return bestValuePerIteration_;
}
