#include <bits/stdc++.h>
#include <windows.h>

using namespace std;

const string VERSION = "Warfarin script 1.6";

bool zeroTrue = false;
bool cli = false;
bool outputSym = true;
bool outputExpr = false;

typedef int Ref;
typedef int ValueType;
typedef string String;
typedef vector<String> VS;

VS EMPTY;

const int ARR_GROW = 1024 * 256;

map<String, int> bindPower;

vector<String> callStack;
map<String, VS> funcTable;
map<String, VS> paramTable;

void registFunc(String, VS, VS);

const string opchar = "+-*/!=><%&|@^";
const string spliter = "+-*/=><:;!&|%@^,";

int tabcnt = 0;

stringstream codeStream("");

String trim(String s) {
	int x = 0, y = s.length() - 1;
	while(isspace(s[x])) x++;
	while(isspace(s[y])) y--;
	return s.substr(x, y - x + 1);
}

String allTab() {
	String r = "";
	for(int i = 0; i < tabcnt; i++) r += "    ";
	return r;
}

struct ExecuteException {
	String msg;
};
struct DenyExecute {
	String msg;
};

int hashCnt;
map<String, int> hashValue;
int getHash(String s) {
	if(hashValue.find(s) == hashValue.end()) hashCnt++, hashValue[s] = hashCnt;
	return hashValue[s] + 1024 * 252;
}

stringstream debugMsg("");

#ifdef DEBUG
#define DEBUG_OUTPUT cout << allTab() << " "
#else
#define DEBUG_OUTPUT debugMsg << allTab() << " "
#endif

#define NEXTL DEBUG_OUTPUT << "======================" << __LINE__ << " run done ============================" << endl;
#define BEGINL DEBUG_OUTPUT << __func__ << " {" << endl; tabcnt++;
#define ENDEDL tabcnt--; DEBUG_OUTPUT << "}" << endl;

#define SYNTAX_ERR(msg) \
	do {\
		cout << "SyntaxError: " << msg << endl;\
		throw ExecuteException();\
	} while(0)

#define RUN_ERR(msg) \
	do {\
		cout << "RuntimeError: " << msg << endl;\
		throw ExecuteException();\
	} while(0)

#define STOP_EXEC(msg) \
	do {\
		cout << "DenyExecute: " << msg << endl;\
		throw DenyExecute();\
	} while(0)

#define WARNING(msg) \
	do {\
		cout << "Warning: " << msg << endl;\
	} while(0)

#define add(s) push_back(s)

#define ENUM_VS(vs, i) for(int i = 0; i < vs.size(); i++)

#define MIDDLE(str) (str.length() >= 2 ? str.substr(1, str.length() - 2) : str)

enum {
    T_NUMBER, T_STRING, T_OBJECT, T_FUNCTION,
    T_REF, T_TRUE, T_FALSE, T_NULL, T_UNKNOWN, T_UNDEF, T_FILE
};

String typeName[] = {
	"Number", "String", "Object", "Closure",
	"Reference", "Bool", "Bool", "Null", "Unknown", "Undefined", "File"
};

enum {
    PUBLIC, PRIVATE
};

struct Object {
	String toString() {
		return "[Unknown Object]";
	}
};

struct Closure {
	String toString() {
		return "[Unknown Closure]";
	}
};

String parseNum(double);

int nameCnt;
String makeName() {
	String ret = "inner";
	ret += parseNum(nameCnt++);
	return ret;
}

struct Value {
	double numVal;
	String strVal;
	Object objVal;
	String funVal;
	Ref refVal;

	Value() {
		type = T_UNDEF;
	}

	ValueType type;
	String toString() {
		if(type == T_NUMBER) return parseNum(numVal);
		else if(type == T_STRING) return strVal;
		else if(type == T_OBJECT) return objVal.toString();
		else if(type == T_FUNCTION) return "" + funVal + "";
		else if(type == T_REF) return "$" + parseNum(refVal);
		else if(type == T_TRUE) return "true";
		else if(type == T_FALSE) return "false";
		else if(type == T_NULL) return "null";
		else return "undefined";
	}

	static Value makeBool(bool val) {
		Value v;
		v.type = val ? T_TRUE : T_FALSE;
		return v;
	}

	static Value makeNumber(double num) {
		Value v;
		v.type = T_NUMBER, v.numVal = num;
		return v;
	}

	static Value makeString(String str) {
		Value v;
		v.type = T_STRING, v.strVal = str;
		return v;
	}

	static Value makeObject(Object obj) {
		Value v;
		v.type = T_OBJECT, v.objVal = obj;
		return v;
	}

	static Value makeFunction(String cls) {
		Value v;
		v.type = T_FUNCTION, v.funVal = cls;
		return v;
	}

	static Value makeRef(Ref ref) {
		Value v;
		v.type = T_REF, v.refVal = ref;
		return v;
	}
	
	static Value makeFile(String file) {
		Value v;
		v.type = T_FILE;
		return v;
	}

	static Value makeNull() {
		Value v;
		v.type = T_NULL;
		return v;
	}

	Value operator + (Value & v) {
		if(type == T_STRING || v.type == T_STRING) return Value::makeString(strVal + v.strVal);
		else if(type == T_NUMBER) {
			if(v.type != T_NUMBER) RUN_ERR(" can not add " + typeName[v.type] + " to a number.");
			return Value::makeNumber(v.numVal + numVal);
		} else if(type == T_REF) {
			if(v.type != T_NUMBER) RUN_ERR(" can not add " + typeName[v.type] + " to an reference.");
			return Value::makeRef(v.numVal + refVal);
		} else RUN_ERR(" type " + typeName[v.type] + " doesn't support operate '+'.");
	}

	Value operator - (Value & v) {
		if(type == T_NUMBER) {
			if(v.type != T_NUMBER) RUN_ERR(" can not substract " + typeName[v.type] + " to a number.");
			return Value::makeNumber(numVal - v.numVal);
		} else if(type == T_REF) {
			if(v.type != T_NUMBER) RUN_ERR(" can not substract " + typeName[v.type] + " to an reference.");
			return Value::makeRef(numVal - v.numVal);
		} else RUN_ERR(" type " + typeName[v.type] + " doesn't support operate '-'.");
	}

