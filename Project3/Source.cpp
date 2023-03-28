#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <set>
using namespace std;
int varnum, outnum, termnum;
char outputvar;
struct term {
	string val;
	int numberof1s = 0;
	bool dc;
	bool pi = true;
	set<int>n; // indicates combinate which minterm
	bool operator == (term t) { return t.val == this->val; }
	void Sub(int index) { // sub index element to "-"
		numberof1s--;
		val[index] = '-';
	}
	bool Search(int num) {
		return n.find(num) != n.end();
	}
};
bool isSame(string s1, string s2) {
	int count = 0;
	for (int i = 0; i < s1.length(); i++)
		for (int j = 0; j < s2.length(); j++)
			if (s2[j] == s1[i])++count;
	
	if (count == s1.size())return true;
	else return false;
}
class sop{
public:
	vector<string>t;
	sop& operator = (const sop& r) {
		this->t = r.t;
		return *this;
	}
	sop& operator *= (const sop& r) {
		sop res;
		for (auto s1 : t) {
			for (auto s2 : r.t) {
				string mul = s1;
				for (char c : s2) {
					bool f = true;
					for (auto Literal : mul) {
						if (Literal == c) {
							f = false;
							break;
						}
					}
					if (f)
						mul = mul + string(1, c);
				}
				//check whether exists 
				bool f = true;
				for (auto check : res.t) {
					if (check.length() == mul.length() && isSame(check, mul)) {
						f = false;
						break;
					}
				}
				if (f)
					res.t.push_back(mul);
			}
		}
		*this = res;
		return *this;
	}
};
int cal(string s) {
	int d  = 0,w=1;
	for (int i = s.size() - 1; i >= 0; i--,w*=2) d += (s[i] - '0')*w; 
	return d;
}
set<int> minterm;
set<int> dontcare;
map<int, vector<term>> matrix; // int indicates which minterm
map<int, vector<term>>mp; // int indicates numberof1s
vector<char> var;
vector<term> prime;
vector<term>terms; // Read pla file
vector<string>result;
bool isduplicate(string s) {
	for (auto t : result) {
		int c = 0;
		for (int i = 0; i < s.length(); i++) {
			if (t[i] == s[i])++c;
		}
		if (c == varnum)return true;
	}
	return false;
}
void process(int index,term t,string r ="") { // convert 
	if (index == t.val.size()) {
		if (mp.find(t.numberof1s) != mp.end()) {
			for (auto term : mp[t.numberof1s]) {
				if (term.val == r) 
					return;		
			}
		}
		t.val = r;
		mp[t.numberof1s].push_back(t);
	}
	else {
		if (t.val[index] != '-') {
			r = r + string(1,t.val[index]);
			if (t.val[index] == '1') t.numberof1s++;
			process(index + 1, t, r);
		}
		else {
			process(index + 1, t, r + string(1, '0'));
			t.numberof1s++;
			process(index + 1, t, r + string(1, '1'));
		}
	}
}
bool verify(char a, char b) {
	return (a == '-' && b == '-') || (a !='-' && b!='-');
}
void Quine_McCluskey() {
	bool flag = true;
	map<int, vector<term>>temp;
	while (flag) {
		temp.clear();
		flag = false;
		for (int i = 0; i < varnum; i++) {
			if (mp.find(i) != mp.end() && mp.find(i+1)  != mp.end() ) { // compare
				for (auto &t1 : mp[i]) { // term
					for (auto &t2 : mp[i + 1]) {
						bool pass = true;
						for (int i = 0; i < t1.val.size(); i++) {
							if (!verify(t1.val[i], t2.val[i])) {
								pass = false;
								break;
							}
						}
						if (pass) {
							vector<int> diff;
							for (int i = 0; i < t1.val.size(); i++) {
								if (t1.val[i] != t2.val[i]) {
									diff.push_back(i);
								}
							}
							if (diff.size() == 1) {
								bool check = true;
								flag = true;
								term res;
								res = t2;
								t1.pi = false;
								t2.pi = false;
								res.Sub(diff[0]);
								for (auto t : temp[res.numberof1s]) {
									if (t == res)
										check = false;
								}
								if (check) {
									for (auto c : t1.n)
										res.n.insert(c);
									res.pi = true;
									temp[res.numberof1s].push_back(res);
								}				
							}
						}
					}
				}
			}
		}
		//loop through mp to check whether there's a prime implicant or not
		for (auto v : mp) {
			for (auto t : v.second) {
				if (t.pi)
					prime.push_back(t);
			}
		}
		mp = temp; // next round
	}
}
int numberofterm(string s) {
	int len = 0;
	for (auto c : s)
		if (c != '-')++len;
	return len;
}
void Petrick() {
	//choose essential pi
	for (auto num : minterm) { 
		for (auto t : prime) {
			if (t.Search(num)) { 
				matrix[num].push_back(t);
			}
		}
	}
	set<int> dealed;
	for (auto& n : matrix) { // loop through matrix
		if (n.second.size() == 1) { // essential pi
			if (!isduplicate(n.second[0].val)) {
				result.push_back(n.second[0].val);
				for (auto t : n.second[0].n) 
					if(dontcare.find(t) ==dontcare.end())dealed.insert(t); // dont care
			}
		}
	}
	if (dealed.size() == minterm.size())
		return;
	for (auto t : dealed) matrix.erase(matrix.find(t));
	
	map<string,char>encode;
	char e = 'a';
	vector<sop>resultsop;
	for (auto col : matrix) {
		sop p;
		for (auto t : col.second) {
			if (encode.find(t.val) == encode.end()) {
				encode[t.val] = e;
				++e;
			}
			p.t.push_back(string(1, encode[t.val] ));
		}
		resultsop.push_back(p);
	}
	sop mul = resultsop[0];
	for (int i = 1; i < resultsop.size() ; i++) mul *= resultsop[i];
	//choose minimum
	int index = 0;
	for (int i = 0, min = 0x7fffffff; i < mul.t.size(); i++) {
		int len = 0;
		for (int j = 0; j < mul.t[i].length(); j++) { // 展開計算
			for (auto it = encode.begin(); it != encode.end(); it++) {
				if (it->second == (mul.t[i])[j]) {
					len += numberofterm(it->first);
					break;
				}
			}
		}
		if (len < min) {
			min = len;
			index = i;
		}
	}
	for (int i = 0; i < mul.t[index].length(); i++) {
		for (auto it = encode.begin(); it != encode.end(); it++) {
			if (it->second == (mul.t[index])[i]) 
				result.push_back(it->first);		
		}
	}
}
void output(char* filename) {
	fstream f(filename,ios::out);
	if (f.is_open()) {
		f << ".i " << varnum << endl << ".o " << outnum << endl << ".ilb ";
		for (auto t : var)
			f << t << " ";
		f << endl;
		f << ".ob " << outputvar << endl << ".p " << result.size() << endl;
		for (auto t : result)
			f << t << " 1" << endl;
		f << ".e";
		f.close();
	}
	else {
		cout << "Error" << endl;
	}
}
void input(char*filename) {
	fstream f(filename);
	string prefix;
	while (f >> prefix) {
		if (prefix == ".i") {
			f >> varnum;
		}
		else if (prefix == ".o") {
			f >> outnum;
		}
		else if (prefix == ".ilb") {
			char c;
			for (int i = 0; i < varnum; i++) {
				f >> c;
				var.push_back(c);
			}
		}
		else if (prefix == ".ob") {
			f >> outputvar;
		}
		else if (prefix == ".p") {
			f >> termnum;
			string s,v;
			for (int i = 0; i < termnum; i++) {
				term t;
				f >> s >> v;
				t.val = s;
				if (v == "-") 
					t.dc = true;
				else 
					t.dc = false;
				terms.push_back(t);
			}
		}
		else if (prefix == ".e") {
			break;
		}
	}
	f.close();
}
int main(int argc,char* argv[]){
	if (argc == 3) {
		input(argv[1]);
		for (auto t : terms)
			process(0,t);
		for (auto& c : mp) {
			for (auto& t : c.second) {
				int temp = cal(t.val);
				t.n.insert(temp);
				if (!t.dc) 
					minterm.insert(temp);	
				else 
					dontcare.insert(temp);	
			}
		}
		Quine_McCluskey();
		Petrick();
		output(argv[2]);
		int literals = 0;
		for (auto term : result) {
			for (auto literal : term)
				if (literal != '-')literals++;
		}
		cout << "Total number of terms: " << result.size() << endl;
		cout << "Total number of literals: " << literals << endl;
	}
	else {
		cout << "parameter error!" << endl;
	}
	return 0;
}