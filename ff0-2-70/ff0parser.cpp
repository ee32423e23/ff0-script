#include <bits/stdc++.h>

using namespace std;

typedef string String;
typedef vector<String> VS;

struct ExecuteException {
	String msg;
	ExecuteException() {
		
	}
	ExecuteException(String m) {
		msg = m;
	}
};

#define ERROR(msg, code) \
	do {\
		cout << "Program exited with an occured exception. ERROR CODE: " << code << endl;\
		cout << msg << endl;\
		throw ExecuteException(msg);\
	} while(false)
	
#define WARN(msg) cout << "Warning: " << msg << endl;

map<String, String> operators;

String findReflect(String reflect) {
	if(operators.find(reflect) != operators.end()) return operators[reflect];
	else return reflect; 
}

vector<VS> tokenize(String text) {
	vector<VS> vses;
	VS vs;
	String cur = "";
	for(int i = 0; i < text.length(); i++) {
		if(text[i] == '"') {
			int top = 1;
			if(cur != "") {
				vs.push_back(cur);
			}
			cur = "\"";
			while(i < text.length()) {
				i++;
				if(text[i] == '\\') {
					cur = cur + text[i];
					if(i < text.length() - 1) {
						i++;
						cur = cur + text[i];
					}
				}
				else if(text[i] == '"') {
					cur = cur + text[i];
					break;
				}
				else cur = cur + text[i];
			}
			vs.push_back(cur);
			cur = "";
		}
		else {
			if(!isspace(text[i])) {
				if(text[i] == ';' || text[i] == ':') {
					if(cur != "") vs.push_back(cur);
					if(text[i] == ':') vs[vs.size() - 1] += ":";
					cur = "";
					if(vs.size()) vses.push_back(vs);
					vs.clear();
				}
				else {
					if(text[i] == ')' || text[i] == '(' || text[i] == '[' || text[i] == ']' || text[i] == '{' || text[i] == '}'|| text[i] == ',') {
						if(cur != "") vs.push_back(cur);
						cur = "";
						cur += text[i];
						vs.push_back(cur);
						cur = "";
					}
					else cur += text[i];
				}
			}
			else {
				if(cur != "") vs.push_back(cur);
				cur = "";
			}
		}
	}
	if(cur != "") vs.push_back(cur);
	if(vs.size()) vses.push_back(vs);
	vector<VS> ret;
	for(int i = 0; i < vses.size(); i++) {
		VS retvs;
		for(int j = 0; j < vses[i].size(); j++) {
			if(vses[i][j] == "to") {
				if(j > 0 && (vses[i][j - 1] == "equals" || vses[i][j - 1] == "not-equals")) retvs[retvs.size() - 1] += " to";
				else ERROR("Need an 'equals' or a 'not-equals' before 'to'.", 1);
			}
			else if(vses[i][j] == "is") {
				retvs.push_back("is");
				if(j < vses[i].size() - 1 && (vses[i][j + 1] == "true" || vses[i][j + 1] == "false")) {
					retvs[retvs.size() - 1] += " " + vses[i][++j];
					continue;
				}
				else if(j < vses[i].size() - 1 && (vses[i][j + 1] == "assigned")) {
					if(j < vses[i].size() - 2 && (vses[i][j + 2] == "with")) retvs[retvs.size() - 1] += " assigned with";
					else ERROR("Need a 'with' after assigned.", 4);
					j++; j++;
					continue;
				}
				else if(j < vses[i].size() - 1 && (vses[i][j + 1] == "bigger" || vses[i][j + 1] == "not-bigger" || vses[i][j + 1] == "smaller" || vses[i][j + 1] == "not-smaller")) retvs[retvs.size() - 1] += " " + vses[i][++j];
				else ERROR("Need a compare-operator or a 'true' or a 'false' after 'is'.", 2);
				if(j < vses[i].size() - 1 && vses[i][j + 1] == "than") retvs[retvs.size() - 1] += " " + vses[i][++j];
				else ERROR("Need a 'than' after the compare-operator.", 3);
			}
			else retvs.push_back(vses[i][j]);
		}
		ret.push_back(retvs);
	}
	return ret;
}

int tabs;
String getTab() {
	String s = "";
	for(int i = 0; i < tabs; i++) s += "    ";
	return s;
}