	Value operator * (Value & v) {
		if(type == T_NUMBER) {
			if(v.type != T_NUMBER) RUN_ERR(" can not multiply " + typeName[v.type] + " to a number.");
			return Value::makeNumber(numVal * v.numVal);
		} else RUN_ERR(" type " + typeName[v.type] + " doesn't support operate '*'.");
	}

	Value operator / (Value & v) {
		if(type == T_NUMBER) {
			if(v.type != T_NUMBER) RUN_ERR(" can not divide " + typeName[v.type] + " to a number.");
			return Value::makeNumber(numVal / v.numVal);
		} else RUN_ERR(" type " + typeName[v.type] + " doesn't support operate '/'.");
	}

	Value operator > (Value & v) {
		if(type == T_NUMBER) {
			if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.");
			return Value::makeBool(numVal > v.numVal);
		} else RUN_ERR(" type " + typeName[v.type] + " doesn't support operate '>'.");
	}

	Value operator < (Value & v) {
		if(type == T_NUMBER) {
			if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.");
			return Value::makeBool(numVal < v.numVal);
		} else RUN_ERR(" type " + typeName[v.type] + " doesn't support operate '/'.");
	}

	Value operator >= (Value & v) {
		if(type == T_NUMBER) {
			if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.");
			return Value::makeBool(numVal >= v.numVal);
		} else RUN_ERR(" type " + typeName[v.type] + " doesn't support operate '>='.");
	}

	Value operator <= (Value & v) {
		if(type == T_NUMBER) {
			if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.");
			return Value::makeBool(numVal <= v.numVal);
		} else RUN_ERR(" type " + typeName[v.type] + " doesn't support operate '/'.");
	}
	
	Value operator % (Value & v) {
		if(type == T_NUMBER) {
			if(v.type != T_NUMBER) RUN_ERR(" can not mod " + typeName[v.type] + " with a number.");
			return Value::makeNumber(fmod(v.numVal, numVal));
		} else RUN_ERR(" type " + typeName[v.type] + " doesn't support operate '%'.");
	}

	Value operator == (Value & v) {
		if(type == T_STRING || v.type == T_STRING) {
			return Value::makeBool(toString() == v.toString());
		} else if(type == T_NUMBER) {
			if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.");
			return Value::makeBool(numVal == v.numVal);
		} else if(type == T_REF) {
			return Value::makeBool(refVal == v.refVal);
		} else return Value::makeBool(toString() == v.toString() && type == v.type);
	}

	Value operator != (Value & v) {
		if(type == T_STRING || v.type == T_STRING) {
			return Value::makeBool(toString() != v.toString());
		} else if(type == T_NUMBER) {
			if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.");
			return Value::makeBool(numVal != v.numVal);
		} else if(type == T_REF) {
			return Value::makeBool(refVal != v.refVal);
		} else return Value::makeBool(toString() != v.toString() || type != v.type);
	}

	Value operator && (Value & v) {
		if(v.type == T_NUMBER) {
			if(type == T_TRUE) return Value::makeBool(true);
			else if(type == T_NUMBER) return Value::makeBool(v.numVal && numVal);
		} else if(type == T_TRUE) {
			if(type == T_TRUE) return Value::makeBool(true);
			else if(type == T_NUMBER) return Value::makeBool(1 && numVal);
		} else return Value::makeBool(false);
	}

	Value operator || (Value & v) {
		if(v.type == T_NUMBER) {
			if(type == T_TRUE) return Value::makeBool(true);
			else if(type == T_NUMBER) return Value::makeBool(v.numVal || numVal);
		} else if(type == T_TRUE) {
			return Value::makeBool(true);
		} else return Value::makeBool(false);
	}

	Value operator ! () {
		if(type == T_NULL) return Value::makeBool(true);
		else if(type == T_FALSE) return Value::makeBool(true);
		else if(type == T_NUMBER && numVal == 0) return Value::makeBool(true);
		else return Value::makeBool(false);
	}
};

bool isFalse(Value v) {
	return (!v).type == T_TRUE;
}

typedef vector<Value> Heap;
typedef map<String, Value> Scope;

Heap EMPTY_V;
Heap heap;
Scope scope;

void setRefVal(Value, Value);

Value getVarVal(String name) {
	for(int i = callStack.size() - 1; i >= 0; i--) {
		String newname = callStack[i] + name;
		if(scope.find(newname) == scope.end()) continue;
		else return scope[newname];
	}
	SYNTAX_ERR("variable " + name + " is not declared.");
	scope[callStack[callStack.size() - 1] + name] = Value();
	return scope[callStack[callStack.size() - 1] + name];
}

void setVarVal(String name, Value v) {
	for(int i = callStack.size() - 1; i >= 0; i--) {
		String newname = callStack[i] + name;
		if(scope.find(newname) != scope.end()) scope[newname] = v;
		else continue;
	}
	scope[callStack[callStack.size() - 1] + name] = v;
}

bool includeChar(String s, char c) {
	for(int i = 0; i < s.length(); i++) if(s[i] == c) return true;
	return false;
}

String joinString(VS vs, String spaces = "") {
	String r = "";
	ENUM_VS(vs, i) r += vs[i] + spaces;
	return r;
}

VS subVS(VS vs, int from, int to) {
	VS result;
	to = (to > vs.size()) ? vs.size() : to;
	for(int i = from; i < to; i++) result.add(vs[i]);
	return result;
}

Value getRefVal(Value v, Value locate) {
	DEBUG_OUTPUT << "getRefVal: " << v.toString() << " @ " << locate.toString() << endl;
	if(v.type != T_STRING && v.type != T_REF) RUN_ERR(" can not get reference of '" + typeName[v.type] + "'");
	if(locate.type != T_NUMBER && locate.type != T_STRING) RUN_ERR(" can not use '" + typeName[v.type] + "' as position" );
	
	if(locate.type == T_STRING) locate = Value::makeNumber(getHash(locate.strVal));
	
	if(v.type == T_STRING) {
		int loc = locate.numVal;
		if(loc < 0 || loc >= v.strVal.length()) RUN_ERR(" index " + locate.toString() + " out of memory size");
		String ret = "";
		ret += v.strVal[loc];
		return Value::makeString(ret);
	} else {
		int position = v.refVal + locate.numVal;
		if(position < 0 || position >= heap.size()) RUN_ERR(" index " + locate.toString() + " out of memory size");
		return heap[position];
	}

	return Value::makeNull();
}

