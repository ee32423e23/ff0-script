#include <bits/stdc++.h>

using namespace std;

int main() {
	vector<string> vs;
	string s;
	while(getline(cin, s)) vs.push_back(s);
	for(int i = 0; i < vs.size(); i++) {
		cout << "\"";
		for(int j = 0; j < vs[i].length(); j++) {
			if(vs[i][j] == '\"') cout << "\\\"";
			else if(vs[i][j] == '\\') cout << "\\\\";
			else cout << vs[i][j];
		}
		cout << "\\n\"" << endl;
	}
	cout << ";" << endl;
	return 0;
}
