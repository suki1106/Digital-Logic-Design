#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <queue>
#include <set>
#include <map>
using namespace std;
int numvar,numter;
vector<char> var;
vector<string> term;
vector<char>choose;
struct node{
    node* left = NULL;
    node* right = NULL;
    char var;
    int index;
};
node ONE,ZERO;
void Match(node* target,node*rootPtr,node* change){
    queue<node*> q;
    q.push(rootPtr);
    set<int> s;
    while(!q.empty()){
        node* ptr = q.front();
        q.pop();
        if(ptr->index >= target->index)
            continue;
        if(s.find(ptr->index) != s.end())
            continue;
        s.insert(ptr->index);
        if(ptr->left == target)
            ptr->left = change;
        if(ptr->right ==target)
            ptr->right = change;
        if(ptr->left )
            q.push(ptr->left);
        if(ptr->right)
            q.push(ptr->right);
    }
}
void remove_redundant(node*rootPtr){
    vector<node*>cmp;
    while(1){
        int c1=0,c2=0;
        queue<node*>q;
        q.push(rootPtr);
        set<int> vis;
        map<char,vector<node*>>mp;
        while(!q.empty()){
            node* c = q.front();
            q.pop();
            if(vis.find(c->index) != vis.end())
                continue;
            vis.insert(c->index);
            if(c->left && c->left->index !=0 && c->left->index != pow(2,numvar))
                q.push(c->left);
            if(c->right&& c->right->index !=0 && c->right->index != pow(2,numvar))
                q.push(c->right);
            if(mp.find(c->var) == mp.end()){
                vector<node*> v;
                mp[c->var] = v;
        }
            mp[c->var].push_back(c);
        }
        for(auto& c : mp){ // c means each layer
            if(c.second.size() >= 2){
                vector<node*> cmp = c.second;
                bool find = true;
                while(find){
                    find =  false;
                    for(int i = 0 ; i<= cmp.size()-2;i++){
                        for(int j = i+1;j<=cmp.size()-1;j++){
                            node* f = cmp[i];
                            node* s = cmp[j];
                            if(f->left == s->left && f->right == s->right){
                                ++c1;
                                find = true;
                                cmp.erase(cmp.begin() + j);
                                Match(s,rootPtr,f);
                                delete s;
                            }
                        }
                        if(find)
                            break;   
                    }
                }
            }
        }
        queue<node*> s;
        s.push(rootPtr);
        set<int> s1;
        while(!s.empty()){
            node* cur = s.front();
            s.pop();
            if(s1.find(cur->index) !=s1.end())
                continue;
            s1.insert(cur->index);
            if(cur->left)
                s.push(cur->left);
            if(cur->right)
                s.push(cur->right);
            if(cur->left != NULL&&cur->right!=NULL &&cur->left == cur->right){
                ++c2;
                Match(cur,rootPtr,cur->left);
                delete cur;
            }
        }
        if(c1 == 0 && c2 == 0)
            break;
    }

}
bool find(char target){
    for(auto c : choose){
        if(c== target)
            return true;
    }
    return false;
}
bool check(){
    int i=  0 ; 
    for(auto s : term){
        for(i = 0 ; i <s.size();i++){
            if(s[i] == '-'){
                continue;
            }else if(s[i] == '1'){
                if(!find(var[i]))
                    break;
            }else{
                if(find(var[i]))
                    break;
            }
        }
        if(i == s.size() )
            return true;
    }
    return false;
}