void setRefVal(Value v, Value setVal) {
	DEBUG_OUTPUT << "setRefVal: " << v.toString() << " to " << setVal.toString() << endl;
	if(v.type != T_REF) SYNTAX_ERR(" can not set reference of '" + typeName[v.type] + "'");

	heap[v.refVal] = setVal;
}

int currentIC = 0;

VS removeAll(VS vs, String remv) {
	VS result;
	ENUM_VS(vs, i) {
		if(remv != vs[i]) result.add(vs[i]);
	}
	return result;
}

Value makeArray(vector<Value> vals) {
	if(vals.size() > ARR_GROW) RUN_ERR(" array size out of maximum limit:\n" + parseNum(ARR_GROW) + " < " + parseNum(vals.size()));
	currentIC++;
	if(currentIC == 256) RUN_ERR("memory out of limit. Current: " + parseNum((currentIC - 1) * ARR_GROW) + " * " + (parseNum(sizeof(Value))));
	heap.resize(ARR_GROW * currentIC);
	for(int i = 0; i < vals.size(); i++) {
		heap[ARR_GROW * (currentIC - 1) + i] = vals[i];
		DEBUG_OUTPUT << "make array: [0x" << ARR_GROW * (currentIC - 1) + i << "] : " << vals[i].toString() << endl;
	}
	
	Value ret = Value::makeRef((currentIC - 1) * ARR_GROW);

	DEBUG_OUTPUT << "makeArray returned " << ret.toString() << endl;
	return ret;
}

String parseNum(double num) {
	stringstream ss("");
	ss << num;
	return ss.str();
}

VS tokenize(string text) {
	string cur = "";
	bool includeSpace = false;
	vector<string> tmp;
	vector<string> ret;
	int top[4] = {
		0, 0, 0, 0
	};

#define ALL_FALSE (!top[0] && !top[1] && !top[2] && !top[3])

	for(int i = 0; i < text.length(); i++) {
		string cur2 = cur + text[i];
		if(text[i] == '(') {
			if(ALL_FALSE) {
				tmp.push_back(cur);
				cur = "";
			}
			top[0]++;
		} else if(text[i] == ')') {
			top[0]--;
		} else if(text[i] == '[') {
			if(ALL_FALSE) {
				tmp.push_back(cur);
				cur = "";
			}
			top[1]++;
		} else if(text[i] == ']') {
			top[1]--;
		} else if(text[i] == '{') {
			if(ALL_FALSE) {
				tmp.push_back(cur);
				cur = "";
			}
			top[2]++;

		} else if(text[i] == '}') {
			top[2]--;
		} else if(text[i] == '"' && (i == 0 || text[i - 1] != '\\')) {
			if(ALL_FALSE) {
				tmp.push_back(cur);
				cur = "";
			}
			top[3] = !top[3];
		}
		if((includeChar(spliter, text[i]) || isspace(text[i])) && !top[0] && !top[1] && !top[2] && !top[3]) {
			tmp.push_back(cur);
			cur = "";
			if(!isspace(text[i]) || includeSpace) {
				cur += text[i];
				tmp.push_back(cur);
				cur = "";
			}
		} else {
			if(top[3]) cur += text[i];
			else if(!isspace(text[i])) cur += text[i];
		}
	}
	tmp.push_back(cur);
	for(int i = 0; i < tmp.size(); i++) {
		if(tmp[i].length() && !isspace(tmp[i][0])) {
			ret.push_back(tmp[i]);
		}
	}

	vector<String> final;
	ENUM_VS(ret, i) {
		if(i > 0) {
			if(ret[i] == "=") {
				if(ret[i - 1] == ">" || ret[i - 1] == "<" || ret[i - 1] == "!" || ret[i - 1] == "=") final[final.size() - 1] += "=";
				else final.add(ret[i]);
			} else if(ret[i] == "&") {
				if(ret[i - 1] == "&") final[final.size() - 1] += "&";
				else final.add(ret[i]);
			} else if(ret[i] == "|") {
				if(ret[i - 1] == "|") final[final.size() - 1] += "|";
				else final.add(ret[i]);
			} else final.add(ret[i]);
		} else final.add(ret[i]);
	}

#undef ALL_FALSE

	return final;
}

VS split(String text, char c, bool space = true) {
	string cur = "";
	bool includeSpace = false;
	vector<string> tmp;
	vector<string> ret;
	int top[4] = {
		0, 0, 0, 0
	};
	for(int i = 0; i < text.length(); i++) {
		string cur2 = cur + text[i];
		if(text[i] == '(') {
			top[0]++;
		} else if(text[i] == ')') {
			top[0]--;
		} else if(text[i] == '[') {
			top[1]++;
		} else if(text[i] == ']') {
			top[1]--;
		} else if(text[i] == '{') {
			top[2]++;
		} else if(text[i] == '}') {
			top[2]--;
		} else if(text[i] == '"' && (i == 0 || text[i - 1] != '\\')) {
			top[3] = !top[3];
		}
		if((((c == text[i]) || (isspace(text[i])))) && !top[0] && !top[1] && !top[2] && !top[3]) {
			tmp.push_back(cur);
			cur = "";
			if(!isspace(text[i]) || includeSpace) {
				cur += text[i];
				tmp.push_back(cur);
				cur = "";
			}
		} else {
			if(top[3]) cur += text[i];
			else if(!isspace(text[i])) cur += text[i];
		}
	}
	tmp.push_back(cur);
	for(int i = 0; i < tmp.size(); i++) {
		if(tmp[i].length() && !isspace(tmp[i][0])) {
			ret.push_back(tmp[i]);
		}
	}
	return ret;
}

String standardSubscript(String str) {
	VS vs = tokenize(str);
	VS result;
	String ret = "";
	ENUM_VS(vs, i) {
		DEBUG_OUTPUT << "token " << vs[i] << endl;
		if(vs[i][0] == '[') {
			String subExpr = MIDDLE(vs[i]);
			result.add("@");
			result.add("(" + subExpr + ")");
		} else result.add(vs[i]);
	}

	ret = joinString(result);
	DEBUG_OUTPUT << "standard -> " << ret << endl;
	return ret;
}

