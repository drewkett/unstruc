#ifndef TRANSLATION_H_896B6D75_C1A2_4956_8FC4_E811C8F300EA
#define TRANSLATION_H_896B6D75_C1A2_4956_8FC4_E811C8F300EA

#include "grid.h"

#include <vector>
#include <string>

struct Name;
struct Grid;

struct TranslationTable {
	std::vector <Name> names;
	std::vector <int> index;
	TranslationTable(int n) : names(0), index(n,-1) {};
};

void ReadTranslationFile(std::string &filename, TranslationTable &transt);
void applyTranslation(Grid &grid, TranslationTable &transt);

#endif