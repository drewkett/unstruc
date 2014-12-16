#include "translation.h"

#include "error.h"
#include "grid.h"
#include "element.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>

void ReadTranslationFile(std::string &filename, TranslationTable &tt) {
	std::ifstream f;
	Name * name;
	std::string line, s;
	std::cerr << "Reading Translation File '" << filename << "'" << std::endl;
	f.open(filename.c_str(),std::ios::in);
	if (!f.is_open()) Fatal("Could not open file");
	while (getline(f,line)) {
		name = new Name();
		name->dim = 2;
		std::istringstream iss(line);
		iss >> name->name;
		tt.names.push_back(name);
		std::cerr << name->name;
		while (! iss.eof()) {
			iss >> s;
			std::cerr << " " << s;
			tt.index[atoi(s.c_str())] = tt.names.size()-1;
		}
		std::cerr << std::endl;
	}
	f.close();
}

void applyTranslation(Grid &grid, TranslationTable &transt) {
	Element *e;
	int offset = grid.names.size();
	for (int i=0; i < transt.names.size(); i++) {
		grid.names.push_back(transt.names[i]);
	}
	for (int i=0; i < transt.index.size(); i++) {
		if (transt.index[i] == -1) {
			transt.index[i] = i;
		} else {
			transt.index[i] += offset;
		}
	}
	for (int i = 0; i < grid.elements.size(); i++) {
		e = grid.elements[i];
		if (!e) continue;
		if (e->name_i != -1) e->name_i = transt.index[e->name_i];
	}
}