VS buildWith(VS vs, String builder) {
	String r = "";
	VS ret;
	ENUM_VS(vs, i) {
		if(vs[i] == builder) {
			ret.add(r);
			r = "";
			ret.add(builder);
		} else r += vs[i] + " ";
	}
	ret.add(r);
	return ret;
}

String standardFunctionCall(String str) {
	VS vs = tokenize(str);
	VS result;
	String ret = "";
	ENUM_VS(vs, i) {
		DEBUG_OUTPUT << "vs[i] = " << vs[i] << endl;
		if(i > 0 && !(includeChar(opchar + "[{}]", vs[i - 1][0])) && vs[i][0] == '(') {
			String allParam = MIDDLE(vs[i]);
			result.add("^");
			result.add("[" + allParam + "]");
		} else result.add(vs[i]);
	}

	ret = joinString(result);
	DEBUG_OUTPUT << "func standard -> " << ret << endl;
	return ret;
}

Value execBlock(VS);
void setVarRef(String, Value);

Value callFunc(Value func, vector<Value> allParameter) {
#define CHECK_ARG_CNT(n)\
	if(allParameter.size() != n) SYNTAX_ERR("Wrong argument count: " + parseNum(allParameter.size()) + " , need: " + parseNum(n))
	DEBUG_OUTPUT << "callFunc(): func = " << func.toString() << endl;
	if(func.type != T_FUNCTION) RUN_ERR(" can not call a non-function value, given: " + typeName[func.type]);
	if(func.toString() == "<main>") {
	}
	if(func.toString() == "print") {
		DEBUG_OUTPUT << "print function built-in called\n";
		for(int i = 0; i < allParameter.size(); i++) cout << (outputSym ? "" : "") + allParameter[i].toString() << ' ';
		cout << endl;
		return Value::makeNumber(allParameter.size());
	} else if(func.toString() == "exit") {
		CHECK_ARG_CNT(1);
		exit(allParameter[0].numVal);
	} else if(func.toString() == "warfarin") {
		return Value::makeString(VERSION);
	} else if(func.toString() == "len") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		int len = 0;
		int i = v.refVal;
		while(true) {
			if(heap[i].type == T_UNDEF) break;
			i++, len++;
		}
		return Value::makeNumber(len);
	} else if(func.toString() == "alert") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		return Value::makeNumber(MessageBox(NULL, (LPCSTR)v.toString().c_str(), "", MB_OK));
	} else if(func.toString() == "system") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		String s = v.toString();
		return Value::makeNumber(system(s.c_str()));
	} else if(func.toString() == "clock") {
		CHECK_ARG_CNT(0);
		return Value::makeNumber(clock());
	} else if(func.toString() == "math_sin") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		return Value::makeNumber(sin(v.numVal));
	} else if(func.toString() == "math_cos") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		return Value::makeNumber(cos(v.numVal));
	} else if(func.toString() == "math_tan") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		return Value::makeNumber(tan(v.numVal));
	} else if(func.toString() == "math_asin") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		return Value::makeNumber(asin(v.numVal));
	} else if(func.toString() == "math_acos") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		return Value::makeNumber(acos(v.numVal));
	} else if(func.toString() == "math_atan") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		return Value::makeNumber(atan(v.numVal));
	} else if(func.toString() == "math_sqrt") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		return Value::makeNumber(sqrt(v.numVal));
	} else if(func.toString() == "system") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		return Value::makeNumber(system(v.strVal.c_str()));
	} else if(func.toString() == "random") {
		CHECK_ARG_CNT(0);
		return Value::makeNumber(rand());
	} else if(func.toString() == "set_seed") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		srand(v.numVal);
		return v;
	} else if(func.toString() == "eval") {
		CHECK_ARG_CNT(1);
		Value v = allParameter[0];
		VS tmp;
		tmp.add(v.toString());
		execBlock(tmp);
		return v;
	} else if(func.toString() == "tostr") {
		vector<Value> arr;
		for(int i = 0; i < allParameter.size(); i++) {
			String r = "";
			r += (char)(allParameter[i].numVal);
			arr.add(Value::makeString(r));
		}
		return Value::makeRef(makeArray(arr).refVal);
	} else if(func.toString() == "toascii") {
		vector<Value> arr;
		for(int i = 0; i < allParameter.size(); i++) {
			Value v = allParameter[i];
			String str = v.toString();
			for(int j = 0; j < v.toString().length(); j++) arr.add(Value::makeNumber(str[j]));
		}
		return Value::makeRef(makeArray(arr).refVal);
	}

	VS list = paramTable[func.toString()];
	VS block = funcTable[func.toString()];

	DEBUG_OUTPUT << "got list and block of " << func.toString() << endl;

	Value returnVal = Value::makeNull();
	String frame = makeName();

	callStack.add(frame + func.toString());

	DEBUG_OUTPUT << "ready to set parameters" << endl;
	DEBUG_OUTPUT << "list size: " << list.size() << endl;

	if(callStack.size() > INT_MAX) RUN_ERR(" stack overflow. To much nested-function!");
	
	for(int i = 0; i < list.size(); i++) {
		setVarRef(trim(list[i]), allParameter[i]);
		DEBUG_OUTPUT << "parameter: " << trim(list[i]) << " -> " << allParameter[i].toString() << endl;
	}

	DEBUG_OUTPUT << "begin to execBlock" << endl;
	returnVal = execBlock(block);

	DEBUG_OUTPUT << "execBlock done. Return " << returnVal.toString() << endl;
	callStack.pop_back();
	return returnVal;
}

String stringExpr(String str) {
	String result = "";
	for(int i = 0; i < str.length(); i++) {
		if(str[i] == '\\') {
			i++;
			if(i > str.length()) RUN_ERR(" need an escape character after '\\'.");
			switch(str[i]) {
				case 'a':
					result += '\a';
					break;
				case 'n':
					result += '\n';
					break;
				case 'r':
					result += '\r';
					break;
				case '\\':
					result += '\\';
					break;
				case '\'':
					result += '\'';
					break;
				case '\"':
					result += '\"';
					break;
				case 'b':
					result += '\b';
					break;
				case 't':
					result += '\t';
					break;
				case 's':
					result += ' ';
					break;
				default: {
					String msg = " unsupported escape character: '\\";
					msg += str[i];
					msg += "'.";
					RUN_ERR(msg);
					break;
				}
			}
		} else result += str[i];
	}
	return result;
}

