#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <utility>
using namespace std;
struct info {
	string in;
	string ns;
	int output;
	bool printed = false;
};
struct element {
	bool iscompatible;
	vector<pair<string, string>>nextstatus;
};
map<pair<string, string>, element>table;
map<string, vector<info>>input;
vector<string>status; 

int numofinputs, numofoutputs, numofterms, numofstates;

string r;

void read(char* filename) {
	fstream f(filename, ios::in);
	string prefix = "";
	while (prefix != ".r") {
		f >> prefix;
		if (prefix == ".i") {
			f >> numofinputs;
		}
		else if (prefix == ".o") {
			f >> numofoutputs;
		}
		else if (prefix == ".p") {
			f >> numofterms;
		}
		else if (prefix == ".s") {
			f >> numofstates;
		}
		else if (prefix == ".r") {
			f >> r;
		}
	}
	info d;
	string cur;
	for (int i = 1; i <= numofterms; i++) { // input cur next output
		f >> d.in >> cur >> d.ns >> d.output;
		if (input.find(cur) == input.end())status.push_back(cur);
		input[cur].push_back(d);
	}
	f.close();
}
void reset() {
	for (auto& t : input) {
		for (auto& n : t.second)
			n.printed = false;
	}
}
void write_to_dot(char* filename) {
	fstream f(filename, ios::out);
	f << "digraph STG {\n" << "\trankdir=LR;\n\n";
	f << "\tINIT [shape=point];\n";
	for (auto s : input)
		f << '\t' << s.first << " [label=\"" << s.first << "\"];\n";
	f << "\tINIT -> " << r << ";\n";
	for (auto& s : status) {
		if (input.find(s) != input.end()) {
			for (auto& n : input[s]) {
				if (!n.printed) {
					f << '\t' << s << " -> " << n.ns << " [label=\"";
					int c = 0;
					for (auto& n2 : input[s]) {
						if (n2.ns == n.ns) {
							++c;
							if (c >= 2) f << ",";
							f << n2.in << "/" << n2.output;
							n2.printed = true;
						}
					}
					f << "\"];\n";
				}
			}
		}
	}
	f << "}";
	f.close();
	reset();
}

void write_to_kiss(char* filename) { //kiss_out
	fstream f(filename, ios::out);
	int num = 1;
	for (int i = 1; i <= numofinputs; i++)num *= 2;
	f << ".start_kiss\n";
	f << ".i " << numofinputs << "\n";
	f << ".o " << numofoutputs << "\n";
	f << ".p " << input.size() * num << "\n";
	f << ".s " << input.size() << "\n";
	f << ".r " << r << "\n";
	for (auto s : status) {
		if (input.find(s) != input.end()) {
			for (auto n : input[s])
				f << n.in << " " << s << " " << n.ns << " " << n.output << "\n";
		}
	}
	f << ".end_kiss";
	f.close();
}
void simplify() {
	//fill in
	for (int i = 0; i < status.size() - 1; i++) {
		for (int j = i + 1; j < status.size(); j++) {
			pair<string, string> p = make_pair(status[i], status[j]); // check output first
			pair<string, string>p2 = make_pair(status[j], status[i]); // symmetric
			vector<info> c1 = input[status[i]];
			vector<info> c2 = input[status[j]];
			bool SameOutput = true;
			for (int i = 0; i < c1.size() && SameOutput; i++) {
				if (c1[i].output != c2[i].output) SameOutput = false;
			}
			if (!SameOutput) {
				table[p].iscompatible = false;
				table[p2].iscompatible = false;
			}
			else {
				table[p].iscompatible = true;
				table[p2].iscompatible = true;
				// fill in next status
				pair<string, string>next;
				for (int i = 0; i < c1.size(); i++) {
					next = make_pair(c1[i].ns, c2[i].ns);
					table[p].nextstatus.push_back(next);
				}
			}

		}
	}
	// simplify
	bool flag = false;
	while (!flag) { 
		flag = true;
		pair<string, string>p;
		for (int i = 0; i < status.size() - 1; i++) {
			for (int j = i + 1; j < status.size(); j++) {
				p = make_pair(status[i], status[j]);
				if (table[p].iscompatible) {
					for (auto v : table[p].nextstatus) {
						if (v.first != v.second && !table[v].iscompatible) {
							flag = false;
							table[p].iscompatible = false;
							table[make_pair(p.second, p.first)].iscompatible = false;
							break;
						}
					}
				}
			}
		}
	}
	pair<string, string>p;
	bool finish = false;
	while (!finish) {
		finish = true;
		for (int i = 0; i < status.size() - 1; i++) {
			for (int j = i + 1; j < status.size(); j++) {
				p = make_pair(status[i], status[j]);
				if (table[p].iscompatible) {
					pair<string, string>p2;
					for (int k = 0; k < j; k++) {
						p2 = make_pair(status[k], status[j]);
						table[p2].iscompatible = false;
					}
					for (int k = j + 1; k < status.size(); k++) {
						p2 = make_pair(status[j], status[k]);
						table[p2].iscompatible = false;
					}
					input.erase(input.find(status[j]));
					for (auto& t : input) {
						for (auto& t2 : t.second) {
							if (t2.ns == status[j]) t2.ns = status[i];
						}
					}
					finish = false;
				}
			}
		}
	}
}
int main(int argc, char* argv[]) {
	char in[] = "input.dot";
	if (argc == 4) {
		read(argv[1]);
		write_to_dot(in); // before simplification
		simplify();
		write_to_kiss(argv[2]);
		write_to_dot(argv[3]);
	}
	else {
		cout << "parameter error!!" << endl;
	}
	return 0;
}