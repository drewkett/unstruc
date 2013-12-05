#include <vector>

struct Name;
struct Grid;

struct TranslationTable {
	std::vector <Name *> names;
	std::vector <int> index;
	TranslationTable(int n) : names(0), index(n,-1) {};
};

TranslationTable * ReadTranslationFile(char * filename, int n_blocks);
void applyTranslation(Grid * grid, TranslationTable * transt);
