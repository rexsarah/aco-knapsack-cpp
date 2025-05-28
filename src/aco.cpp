#include "aco.h"
#include <algorithm> // Para std::max_element, std::min_element, std::shuffle
#include <numeric>   // Para std::iota
#include <cmath>     // Para std::pow
#include <iostream>  // Para depuração (opcional, remova em produção)

// Construtor
ACO::ACO(int numAnts, double evaporationRate, double alpha, double beta,
         int capacity, const std::vector<Item>& items, int maxIterations, unsigned int seed)
    : numAnts_(numAnts), evaporationRate_(evaporationRate), alpha_(alpha), beta_(beta),
      capacity_(capacity), items_(items), maxIterations_(maxIterations),
      seed_(seed), rng_(seed), bestValueGlobal_(0) { // Inicializa rng_ com o seed
    initializePheromones();
}

// Inicializa a matriz de feromônios
// Todos os caminhos começam com a mesma quantidade de feromônio
void ACO::initializePheromones() {
    // pheromones_[item_idx][0] = feromônio para NÃO pegar o item
    // pheromones_[item_idx][1] = feromônio para PEGAR o item
    pheromones_.assign(items_.size(), std::vector<double>(2, 0.1)); // Um valor inicial pequeno e positivo
}

// Constrói uma solução para uma formiga
// A formiga tenta preencher a mochila selecionando itens
std::vector<int> ACO::constructSolution() {
    std::vector<int> currentSolution(items_.size(), 0); // 0 = item não selecionado
    std::vector<bool> itemTaken(items_.size(), false);   // Para controlar se um item já está na mochila
    int currentWeight = 0;

    // Embaralha a ordem de consideração dos itens para introduzir aleatoriedade
    std::vector<int> itemIndices(items_.size());
    std::iota(itemIndices.begin(), itemIndices.end(), 0); // Preenche com 0, 1, ..., numItems-1
    std::shuffle(itemIndices.begin(), itemIndices.end(), rng_); // Embaralha a ordem

    // Distribuição para a decisão probabilística
    std::uniform_real_distribution<> dist(0.0, 1.0);

    // Tenta adicionar itens
    for (int itemIdx : itemIndices) {
        if (!itemTaken[itemIdx]) { // Se o item ainda não foi adicionado
            double prob_take = calculateProbability(itemIdx, 1, currentWeight); // Probabilidade de PEGAR o item

            if (dist(rng_) < prob_take) { // Se a formiga decidir tentar PEGAR o item
                if (currentWeight + items_[itemIdx].weight <= capacity_) {
                    currentSolution[itemIdx] = 1; // Pega o item
                    itemTaken[itemIdx] = true;
                    currentWeight += items_[itemIdx].weight;
                }
            }
            // Se não pegou o item (por prob_take baixa ou por não caber), ele não é pego nesta iteração
            // E o feromônio para "não pegar" pode ser reforçado mais tarde
        }
    }

    // Heurística de reparo/preenchimento: Após a construção inicial,
    // tenta adicionar itens que ainda cabem para maximizar o valor,
    // priorizando os de maior valor/peso.
    // Isso ajuda a garantir que as soluções sejam densas e viáveis.
    std::vector<std::pair<double, int>> remainingItemsSorted; // {valor_por_peso, item_idx}
    for (size_t i = 0; i < items_.size(); ++i) {
        if (currentSolution[i] == 0) { // Se o item não foi pego
            remainingItemsSorted.push_back({static_cast<double>(items_[i].value) / items_[i].weight, (int)i});
        }
    }
    std::sort(remainingItemsSorted.rbegin(), remainingItemsSorted.rend()); // Ordena do maior valor/peso para o menor

    for (const auto& p : remainingItemsSorted) {
        int itemIdx = p.second;
        if (currentWeight + items_[itemIdx].weight <= capacity_ && currentSolution[itemIdx] == 0) {
            currentSolution[itemIdx] = 1;
            currentWeight += items_[itemIdx].weight;
        }
    }

    return currentSolution;
}

// Calcula a probabilidade de escolher uma opção (pegar ou não pegar um item)
// Esta é uma regra de transição mais padrão para ACO
double ACO::calculateProbability(int itemIndex, int option, int currentWeight) {
    // Opção 1: Pegar o item
    // Se o item não couber, a probabilidade é 0
    if (option == 1 && (currentWeight + items_[itemIndex].weight > capacity_)) {
        return 0.0;
    }

    double heuristic_info = getHeuristicInformation(itemIndex);

    double numerator;
    if (option == 1) { // Numerador para PEGAR o item
        numerator = std::pow(pheromones_[itemIndex][1], alpha_) * std::pow(heuristic_info, beta_);
    } else { // Numerador para NÃO PEGAR o item
        numerator = std::pow(pheromones_[itemIndex][0], alpha_); // Heurística para não pegar é 1 (ou outra coisa)
    }

    // Denominador: Soma das atratividades para PEGAR e NÃO PEGAR
    double denominator_take = std::pow(pheromones_[itemIndex][1], alpha_) * std::pow(heuristic_info, beta_);
    double denominator_not_take = std::pow(pheromones_[itemIndex][0], alpha_);

    double totalDenominator = denominator_take + denominator_not_take;

    if (totalDenominator == 0) {
        return 0.5; // Caso de divisão por zero, retorna 0.5 (aleatório)
    }

    return numerator / totalDenominator;
}

// Retorna a informação heurística para um item (relação valor/peso)
double ACO::getHeuristicInformation(int itemIndex) const {
    if (items_[itemIndex].weight == 0) {
        return static_cast<double>(items_[itemIndex].value); // Evita divisão por zero, prioriza valor
    }
    return static_cast<double>(items_[itemIndex].value) / items_[itemIndex].weight;
}

