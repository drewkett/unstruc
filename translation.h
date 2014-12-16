#include <vector>
#include <string>

struct Name;
struct Grid;

struct TranslationTable {
	std::vector <Name *> names;
	std::vector <int> index;
	TranslationTable(int n) : names(0), index(n,-1) {};
};

void ReadTranslationFile(std::string &filename, TranslationTable * transt);
void applyTranslation(Grid &grid, TranslationTable * transt);