String translate(VS vs) {
	stringstream result("");
	String flag = vs[0];
	if(vs[0] == "If") {
		result << getTab();
		result << "if (";
		for(int i = 1; i < vs.size() - 3; i++) {
			result << ' ' << findReflect(vs[i]);
		}
		if(vs[vs.size() - 3] != "is true") ERROR("Need an 'is true' after the expression.", 4);
		if(vs[vs.size() - 2] != ",") ERROR("Need a comma after 'is true'.", 5);
		if(vs[vs.size() - 1] != "do:") ERROR("Need a 'then:' after the if-statement condition.", 6);
		result << " ) {";
		tabs++;
	}
	else if(vs[0] == "Or") {
		result << getTab();
		result << "elseif (";
		for(int i = 1; i < vs.size() - 3; i++) {
			result << ' ' << findReflect(vs[i]);
		}
		if(vs[vs.size() - 3] != "is true") ERROR("Need an 'is true' after the expression.", 4);
		if(vs[vs.size() - 2] != ",") ERROR("Need a comma after 'is true'.", 5);
		if(vs[vs.size() - 1] != "do:") ERROR("Need a 'do:' after the or-statement condition.", 6);
		result << " ) {";
		tabs++;
	}
	else if(vs[0] == "Otherwise") {
		if(vs.size() <= 1 || vs[1] != ",") ERROR("Need a comma after 'Otherwise'.", 4);
		if(vs.size() <= 2 || vs[2] != "do:") ERROR("Need a 'do:' after the comma.", 4);
		result << getTab();
		result << "else {";
		tabs++;
	}
	else if(vs[0] == "Done") {
		tabs--;
		result << getTab();
		result << "}" << endl;
	}
	else if(vs[0] == "Repeat") {
		result << getTab();
		result << "while (";
		if(vs.size() <= 1 || vs[1] != "until") ERROR("Need an 'until' after 'Repeat'.", 4);
		for(int i = 2; i < vs.size() - 3; i++) {
			result << ' ' << findReflect(vs[i]);
		}
		if(vs[vs.size() - 3] != "is false") ERROR("Need an 'is false' after the expression.", 4);
		if(vs[vs.size() - 2] != ",") ERROR("Need a comma after 'is false'.", 5);
		if(vs[vs.size() - 1] != "do:") ERROR("Need a 'do:' after the repeat-statement condition.", 6);
		result << " ) {";
		tabs++;
	}
	else if(vs[0] == "Function") {
		result << getTab();
		result << "function ";
		if(vs.size() <= 1 || vs[1] != ",") ERROR("Need a comma after 'Function'.", 4);
		if(vs.size() <= 2 || vs[2] != "called") ERROR("Need a 'called' after the comma.", 4);
		if(vs.size() <= 3) ERROR("Need a name after the 'called'.", 4);
		else result << vs[3] << " (";
		if(vs.size() <= 4 || vs[4] != ",") ERROR("Need a comma after the name of function.", 4);
		if(vs.size() <= 5 || vs[5] != "needs") ERROR("Need a 'needs' after the comma.", 4);
		int cnt;
		if(vs.size() <= 6) ERROR("Need a number after the comma.", 4);
		else {
			if(vs[6] == "no") cnt = 0;
			else cnt = atoi(vs[6].c_str());
			if(cnt < 0) ERROR("The count of parameters can not be lower than zero.", 8);
		}
		if(vs.size() <= 7) ERROR("Need a " + (String)(cnt == 1 ? "'parameter'" : "'parameters'") + " after the number.", 4);
		else {
			if(cnt == 1) {
				if(vs[7] != "parameter") ERROR("Need a 'parameter' after the number.", 4);
			}
			else if(vs[7] != "parameters") ERROR("Need a 'parameters' after the number.", 4);
		}
		if(vs.size() <= 8 || vs[8] != ",") ERROR("Need a comma after " + (String)(cnt == 1 ? "'parameter'" : "'parameters'") + ".", 4);
		if(cnt != 0) {
			if(vs.size() <= 9 || vs[9] != "called") ERROR("Need a 'called' after the comma.", 4);
			int x = 10;
			while(cnt--) {
				if(vs.size() > x) {
					result << ' ' << vs[x];
					if(cnt != 0) result << " ,"; 
					x++;
				}
				else ERROR("Need a name.", 4);
				if(vs.size() <= x || vs[x] != ",") {
					ERROR("Need a comma after the name.", 4);
				}
				else x++;
			}
			if(vs.size() <= x || vs[x] != "do:") ERROR("Need a 'do:' after the parameter list.", 4);
			result << " ) {";
		}
		else {
			if(vs.size() <= 9 || vs[9] != "do:") ERROR("Need a 'do:' after the parameter list.", 4);
			result << " ) {";
		}
		tabs++;
	}
	else if(vs[0] == "For") {
		result << getTab();
		result << "for (";
		
		String variable, from, to;
		if(vs.size() <= 1) ERROR("Need a name after 'For'.", 7);
		variable = vs[1];
		result << ' ' << vs[1] << " ,";
		if(vs.size() <= 2 || vs[2] != "from") ERROR("Need a name after 'From'.", 8);
		
		int i;
		for(i = 3; i < vs.size(); i++) {
			if(vs[i] == "->") break;
			result << ' ' << findReflect(vs[i]);
		}
		result << " , " << variable << " !=";
		for(i = i + 1; i < vs.size() - 2; i++) {
			result << ' ' << findReflect(vs[i]); 
		}
		result << " , 1 ) {";
		
		
		if(vs[vs.size() - 2] != ",") ERROR("Need a comma after the expression.", 5);
		if(vs[vs.size() - 1] != "do:") ERROR("Need a 'do:' after the for-statement condition.", 6);
		tabs++;
	}
	else {
		result << getTab();
		if(vs.size() > 0) result << findReflect(vs[0]);
		for(int i = 1; i < vs.size(); i++) result << ' ' << findReflect(vs[i]);
	}
	return result.str();
}

