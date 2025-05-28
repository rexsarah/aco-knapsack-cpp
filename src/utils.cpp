#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

std::pair<int, std::vector<Item>> readKnapsackInstance(const std::string& filename) {

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
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