Value evalSimpleExpr(String s) {
	if(s[0] == '\'' || s[0] == '\"') return Value::makeString(stringExpr(MIDDLE(s)));
	if(isdigit(s[0])) return Value::makeNumber(atof(s.c_str()));
	else return getVarVal(s);
}

Value evalExpr(String expr) {
	BEGINL
	stack<Value> st;
	Value num, op1, op2;
	VS vs;
	stack<string> stk;
	VS str;

	expr = standardFunctionCall(standardSubscript(expr));

	for(int i = 0; i < expr.length(); i++) {
		if(expr[i] == '-') {
			if(i == 0) {
				expr.insert(0, 1, '0');
			} else if(expr[i - 1] == '(') {
				expr.insert(i, 1, '0');
			}
		}
	}

	str = tokenize(expr);

	for(int i = 0; i < str.size(); i++) {
		string tmp = "";
		string a = "";
		if(includeChar(opchar, str[i][0])) {
			string c = str[i];
			if(stk.empty() || stk.top() == "(") {
				stk.push(c);
			} else {
				while(!stk.empty() && bindPower[stk.top()] >= bindPower[c] ) {
					tmp += stk.top();
					vs.add(tmp);
					stk.pop();
					tmp = "";
				}
				stk.push(c);
			}
		} else vs.add(str[i]);
	}

	while(!stk.empty()) {
		string tmp = "";
		tmp += stk.top();
		vs.add(tmp);
		stk.pop();
	}

	ENUM_VS(vs, i) DEBUG_OUTPUT << "vs[" << i << "] = " << vs[i] << endl;

	for(int i = 0; i < vs.size(); i++) {
		string tmp = vs[i];
		if(st.empty()) DEBUG_OUTPUT << "current top = (nothing)" << endl;
		else DEBUG_OUTPUT << "current top = " << st.top().toString() << endl;
		if(vs[i] == "+") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 + op2);
		} else if(vs[i] == "-") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 - op2);
		} else if(vs[i] == "*") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 * op2);
		} else if(vs[i] == "/") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 / op2);
		} else if(vs[i] == "%") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op2 % op1);
		} else if(vs[i] == ">") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 > op2);
		} else if(vs[i] == "<") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 < op2);
		} else if(vs[i] == ">=") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 >= op2);
		} else if(vs[i] == "<=") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 <= op2);
		} else if(vs[i] == "!=") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 != op2);
		} else if(vs[i] == "==") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 == op2);
		} else if(vs[i] == "!") {
			op2 = st.top();
			st.pop();
			st.push(!op2);
		} else if(vs[i] == "&&") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 && op2);
		} else if(vs[i] == "||") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(op1 || op2);
		} else if(vs[i] == "@") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			st.push(getRefVal(op1, op2));
		} else if(vs[i] == "^") {
			String parameter;
			op2 = st.top();
			st.pop();
			parameter = op2.toString();
			op1 = st.top();
			st.pop();
			DEBUG_OUTPUT << "parameter : " + op2.toString() << endl;


			VS args = removeAll(split(parameter, ','), ",");
			ENUM_VS(args, i) DEBUG_OUTPUT << "args : " << args[i] << endl;

			vector<Value> vals;
			ENUM_VS(args, i) vals.add(evalExpr(args[i]));

			st.push(callFunc(op1, vals));
		} else {
			string simpleExpr = vs[i];
			if(vs[i][0] == '(') {
				simpleExpr = MIDDLE(vs[i]);
				num = evalExpr(simpleExpr);
			} else if(vs[i][0] == '[') {
				simpleExpr = MIDDLE(vs[i]);
				num = Value::makeString(simpleExpr);
			} else if(vs[i][0] == '{') {
				simpleExpr = MIDDLE(vs[i]);
				DEBUG_OUTPUT << "elem = " << simpleExpr << endl;
				VS args = removeAll(split(simpleExpr, ','), ",");
				vector<Value> vals;
				ENUM_VS(args, i) vals.add(evalExpr(args[i]));
				num = makeArray(vals);
			} else num = evalSimpleExpr(simpleExpr);
			st.push(num);
		}
	}
	if(st.size() != 1) SYNTAX_ERR("invalid expression: " + expr);
	DEBUG_OUTPUT << "return val " << st.top().toString() << endl;
	ENDEDL
	return st.top();
}

enum { SUB_SET, VAR_SET };

