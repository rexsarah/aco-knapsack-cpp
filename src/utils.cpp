#include "utils.h"
#include <fstream>
#include <iostream>

std::pair<int, std::vector<Item>> readKnapsackInstance(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filePath << std::endl;
        return {0, {}};
    }

    int numItems = 0;
    file >> numItems;

    int capacity = 0;
    file >> capacity;

    std::vector<Item> items(numItems);
    for (int i = 0; i < numItems; ++i) {
        items[i].id = i;
        file >> items[i].value >> items[i].weight;
    }

    file.close();
    return {capacity, items};
}
