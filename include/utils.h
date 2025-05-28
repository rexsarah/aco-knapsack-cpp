#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <utility>

struct Item {
    int id;
    int value;
    int weight;
};

// Função para ler a instância do arquivo
std::pair<int, std::vector<Item>> readKnapsackInstance(const std::string& filePath);

#endif // UTILS_H