void setVarRef(String var, String exp) {
	BEGINL
	stack<Value> st;
	Value num, op1, op2;
	VS vs;
	stack<string> stk;
	VS str;
	String expr = var;
	int type = VAR_SET;

	for(int i = 0; i < var.length(); i++) {
		if(var[i] == '[') type = SUB_SET;
	}

	if(type == VAR_SET) {
		Value v;
		DEBUG_OUTPUT << "expression " << exp << endl;
		v = evalExpr(exp);
		setVarVal(var, v);
		DEBUG_OUTPUT << "setted normal var " << var << " to " << v.toString() << endl;
		ENDEDL
		return;
		DEBUG_OUTPUT << "!!![SHOULD NOT BE RUN]!!!" << endl;
	}

	expr = standardFunctionCall(standardSubscript(expr));

	for(int i = 0; i < expr.length(); i++) {
		if(expr[i] == '-') {
			if(i == 0) {
				expr.insert(0, 1, '0');
			} else if(expr[i - 1] == '(') {
				expr.insert(i, 1, '0');
			}
		}
	}

	str = tokenize(expr);

	for(int i = 0; i < str.size(); i++) {
		string tmp = "";
		string a = "";
		if(includeChar(opchar, str[i][0])) do {
				string c = str[i];
			
				if(stk.empty() || stk.top() == "(") {
					stk.push(c);
				} else {
					while(!stk.empty() && bindPower[stk.top()] >= bindPower[c] ) {
						tmp += stk.top();
						vs.add(tmp);
						stk.pop();
						tmp = "";
					}
					stk.push(c);
				}
			} while(0);
		else vs.add(str[i]);
	}

	while(!stk.empty()) {
		string tmp = "";
		tmp += stk.top();
		vs.add(tmp);
		stk.pop();
	}

	DEBUG_OUTPUT << "vs.size() = " << vs.size() << endl;

	ENUM_VS(vs, i) DEBUG_OUTPUT << "getRefVal vs = " << vs[i] << endl;

	for(int i = 0; i < vs.size() - 1; i++) {
		string tmp = vs[i];
		if(!st.empty()) DEBUG_OUTPUT << "Current val: " << st.top().toString() << endl;
		if(vs[i] == "@") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			/*
			if(op2.type == T_STRING) {
				DEBUG_OUTPUT << "string pushing: " << op1.toString() << " of loc @ " << getHash(op2.strVal) << endl;
				st.push(Value::makeRef(getHash(op2.strVal) + op1.refVal));
				//st.push(getRefVal(op1, Value::makeNumber(getHash(op2.strVal))));
				DEBUG_OUTPUT << "current stack top: " << st.top().toString() << endl;
			}
			else {
				DEBUG_OUTPUT << "var pushing: " << op1.toString() << " of loc @ " << op2.numVal << endl;
				//st.push(Value::makeRef(heap[op1.refVal].refVal + op2.refVal));
				st.push(Value::makeRef(op2.numVal + op1.refVal));
			}
			*/
			DEBUG_OUTPUT << "subscript pushing: " << op1.toString() << " of loc @ " << op2.numVal << endl; 
			st.push(getRefVal(op1, op2));
		} else {
			string simpleExpr = vs[i];
			if(vs[i][0] == '(') {
				simpleExpr = MIDDLE(vs[i]);
				num = evalExpr(simpleExpr);
			} else if(vs[i][0] == '[') {
				simpleExpr = MIDDLE(vs[i]);
				num = Value::makeString(simpleExpr);
				DEBUG_OUTPUT << "current string subscript is " << num.toString() << ", and hash " << getHash(num.strVal) << endl;
				num = Value::makeNumber(getHash(num.strVal));
			} else if(vs[i][0] == '{') {
				simpleExpr = MIDDLE(vs[i]);
				DEBUG_OUTPUT << "elem = " << simpleExpr << endl;
				VS args = removeAll(split(simpleExpr, ','), ",");
				vector<Value> vals;
				ENUM_VS(args, i) vals.add(evalExpr(args[i]));
				num = makeArray(vals);
			} else num = evalSimpleExpr(simpleExpr);
			if(num.type == T_STRING) {
				DEBUG_OUTPUT << "current string subscript is " << num.toString() << ", and hash " << getHash(num.strVal) << endl;
				num = Value::makeNumber(getHash(num.strVal));
			}
			DEBUG_OUTPUT << "simple expression subscript pushed " << num.toString() << endl;
			st.push(num);
		}
	}
	
	op1 = st.top();
	st.pop();
	op2 = st.top();
	st.pop();
	Value result = Value::makeRef(op2.refVal + op1.numVal);
	st.push(result);
	DEBUG_OUTPUT << "final location: " << result.toString() << endl;

	Value v2 = evalExpr(exp);
	DEBUG_OUTPUT << "Will set " << st.top().toString() << " to " << v2.toString() << endl;
	setRefVal(st.top(), v2);
	ENDEDL
}

void setVarRef(String var, Value exp) {
	BEGINL
	stack<Value> st;
	Value num, op1, op2;
	VS vs;
	stack<string> stk;
	VS str;
	String expr = var;
	int type = VAR_SET;

	for(int i = 0; i < var.length(); i++) {
		if(var[i] == '[') type = SUB_SET;
	}

	if(type == VAR_SET) {
		Value v;
		DEBUG_OUTPUT << "expression " << exp.toString() << endl;
		v = exp;
		setVarVal(var, v);
		DEBUG_OUTPUT << "setted normal var " << var << " to " << v.toString() << endl;
		ENDEDL
		return;
		DEBUG_OUTPUT << "!!![SHOULD NOT BE RUN]!!!" << endl;
	}

	expr = standardFunctionCall(standardSubscript(expr));

	for(int i = 0; i < expr.length(); i++) {
		if(expr[i] == '-') {
			if(i == 0) {
				expr.insert(0, 1, '0');
			} else if(expr[i - 1] == '(') {
				expr.insert(i, 1, '0');
			}
		}
	}

	str = tokenize(expr);

	for(int i = 0; i < str.size(); i++) {
		string tmp = "";
		string a = "";
		if(includeChar(opchar, str[i][0])) do {
				string c = str[i];
				if(stk.empty() || stk.top() == "(") {
					stk.push(c);
				} else {
					while(!stk.empty() && bindPower[stk.top()] >= bindPower[c] ) {
						tmp += stk.top();
						vs.add(tmp);
						stk.pop();
						tmp = "";
					}
					stk.push(c);
				}
			} while(0);
		else vs.add(str[i]);
	}

	while(!stk.empty()) {
		string tmp = "";
		tmp += stk.top();
		vs.add(tmp);
		stk.pop();
	}

	DEBUG_OUTPUT << "vs.size() = " << vs.size() << endl;

	ENUM_VS(vs, i) DEBUG_OUTPUT << "getRefVal vs = " << vs[i] << endl;

	for(int i = 0; i < vs.size() - 1; i++) {
		string tmp = vs[i];
		if(!st.empty()) DEBUG_OUTPUT << "Current val: " << st.top().toString() << endl;
		if(vs[i] == "@") {
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			/* 
			if(op2.type == T_STRING) {
				DEBUG_OUTPUT << "string pushing: " << op1.toString() << " of loc @ " << getHash(op2.strVal) << endl;
				st.push(Value::makeRef(getHash(op2.strVal) + op1.refVal));
			}
			else {
				DEBUG_OUTPUT << "var pushing: " << op1.toString() << " of loc @ " << op2.numVal << endl;
				/*if(i != vs.size() - 2) st.push(Value::makeRef(heap[op1.refVal].refVal + op2.refVal));
				else st.push(Value::makeRef(op2.numVal + op1.refVal)); 
			}
			*/
			DEBUG_OUTPUT << "subscript pushing: " << op1.toString() << " of loc @ " << op2.numVal << endl; 
			st.push(getRefVal(op1, op2));
		} else {
			string simpleExpr = vs[i];
			if(vs[i][0] == '(') {
				simpleExpr = MIDDLE(vs[i]);
				num = evalExpr(simpleExpr);
			} else if(vs[i][0] == '[') {
				simpleExpr = MIDDLE(vs[i]);
				num = Value::makeString(simpleExpr);
			} else if(vs[i][0] == '{') {
				simpleExpr = MIDDLE(vs[i]);
				DEBUG_OUTPUT << "elem = " << simpleExpr << endl;
				VS args = removeAll(split(simpleExpr, ','), ",");
				vector<Value> vals;
				ENUM_VS(args, i) vals.add(evalExpr(args[i]));
				num = makeArray(vals);
			} else num = evalSimpleExpr(simpleExpr);
			if(num.type == T_STRING) {
				DEBUG_OUTPUT << "current string subscript is " << num.toString() << ", and hash " << getHash(num.strVal) << endl;
				num = Value::makeNumber(getHash(num.strVal));
			}
			DEBUG_OUTPUT << "simple expression subscript pushed " << num.toString() << endl;
			st.push(num);
		}
	}
	
	op1 = st.top();
	st.pop();
	op2 = st.top();
	st.pop();
	Value result = Value::makeRef(op2.refVal + op1.numVal);
	st.push(result);
	DEBUG_OUTPUT << "final location: " << result.toString() << endl;

	Value v2 = exp;
	DEBUG_OUTPUT << "Will set " << st.top().toString() << " to " << v2.toString() << endl;
	setRefVal(st.top(), v2);
	ENDEDL
}

