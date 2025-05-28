#include "aco.h"
#include <algorithm> // std::max_element, std::min_element, std::shuffle
#include <numeric>   // std::iota
#include <cmath>     // std::pow
#include <iostream>  // std::cout, std::endl (opcional)
#include <climits> // Para INT_MAX

ACO::ACO(int numAnts, double evaporationRate, double alpha, double beta,
         int capacity, const std::vector<Item>& items, int maxIterations, unsigned int seed)
    : numAnts_(numAnts), evaporationRate_(evaporationRate), alpha_(alpha), beta_(beta),
      capacity_(capacity), items_(items), maxIterations_(maxIterations),
      seed_(seed), rng_(seed), bestValueGlobal_(0) {
    initializePheromones();
}

void ACO::initializePheromones() {
    pheromones_.assign(items_.size(), std::vector<double>(2, 0.1));
}

std::vector<int> ACO::constructSolution() {
    std::vector<int> currentSolution(items_.size(), 0);
    std::vector<bool> itemTaken(items_.size(), false);
    int currentWeight = 0;

    std::vector<int> itemIndices(items_.size());
    std::iota(itemIndices.begin(), itemIndices.end(), 0);
    std::shuffle(itemIndices.begin(), itemIndices.end(), rng_);

    std::uniform_real_distribution<> dist(0.0, 1.0);

    for (int itemIdx : itemIndices) {
        if (!itemTaken[itemIdx]) {
            double prob_take = calculateProbability(itemIdx, 1, currentWeight);

            if (dist(rng_) < prob_take) {
                if (currentWeight + items_[itemIdx].weight <= capacity_) {
                    currentSolution[itemIdx] = 1;
                    itemTaken[itemIdx] = true;
                    currentWeight += items_[itemIdx].weight;
                }
            }
        }
    }

    std::vector<std::pair<double, int>> remainingItemsSorted;
    for (size_t i = 0; i < items_.size(); ++i) {
        if (currentSolution[i] == 0) {
            remainingItemsSorted.push_back({static_cast<double>(items_[i].value) / items_[i].weight, (int)i});
        }
    }
    std::sort(remainingItemsSorted.rbegin(), remainingItemsSorted.rend());

    for (const auto& p : remainingItemsSorted) {
        int itemIdx = p.second;
        if (currentWeight + items_[itemIdx].weight <= capacity_ && currentSolution[itemIdx] == 0) {
            currentSolution[itemIdx] = 1;
            currentWeight += items_[itemIdx].weight;
        }
    }

    return currentSolution;
}

double ACO::calculateProbability(int itemIndex, int option, int currentWeight) {
    if (option == 1 && (currentWeight + items_[itemIndex].weight > capacity_)) {
        return 0.0;
    }

    double heuristic_info = getHeuristicInformation(itemIndex);

    double numerator = (option == 1)
        ? std::pow(pheromones_[itemIndex][1], alpha_) * std::pow(heuristic_info, beta_)
        : std::pow(pheromones_[itemIndex][0], alpha_);

    double denominator_take = std::pow(pheromones_[itemIndex][1], alpha_) * std::pow(heuristic_info, beta_);
    double denominator_not_take = std::pow(pheromones_[itemIndex][0], alpha_);
    double totalDenominator = denominator_take + denominator_not_take;

    return (totalDenominator == 0.0) ? 0.5 : (numerator / totalDenominator);
}

double ACO::getHeuristicInformation(int itemIndex) const {
    return (items_[itemIndex].weight == 0)
        ? static_cast<double>(items_[itemIndex].value)
        : static_cast<double>(items_[itemIndex].value) / items_[itemIndex].weight;
}

void ACO::updatePheromones(const std::vector<std::vector<int>>& solutions) {
    for (size_t i = 0; i < pheromones_.size(); ++i) {
        pheromones_[i][0] *= (1.0 - evaporationRate_);
        pheromones_[i][1] *= (1.0 - evaporationRate_);
        pheromones_[i][0] = std::max(pheromones_[i][0], 0.001);
        pheromones_[i][1] = std::max(pheromones_[i][1], 0.001);
    }

    std::vector<std::pair<int, std::vector<int>>> viableSolutions;
    for (const auto& sol : solutions) {
        if (isFeasible(sol)) {
            viableSolutions.push_back({calculateValue(sol), sol});
        }
    }

    std::sort(viableSolutions.rbegin(), viableSolutions.rend());

    int solutionsToDeposit = std::min((int)viableSolutions.size(), 5);
    for (int k = 0; k < solutionsToDeposit; ++k) {
        const std::vector<int>& sol = viableSolutions[k].second;
        int value = viableSolutions[k].first;

        double pheromoneDepositAmount = static_cast<double>(value) / capacity_;

        for (size_t i = 0; i < sol.size(); ++i) {
            if (sol[i] == 1) {
                pheromones_[i][1] += pheromoneDepositAmount;
            } else {
                pheromones_[i][0] += pheromoneDepositAmount;
            }
        }
    }
}

int ACO::calculateValue(const std::vector<int>& solution) const {
    int value = 0;
    for (size_t i = 0; i < solution.size(); ++i) {
        if (solution[i] == 1) {
            value += items_[i].value;
        }
    }
    return value;
}

int ACO::calculateWeight(const std::vector<int>& solution) const {
    int weight = 0;
    for (size_t i = 0; i < solution.size(); ++i) {
        if (solution[i] == 1) {
            weight += items_[i].weight;
        }
    }
    return weight;
}

bool ACO::isFeasible(const std::vector<int>& solution) const {
    return calculateWeight(solution) <= capacity_;
}

std::tuple<std::vector<int>, int, int> ACO::solve() {
    bestValueGlobal_ = 0;
    bestSolutionGlobal_.clear();
    bestValuePerIteration_.clear();

    int worstValueGlobal = INT_MAX;  // Inicializa com valor alto para achar o mínimo viável

    for (int iter = 0; iter < maxIterations_; ++iter) {
        std::vector<std::vector<int>> currentIterationSolutions;
        int bestValueThisIteration = 0;

        for (int i = 0; i < numAnts_; ++i) {
            std::vector<int> antSolution = constructSolution();
            int antValue = calculateValue(antSolution);
            currentIterationSolutions.push_back(antSolution);

            if (isFeasible(antSolution)) {
                if (antValue > bestValueGlobal_) {
                    bestValueGlobal_ = antValue;
                    bestSolutionGlobal_ = antSolution;
                }
                if (antValue > bestValueThisIteration) {
                    bestValueThisIteration = antValue;
                }
                if (antValue < worstValueGlobal) {
                    worstValueGlobal = antValue;
                }
            }
        }

        if (bestValueGlobal_ == 0 && bestValueThisIteration > 0) {
            bestValueGlobal_ = bestValueThisIteration;
        }

        updatePheromones(currentIterationSolutions);
        bestValuePerIteration_.push_back(bestValueGlobal_);
    }

    // Se não encontrou solução viável, define pior valor como zero para evitar INT_MAX no retorno
    if (worstValueGlobal == INT_MAX) {
        worstValueGlobal = 0;
    }

    return {bestSolutionGlobal_, bestValueGlobal_, worstValueGlobal};
}

const std::vector<int>& ACO::getBestValueHistory() const {
    return bestValuePerIteration_;
}