// Atualiza os níveis de feromônio
void ACO::updatePheromones(const std::vector<std::vector<int>>& solutions) {
    // 1. Evaporação em todos os caminhos
    for (size_t i = 0; i < pheromones_.size(); ++i) {
        pheromones_[i][0] *= (1.0 - evaporationRate_); // Feromônio para não pegar
        pheromones_[i][1] *= (1.0 - evaporationRate_); // Feromônio para pegar
        // Garante que o feromônio não caia abaixo de um limiar mínimo para evitar estagnação
        pheromones_[i][0] = std::max(pheromones_[i][0], 0.001);
        pheromones_[i][1] = std::max(pheromones_[i][1], 0.001);
    }

    // 2. Depósito de feromônio pelas formigas da iteração
    // Usaremos as 5 melhores soluções da iteração para depositar feromônio
    // Uma abordagem de "Rank-based Ant System" para focar nas melhores soluções
    std::vector<std::pair<int, std::vector<int>>> viableSolutions; // {value, solution_vector}

    for (const auto& sol : solutions) {
        if (isFeasible(sol)) {
            viableSolutions.push_back({calculateValue(sol), sol});
        }
    }

    // Ordena as soluções viáveis por valor (do maior para o menor)
    std::sort(viableSolutions.rbegin(), viableSolutions.rend());

    // Deposita feromônio nas k melhores soluções
    int solutionsToDeposit = std::min((int)viableSolutions.size(), 5); // Exemplo: top 5
    for (int k = 0; k < solutionsToDeposit; ++k) {
        const std::vector<int>& sol = viableSolutions[k].second;
        int value = viableSolutions[k].first;

        // A quantidade de feromônio depositado pode ser proporcional ao valor da solução
        // ou ao seu rank. Usaremos o valor da solução.
        double pheromoneDepositAmount = static_cast<double>(value) / capacity_; // Adaptação

        for (size_t i = 0; i < sol.size(); ++i) {
            if (sol[i] == 1) { // Item foi pego
                pheromones_[i][1] += pheromoneDepositAmount;
            } else { // Item não foi pego
                pheromones_[i][0] += pheromoneDepositAmount;
            }
        }
    }

    // (Opcional: Limite superior para o feromônio para evitar que um caminho domine demais)
    // for (size_t i = 0; i < pheromones_.size(); ++i) {
    //     pheromones_[i][0] = std::min(pheromones_[i][0], 100.0); // Exemplo: limite 100
    //     pheromones_[i][1] = std::min(pheromones_[i][1], 100.0);
    // }
}

// Calcula o valor total de uma solução
int ACO::calculateValue(const std::vector<int>& solution) const {
    int value = 0;
    for (size_t i = 0; i < solution.size(); ++i) {
        if (solution[i] == 1) {
            value += items_[i].value;
        }
    }
    return value;
}

// Calcula o peso total de uma solução
int ACO::calculateWeight(const std::vector<int>& solution) const {
    int weight = 0;
    for (size_t i = 0; i < solution.size(); ++i) {
        if (solution[i] == 1) {
            weight += items_[i].weight;
        }
    }
    return weight;
}

// Verifica se uma solução é viável
bool ACO::isFeasible(const std::vector<int>& solution) const {
    return calculateWeight(solution) <= capacity_;
}

// Método principal do algoritmo ACO
std::pair<std::vector<int>, int> ACO::solve() {
    bestValueGlobal_ = 0;        // Reseta o melhor valor global para esta execução
    bestSolutionGlobal_.clear(); // Limpa a melhor solução global
    bestValuePerIteration_.clear(); // Limpa o histórico de convergência

    for (int iter = 0; iter < maxIterations_; ++iter) {
        std::vector<std::vector<int>> currentIterationSolutions; // Soluções de todas as formigas nesta iteração
        int bestValueThisIteration = 0; // Melhor valor encontrado nesta iteração

        // Cada formiga constrói uma solução
        for (int i = 0; i < numAnts_; ++i) {
            std::vector<int> antSolution = constructSolution();
            int antValue = calculateValue(antSolution);
            currentIterationSolutions.push_back(antSolution); // Adiciona para uso na atualização do feromônio

            // Se a solução da formiga é viável e melhor que a melhor global até agora
            if (isFeasible(antSolution) && antValue > bestValueGlobal_) {
                bestValueGlobal_ = antValue;
                bestSolutionGlobal_ = antSolution;
            }
            // Também monitora o melhor desta iteração para registro de convergência
            if (isFeasible(antSolution) && antValue > bestValueThisIteration) {
                bestValueThisIteration = antValue;
            }
        }

        // Se a melhor solução global ainda é 0 e não houve nenhuma solução viável melhor nesta iteração,
        // mas alguma formiga encontrou algo, use o melhor desta iteração como a referência inicial
        // (Isso é uma pequena heurística para iniciar a busca se o 0 persistir)
        if (bestValueGlobal_ == 0 && bestValueThisIteration > 0) {
             bestValueGlobal_ = bestValueThisIteration;
             // Não atualiza bestSolutionGlobal_ aqui, pois não teríamos o vetor.
             // Isso pode ser aprimorado para armazenar a melhor solução da primeira iteração.
        }


        // Atualiza os feromônios com base nas soluções geradas nesta iteração
        updatePheromones(currentIterationSolutions);

        // Armazena o melhor valor encontrado até esta iteração (globalmente) para o histórico
        bestValuePerIteration_.push_back(bestValueGlobal_);
    }

    return {bestSolutionGlobal_, bestValueGlobal_};
}

// Implementação do getter para o histórico de convergência
const std::vector<int>& ACO::getBestValueHistory() const {
    return bestValuePerIteration_;
}