void init() {
	callStack.add("main");
#define BIND(c, rbp, code) bindPower[(c)] = (rbp)
	BIND("+", 60, ADD);
	BIND("-", 60, DEC);
	BIND("*", 70, MUL);
	BIND("/", 70, DIV);
	BIND("%", 70, MOD);
	BIND(">", 50, BIG);
	BIND(">=", 50, BIGEQL);
	BIND("<", 50, SML);
	BIND("<=", 50, SMLEQL);
	BIND("==", 40, EQL);
	BIND("!=", 40, NEQL);
	BIND("!", 30, NOT);
	BIND("&&", 30, AND);
	BIND("||", 29, OR);
	BIND("@", 90, SUB);
	BIND("^", 90, CALL);
#undef BIND
	setVarVal("warfarin", Value::makeFunction("warfarin"));
	setVarVal("print", Value::makeFunction("print"));
	setVarVal("exit", Value::makeFunction("exit"));
	setVarVal("len", Value::makeFunction("len"));
	setVarVal("system", Value::makeFunction("system"));
	setVarVal("alert", Value::makeFunction("alert"));
	setVarVal("clock", Value::makeFunction("clock"));
	setVarVal("math_sin", Value::makeFunction("math_sin"));
	setVarVal("math_cos", Value::makeFunction("math_cos"));
	setVarVal("math_tan", Value::makeFunction("math_tan"));
	setVarVal("math_asin", Value::makeFunction("math_asin"));
	setVarVal("math_acos", Value::makeFunction("math_acos"));
	setVarVal("math_atan", Value::makeFunction("math_atan"));
	setVarVal("math_sqrt", Value::makeFunction("math_sqrt"));
	setVarVal("system", Value::makeFunction("system"));
	setVarVal("random", Value::makeFunction("random"));
	setVarVal("set_seed", Value::makeFunction("set_seed"));
	setVarVal("eval", Value::makeFunction("eval"));
	setVarVal("toascii", Value::makeFunction("toascii"));
	setVarVal("tostr", Value::makeFunction("tostr"));
	setVarVal("math_pi", Value::makeNumber(3.14159265358797932384626));
	setVarVal("null", Value::makeNull());
	setVarVal("true", Value::makeBool(true));
	setVarVal("false", Value::makeBool(false));
}

enum { EXPR, ASSIGN };

Value execute(String stat) {
	BEGINL
	VS vs = tokenize(stat);
	int i = 0;
	int type = EXPR;
	String flag = vs[0];

	bool inq = false;

	for(i = 0; i < stat.length(); i++) {
		if(i > 0 && stat[i - 1] != '\\' && stat[i] == '\"') inq = !inq;
		if(stat[i] == '=' && !inq) {
			type = ASSIGN;
			break;
		}
	}

	if(type == ASSIGN) {
		String left = trim(stat.substr(0, i));
		String right = trim(stat.substr(i + 1, stat.length()));
		DEBUG_OUTPUT << "left : " << left << endl;
		DEBUG_OUTPUT << "right : " << right << endl;
		setVarRef(left, right);
		DEBUG_OUTPUT << "execute set done" << endl;

	} else if(flag == "stop") {
		STOP_EXEC(joinString(vs));
	} else {
		DEBUG_OUTPUT << evalExpr(stat).toString() << endl;
	}
	ENDEDL
	return Value::makeNumber(0);
}

void registFunc(String func, VS list, VS block) {
	DEBUG_OUTPUT << "Regist function: " << func << endl;
	DEBUG_OUTPUT << "List: " << joinString(list) << " size: " << list.size() << endl;
	funcTable[func] = block;
	paramTable[func] = list;
}

void readCodeFrom(String fileName) {
	ifstream fcin(fileName.c_str());
	String s;
	while(getline(fcin, s)) {
		VS vs = tokenize(s);
		if(vs[0] == "import") {
			String name = "";
			for(int i = 1; i < vs.size(); i++) name += vs[i];
			readCodeFrom(name);
		} else codeStream << s << endl;
	}
}

void readCode() {
	String s;
	cout << ">>> ";
	while(getline(cin, s)) {
		VS vs = tokenize(s);
		if(vs[0] == "import") {
			String name = "";
			for(int i = 1; i < vs.size(); i++) name += vs[i];
			readCodeFrom(name);
		} else codeStream << s << endl;
		cout << ">>> ";
	}
}