void output(node*rootPtr,string filename){
    ofstream f(filename);
    if(filename == "robdd.dot")
        f << "digraph ROBDD {\n";
    else
        f << "digraph OBDD {\n";
    char var = rootPtr->var;
    queue<node*>q;
    q.push(rootPtr);
    set<int> v;
    map<char,vector<int> >mp;
    while(!q.empty()){
        node*ptr = q.front();
        q.pop();
        if(ptr->left && ptr->left->index != 0 && ptr->left->index != pow(2,numvar) && v.find(ptr->left->index) == v.end() ){
            q.push(ptr->left);
            v.insert(ptr->left->index);
        }
        if(ptr->right&& ptr->right->index != 0 && ptr->right->index != pow(2,numvar) && v.find(ptr->right->index) == v.end()){
            q.push(ptr->right);
            v.insert(ptr->right->index);
        }
        if(mp.find(ptr->var) == mp.end()){
            vector<int> v;
            mp[ptr->var] = v;
        }
        mp[ptr->var].push_back(ptr->index);
    }
    for(auto& v : mp){
        f << "  {rank=same ";
        for(auto index : v.second){
            f << index << " ";
        }
        f << "}\n";
    }
    f << "   0 [label=0, shape=box]\n";
    q.push(rootPtr);
    set<int> vis;
    while(!q.empty()){
        node* ptr = q.front();
        q.pop();
        if(vis.find(ptr->index) != vis.end())
            continue;
        vis.insert(ptr->index);
        if(ptr->left)
            q.push(ptr->left);
        if(ptr->right)
            q.push(ptr->right);
        if(ptr->index !=0 && ptr->index != pow(2,numvar) )
            f << "   " << ptr->index <<" [label=" << "\"" << ptr->var << "\"]\n"; 
    }
    f <<"   " << pow(2,numvar) << " [label=1, shape=box]\n\n";
    q.push(rootPtr);
    vis.clear();
    while(!q.empty()){
        node* ptr = q.front();
        q.pop();
        if(vis.find(ptr->index) != vis.end())
            continue;
        vis.insert(ptr->index);
        if(ptr->left){
            q.push(ptr->left);
            f <<"   " <<ptr->index << " -> "   << ptr->left->index << " [label=\"0\",style=dotted]\n";
        }
        if(ptr->right){
            q.push(ptr->right);
            f <<"   " <<ptr->index << " -> "   << ptr->right->index << " [label=\"1\",style=solid]\n";
        }
    }
    f << "}\n";
    f.close();
}
void input(char* filename){
    ifstream f(filename);
    string s,temp;
    char v;
    while(f >> s){
        if(s == ".i"){
            f >> numvar;
        }else if(s==".ilb"){
            for(int i = 0 ; i < numvar;i++){
                f >> v;
                var.push_back(v);
            }
        }else if(s==".p"){
            f >> numter;
            for(int i = 0; i < numter;i++){
                f >> temp;
                term.push_back(temp);
                f >> v;
            }
        }else if(s==".e"){
            break;
        }
    }
    f.close();
}
int main(int argc ,char *argv[]){
    if(argc == 2){
        input(argv[1]);
        ONE.var = '1';
        ZERO.var = '0';
        ONE.index = pow(2,numvar);
        ZERO.index = 0;
        node* rootPtr = (node*) new node;
        rootPtr->var = var[0];
        rootPtr->index = 1;
        for(int i = 0 ; i < pow(2,numvar);i++){
            int p =  i ;
            node* curPtr = rootPtr;
            for(int i = 0;i<numvar;i++){
                if(p&1){ // choose 
                    choose.push_back(var[i]);
                    if(i == numvar-1)
                        break;
                    if(curPtr->right == NULL){
                        node* newPtr =(node*) new node;
                        newPtr->var = var[i+1];
                        newPtr->index = curPtr->index * 2 +1 ; 
                        curPtr->right = newPtr;
                    }
                    curPtr = curPtr->right;
                }else{
                    if(i == numvar-1)
                        break;
                    if(curPtr->left == NULL){
                        node* newPtr =(node*) new node;
                        newPtr->var = var[i+1];
                        newPtr->index = curPtr->index * 2; 
                        curPtr->left = newPtr;
                    }
                    curPtr = curPtr->left;
                }
                p>>=1;
            }
            if(check()){
                if(find(curPtr->var))
                    curPtr->right = &ONE;
                else
                    curPtr->left = &ONE;
            }else{
                if(find(curPtr->var))
                    curPtr->right = &ZERO;
                else
                    curPtr->left = &ZERO;
            }
            choose.clear();
        }
        output(rootPtr,"obdd.dot");
        remove_redundant(rootPtr);
        output(rootPtr,"robdd.dot");
    }else{
        cout << "parameter error!" << endl;
    }
    return 0;
}