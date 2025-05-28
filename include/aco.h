#ifndef ACO_H
#define ACO_H

#include <vector>    // Para std::vector
#include <random>    // Para std::mt19937 e std::uniform_real_distribution
#include <utility>   // Para std::pair

#include "utils.h"   // Assumindo que struct Item está definido aqui

class ACO {
public:
    // Construtor
    ACO(int numAnts, double evaporationRate, double alpha, double beta,
        int capacity, const std::vector<Item>& items, int maxIterations, unsigned int seed);

    // Método principal para resolver o problema da mochila
    std::tuple<std::vector<int>, int, int> solve();

    // Getter para o histórico do melhor valor por iteração
    const std::vector<int>& getBestValueHistory() const;

private:
    // Parâmetros do algoritmo
    int numAnts_;
    double evaporationRate_;
    double alpha_;
    double beta_;
    int capacity_;
    std::vector<Item> items_;
    int maxIterations_;

    // Estruturas de dados internas
    // Matriz de feromônios: pheromones_[item_idx][1] = feromônio para PEGAR o item
    //                    pheromones_[item_idx][0] = feromônio para NÃO PEGAR o item
    std::vector<std::vector<double>> pheromones_;

    // Gerador de números aleatórios
    std::mt19937 rng_;
    unsigned int seed_;

    // Melhor solução encontrada por esta instância do ACO
    int bestValueGlobal_;
    std::vector<int> bestSolutionGlobal_;

    // Histórico de convergência
    std::vector<int> bestValuePerIteration_;

    // Métodos auxiliares
    void initializePheromones();

    // Constrói uma solução para uma formiga, adicionando itens até a mochila estar cheia ou não caber mais nada
    std::vector<int> constructSolution();

    // Calcula a probabilidade de escolher um item dado seu estado (pegar ou não pegar)
    double calculateProbability(int itemIndex, int option, int currentWeight); // option: 0=not_take, 1=take

    // Atualiza os níveis de feromônio após cada iteração
    void updatePheromones(const std::vector<std::vector<int>>& solutions);

    // Avalia uma solução: calcula valor e peso
    int calculateValue(const std::vector<int>& solution) const;
    int calculateWeight(const std::vector<int>& solution) const;
    bool isFeasible(const std::vector<int>& solution) const;

    // Função heurística (visibilidade) para um item
    double getHeuristicInformation(int itemIndex) const;
};

#endif // ACO_H