String translateBlock(vector<VS> vses) {
	stringstream ss("");
	for(int i = 0; i < vses.size(); i++) ss << translate(vses[i]) << endl;
	return ss.str();
}

void initialize() {
	operators["add"] = "+"; operators["substract"] = "-"; operators["multiply"] = "*"; operators["divide"] = "/";
	operators["is bigger than"] = ">"; operators["is smaller than"] = "<";
	operators["is not-bigger than"] = "<="; operators["is not-smaller than"] = ">=";
	operators["equals to"] = "=="; operators["not-equals to"] = "!="; operators["<-"] = "="; operators["is assigned with"] = "=";
	operators["and"] = "&&"; operators["or"] = "||";
	operators["not"] = "!"; operators["substract"] = "-"; operators["multiply"] = "*"; operators["divide"] = "/";
}

int main() {
	initialize();
	String s;
	stringstream code("");
	while(getline(cin, s)) {
		code << s << ";" << endl;	
	}
	try {
		vector<VS> vses = tokenize(code.str());
		cout << translateBlock(vses) << endl;
	}
	catch(ExecuteException exc) {
		cout << "Translate Failed." << endl;
	}
}

/*
使用方式：直接运行程序，输入源代码，使用 Ctrl+Z 停止输入后即可获得输出。
cmd 格式：ff0parser < 输入文件名 > 输出文件名 

1. 运算符格式
常用运算符对照如下：
+ add			<	is bigger than			&&	and
- substract		>	is smaller than			||	or
* multiply		<=	is not-bigger than 		!=	not-equal
/ divide 		>= 	is not-smaller than		==	equal

赋值有两种写法。<- 或 is assigned with 
如 a = b 可写成 a <- b 或 a is assigned with b。

函数调用、下标可按编程语言格式，如 func(1, 2) 和 test[5].propotype。 

2. if-elseif-else 语句
格式如下： 
If <表达式> is true, do:
	<代码>
Done;
Or <表达式> is true, do:
	<代码>
Done; 
(可重复多次 Or)
Otherwise, do:
	<代码> 
Done;

等同于 Warfarin 中的

if(...) {
	...
} 
elseif(...) {
	...
}
else {
	...
}

2. Repeat 语句
格式：
Repeat until <表达式> is false, do:
	<代码>
Done;
等同于
while(...) {
	...
}

3. For 语句
For 语句的支持有限。格式：
For <变量> from <起始值> -> <终值>, do:
	<代码>
Done;
等同于：
for(<变量>, <起始值>, <变量> != <终值>, 1) {
	...
}

4. 函数定义
基本格式：
Function, called <名字>, needs <数量> parameters, [called <参数名1>, <参数名2>, ...,] do:
	<代码>
Done;

注意：
如果参数数量是 0 可以写成 no；
遵循英文复数语法，当数量为 1 时 parameter 不加 s，为 0 或为其他不为 1 的数时要加 s，例：
Function, called Z, needs 0 parameters, do: 
Function, called A, needs 1 parameter, called p1, do:
Function, called B, needs 2 parameters, called p1, p2, do: 
...

5. 其他
一行里面可以写多条语句，但是必须用分号 ; 分割。
转化成的代码是 Warfarin 格式。如果要转成 JavaScript 可以使用 Warfarin 虚拟机自带的反编译功能反编译成 JavaScript 代码。
All rights are reserved. 
*/