VS makeBlock() {
	String s;
	VS ret;
	BEGINL
	while(getline(codeStream, s)) {
		VS vs;
		s = trim(s);
		vs = tokenize(s);
		if(s == "") continue;
		if(s == "}") break;
		if(vs[0] == "if") {
			String expr = MIDDLE(vs[1]);
			String stat = "if (";
			String rname = makeName();
			VS block;
			stat += expr;
			stat += ") ";
			stat += rname;
			block = makeBlock();
			DEBUG_OUTPUT << "if stat : " << stat << endl;
			registFunc(rname, EMPTY, block);
			ret.add(stat);
		} else if(vs[0] == "while") {
			String expr = MIDDLE(vs[1]);
			String stat = "while (";
			String rname = makeName();
			VS block;
			stat += expr;
			stat += ") ";
			stat += rname;
			block = makeBlock();
			DEBUG_OUTPUT << "while stat : " << stat << endl;
			registFunc(rname, EMPTY, block);
			ret.add(stat);
		} else if(vs[0] == "for") {
			String expr = MIDDLE(vs[1]);
			String stat = "for ";
			VS condition = buildWith(split(expr, ','), ",");
			if(condition.size() != 7) SYNTAX_ERR("for statement syntax: for('name', expression, expression, expression) {block}");

			for(int i = 0; i < 7; i++) {
				if(condition[i] == ",") continue;
				String tmp = "(";
				tmp += condition[i];
				tmp += ")";
				stat += tmp + " ";
			}

			String rname = makeName();
			VS block;
			stat += rname;
			block = makeBlock();
			DEBUG_OUTPUT << "for stat : " << stat << endl;
			registFunc(rname, EMPTY, block);
			ret.add(stat);
		} else if(vs[0] == "function") {
			String name = vs[1];
			String expr = MIDDLE(vs[2]);
			VS args = removeAll(buildWith(split(expr, ','), ","), ",");
			VS finalArgs;
			ENUM_VS(args, i) {
				if(args[i] != "") finalArgs.add(args[i]);
			}

			String rname = name;
			VS block = makeBlock();

			setVarRef(name, Value::makeFunction(rname));

			DEBUG_OUTPUT << "function defined. " << name << " is " << rname << endl;
			registFunc(rname, finalArgs, block);
		} else ret.add(s);
	}
	ENDEDL
	return ret;
}

Value execBlock(VS block);

Value execBlock(VS block) {
	ENUM_VS(block, index) {
		VS token = tokenize(trim(block[index]));
		if(token[0] == "if") {
			String condition = MIDDLE(token[1]);
			if((!evalExpr(condition)).type == T_FALSE) callFunc(Value::makeFunction(token[2]), EMPTY_V);
		} else if(token[0] == "while") {
			String condition = MIDDLE(token[1]);
			while((!evalExpr(condition)).type == T_FALSE) callFunc(Value::makeFunction(token[2]), EMPTY_V);
		} else if(token[0] == "for") {
			String variable = MIDDLE(token[1]);
			String from = MIDDLE(token[2]);
			String condition = MIDDLE(token[3]);
			Value step = evalExpr(MIDDLE(token[4]));
			for(
			    setVarRef(variable, evalExpr(from));
			    !isFalse(evalExpr(condition));
			    setVarRef(variable, getVarVal(variable) + step)
			) callFunc(Value::makeFunction(token[5]), EMPTY_V);
		} else if(token[0] == "return") {
			String r = "";
			for(int i = 1; i < token.size(); i++) r += token[i];
			return evalExpr(r);
		} else execute(trim(block[index]));
	}
	return Value::makeNull();
}

void parseArg(String arg) {
	arg = arg.substr(1, arg.length());
}

void readByteCode(String bytefile) {
	ifstream fcin(bytefile.c_str());
	int funcCount;
	String temp;
	getline(fcin, temp);
	if(temp != "ff0") {
		RUN_ERR("Invalid bytecode file : " + bytefile);
	}
	getline(fcin, temp);
	funcCount = atof(temp.c_str());
	for(int func = 0; func < funcCount; func++) {
		String funcName;
		VS pList;
		VS cmdList;
		int paramCnt;
		int lineCnt;
		getline(fcin, funcName);
//		cout << funcName << endl;
		getline(fcin, temp);
		paramCnt = atof(temp.c_str());
		for(int i = 0; i < paramCnt; i++) {
			String pName;
			getline(fcin, pName);
			pList.push_back(pName);
		}
		getline(fcin, temp);
		lineCnt = atof(temp.c_str());
		for(int i = 0; i < lineCnt; i++) {
			String s;
			getline(fcin, s);
			cmdList.push_back(s);
//			cout << "cmd #" << i + 1 << ": " << cmdList[i] << endl; 
		}
		registFunc(funcName, pList, cmdList);
		setVarVal(funcName, Value::makeFunction(funcName));
	}
}
String codefileName;

String getOutputFile() {
	for(int i = codefileName.size() - 1; i >= 0; i--) {
		if(codefileName[i] == '.') {
			return codefileName.substr(0, i) + ".ff0";
		}
	}
	 
}

void compile() {
	ofstream fcout(getOutputFile().c_str());
	map<String, VS>::iterator paramIt = paramTable.begin();
	fcout << funcTable.size() << endl;
	for(map<String, VS>::iterator it = funcTable.begin(); it != funcTable.end(); it++) {
		fcout << it -> first << endl;
		fcout << paramIt -> second.size() << endl;
		for(int i = 0; i < paramIt -> second.size(); i++) fcout << paramIt -> second[i] << endl;
		fcout << it -> second.size() << endl;
		for(int i = 0; i < it -> second.size(); i++) fcout << it -> second[i] << endl;
		paramIt++;
	}
}

int main(int argc, char ** argv) {
	init();
//	cout << VERSION << endl;
	codefileName = "a.ff0";
	vector<Value> EMPTY_LIST;
	if(argc == 1) {
		cout << "usage: ff0 <file>" << endl;
		exit(-1);
	} else {
		for(int i = 1; i < argc; i++) {
			codefileName = argv[i];
		}
	}
	readByteCode(codefileName);
	try {
		Value starter = Value::makeFunction("<main>");
		callFunc(starter, EMPTY_LIST);
	}
	catch(ExecuteException ex) {
		cout << ex.msg << endl;
		exit(1);
	}
	return 0;
}
