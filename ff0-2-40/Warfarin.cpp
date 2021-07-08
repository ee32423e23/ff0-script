#include <bits/stdc++.h>
#ifdef WIN32
#include <windows.h>
#endif
//#define DEBUG
using namespace std;

const string VERSION = "Warfarin script 2.40";

bool zeroTrue = false;
bool cli = false;
bool outputSym = true;
bool outputExpr = false;

typedef int Ref;
typedef int ValueType;
typedef string String;
typedef vector<String> VS;

String currentCommand;

VS EMPTY;

VS protectVar;

const int ARR_GROW = 1024 * 256;

map<String, int> bindPower;

vector<vector<String> > usedLocalVar;
vector<String> callStack;
map<String, VS> funcTable;
map<String, VS> paramTable;
map<String, VS> ptypeTable;

const string opchar = "+-*/!=><%&|@^";
const string spliter = "+-*/=><;!&|%@^,~";

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

enum { BREAK, CONTINUE };
struct LoopMessage {
	int type;
};

int hashCnt;
map<String, int> hashValue;
int getHash(String s) {
    if(hashValue.find(s) == hashValue.end()) hashCnt++, hashValue[s] = hashCnt;
    return hashValue[s] + 1024 * 252;
}

String parseNum(double num) {
    char buffer[205];
    sprintf(buffer, "%.14g", num);
    return buffer;
}

stringstream debugMsg("");

#ifdef DEBUG
#define DEBUG_OUTPUT cout << allTab() << __LINE__ << " "
#else
#define DEBUG_OUTPUT debugMsg << allTab() << __LINE__ << " "
#endif

#define NEXTL DEBUG_OUTPUT << "======================" << __LINE__ << " run done ============================" << endl;
#define BEGINL DEBUG_OUTPUT << __func__ << " {" << endl; tabcnt++;
#define ENDEDL tabcnt--; DEBUG_OUTPUT << "}" << endl;

#define SYNTAX_ERR(msg) \
    do {\
        cout << "SyntaxError: " << msg << endl;\
        cout << currentCommand << endl;\
        throw ExecuteException();\
    } while(0)

#define RUN_ERR(msg) \
    do {\
        cout << "RuntimeError: " << msg << endl;\
        cout << currentCommand <<endl;\
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

const int ARRCNT = 64;
bool allIndexes[ARRCNT];

bool checkName(String s) {
	if(!isalpha(s[0]) && s[0] != '_' && s[0] != '>' && s[0] != '<') return false;
	for(int i = 1; i < s.length(); i++) if(!iscsym(s[i]) && s[0] != '>' && s[0] != '<') return false;
	return true;
}

int getUsedCount() {
	int x = 0;
	for(int i = 0; i < 128; i++) if(allIndexes[i]) x++;
	return x;
}

int allocMemory() {
	for(int i = 0; i < 128; i++) if(!allIndexes[i]) {
		allIndexes[i] = true;
		DEBUG_OUTPUT << "All used count: " << getUsedCount() << endl;
		return i;
	}
}

void freeMemory(int index) {
	allIndexes[index] = false;
}

vector<FILE *> filePtr;
int fileOpen(String file, String mode = "rw") {
	FILE * ptr = fopen(file.c_str(), mode.c_str());
	if(ptr == NULL) {
		RUN_ERR("File doesn't exist: " + file);
	}
	filePtr.add(ptr);
	return filePtr.size() - 1;
}

void checkHandle(int handle) {
	if(handle < 0 || handle >= filePtr.size()) RUN_ERR("Invalid handle: " + parseNum(handle));
}

void fileClose(int handle) {
	checkHandle(handle);
	fclose(filePtr[handle]);
}

char fbuffer[2048];
String freadString(int handle) {
	checkHandle(handle);
	String result = "";
	char c;
	while(isspace(c) && !feof(filePtr[handle])) c = fgetc(filePtr[handle]);
	c = fgetc(filePtr[handle]);
	while(!isspace(c) && !feof(filePtr[handle])) {
		result += c;
		c = fgetc(filePtr[handle]);
	}
	return result;
}
String freadStrLine(int handle) {
	checkHandle(handle);
	String result = "";
	char c;
	while(isspace(c) && !feof(filePtr[handle])) c = fgetc(filePtr[handle]);
	c = fgetc(filePtr[handle]);
	while(c != '\n' && !feof(filePtr[handle])) {
		result += c;
		c = fgetc(filePtr[handle]);
	}
	return result;
}
void fwriteString(int handle, String text) {
	checkHandle(handle);
	for(int i = 0; i < text.length(); i++) {
		fputc(text[i], filePtr[handle]);
	}
	fflush(filePtr[handle]);
}
double freadNumber(int handle) {
	checkHandle(handle);
	return atof(freadString(handle).c_str());
}
void fwriteNumber(int handle, double x) {
	checkHandle(handle);
	fprintf(filePtr[handle], "%.14g", x);
	fflush(filePtr[handle]);
}

enum {
    T_NUMBER, T_STRING, T_OBJECT, T_FUNCTION,
    T_REF, T_TRUE, T_FALSE, T_NULL, T_UNKNOWN, T_UNDEF, T_HWND
};

String typeName[] = {
    "Number", "String", "Object", "Closure",
    "Reference", "Bool", "Bool", "Null", "Unknown", "Undefined", "HWND"
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
    String ret = "_inner";
    ret += parseNum(nameCnt++);
    return ret;
}

#ifdef WIN32
String parseHWND(HWND hwnd) {
	stringstream ss("");
	ss << "[HWND ";
	ss << hwnd;
	ss << "]";
	return ss.str();
}
#endif

struct Value {
    double numVal;
    String strVal;
    Object objVal;
    String funVal;
    Ref refVal;
    #ifdef WIN32
    HWND hwndVal;
    #endif
    
    Value() {
        type = T_UNDEF;
    }

    ValueType type;
    String toString() {
        if(type == T_NUMBER) return parseNum(numVal);
        else if(type == T_STRING) return strVal;
        else if(type == T_OBJECT) return objVal.toString();
        else if(type == T_FUNCTION) return "[function " + funVal + "]";
        else if(type == T_REF) return "$" + parseNum(refVal);
        else if(type == T_TRUE) return "true";
        else if(type == T_FALSE) return "false";
        else if(type == T_NULL) return "null";
        #ifdef WIN32
        else if(type == T_HWND) return parseHWND(hwndVal);
        #endif
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
	
	#ifdef WIN32
    static Value makeHWND(HWND hwnd) {
        Value v;
        v.type = T_HWND;
        v.hwndVal = hwnd;
        return v;
    }
	#endif
	
    static Value makeNull() {
        Value v;
        v.type = T_NULL;
        return v;
    }

    Value operator + (Value & v) {
        if(type == T_STRING || v.type == T_STRING) return Value::makeString(strVal + v.toString());
        else if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not add " + typeName[v.type] + " to a number.");
            return Value::makeNumber(v.numVal + numVal);
        } else if(type == T_REF) {
            if(v.type != T_NUMBER) RUN_ERR(" can not add " + typeName[v.type] + " to an reference.");
            return Value::makeRef(v.numVal + refVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate '+'.");
    }

    Value operator - (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not substract " + typeName[v.type] + " to a number.");
            return Value::makeNumber(numVal - v.numVal);
        } else if(type == T_REF) {
            if(v.type != T_NUMBER) RUN_ERR(" can not substract " + typeName[v.type] + " to an reference.");
            return Value::makeRef(numVal - v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate '-'.");
    }

    Value operator * (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not multiply " + typeName[v.type] + " to a number.");
            return Value::makeNumber(numVal * v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate '*'.");
    }

    Value operator / (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not divide " + typeName[v.type] + " to a number.");
            return Value::makeNumber(numVal / v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate '/'.");
    }
    
    Value operator & (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not bit-and " + typeName[v.type] + " to a number.");
            return Value::makeNumber((int)numVal & (int)v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate '&'.");
    }
    
    Value operator xor (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not bit-xor " + typeName[v.type] + " to a number.");
            return Value::makeNumber((int)numVal xor (int)v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate xor.");
    }

    Value operator | (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not bit-or " + typeName[v.type] + " to a number.");
            return Value::makeNumber((int)numVal | (int)v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate '|'.");
    }

    Value operator > (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.");
            return Value::makeBool(numVal > v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate '>'.");
    }

    Value operator < (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.");
            return Value::makeBool(numVal < v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate '<'.");
    }

    Value operator >= (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.");
            return Value::makeBool(numVal >= v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate '>='.");
    }

    Value operator <= (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.");
            return Value::makeBool(numVal <= v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate '<='.");
    }

    Value operator % (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not mod " + typeName[v.type] + " with a number.");
            return Value::makeNumber(fmod(v.numVal, numVal));
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operate '%'.");
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
        if(type == T_NUMBER) {
            if(v.type == T_TRUE) return Value::makeBool(true);
            else if(v.type == T_NUMBER) return Value::makeBool(v.numVal && numVal);
        } else if(type == T_TRUE) {
            if(v.type == T_TRUE) return Value::makeBool(true);
            else if(v.type == T_NUMBER) return Value::makeBool(1 && numVal);
        } else return Value::makeBool(false);
        return Value::makeBool(false);
    }

    Value operator || (Value & v) {
        if(type == T_NUMBER) {
            if(v.type == T_TRUE) return Value::makeBool(true);
            else if(v.type == T_NUMBER) return Value::makeBool(v.numVal || numVal);
        } else if(type == T_TRUE) {
            return Value::makeBool(true);
        } else return Value::makeBool(false);
        return Value::makeBool(false);
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

bool findchar(String s, char c) {
	for(int i = 0; i < s.length(); i++) {
		if(s[i] == c) return true;
	}
	return false;
}

String parseSubscript(String);
Value evalExpr(String expr);

Value getVarVal(String name) {
	DEBUG_OUTPUT << "getVarVal received argument " << name << endl;
	if(findchar(name, '.')) return evalExpr(parseSubscript(name));
	if(!checkName(name)) RUN_ERR("invalid variable name: " + name);
    for(int i = callStack.size() - 1; i >= 0; i--) {
        String newname = callStack[i] + name;
        if(scope.find(newname) == scope.end()) continue;
        else return scope[newname];
    }
    SYNTAX_ERR("variable " + name + " is not declared.");
    scope[callStack[callStack.size() - 1] + name] = Value();
    return scope[callStack[callStack.size() - 1] + name];
}

void setLocalVarVal(String name, Value v) {
   	DEBUG_OUTPUT << "Detected local var " << name << " of " << v.toString() << endl;
    usedLocalVar[usedLocalVar.size() - 1].add(name);
    scope[callStack[callStack.size() - 1] + name] = v;
}

void setVarVal(String name, Value v) {
	if(!checkName(name)) RUN_ERR("invalid variable name: " + name);
    for(int i = callStack.size() - 1; i >= 0; i--) {
        String newname = callStack[i] + name;
        if(scope.find(newname) != scope.end()) {
        	scope[newname] = v;
        	return;
		}
        else continue;
    }
    DEBUG_OUTPUT << "Detected local Var " << name << " of " << v.toString() << endl;
    usedLocalVar[usedLocalVar.size() - 1].add(name);
    scope[callStack[callStack.size() - 1] + name] = v;
}
Value getRefVal(Value, Value);
void freeVariable(Value v) {
	if(v.type != T_REF) return;
	int len = 0;
    int i = v.refVal;
    DEBUG_OUTPUT << "free ref " << v.toString() << endl;
    while(true) {
        if(heap[i].type == T_UNDEF) break;
        i++, len++;
    }
    freeMemory(v.refVal / ARR_GROW);
	if(v.type == T_REF) {
		for(int x = 0; x < len; x++) {
			DEBUG_OUTPUT << "free variable " << v.toString() << " of subscript " << x << endl;
			freeVariable(getRefVal(v, Value::makeNumber(x)));
		}
	}
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

void lookVariable() {
	for(map<String, Value>::iterator it = scope.begin(); it != scope.end(); it++) {
		DEBUG_OUTPUT << it -> first << '\t' << it -> second.toString() << endl;
	}
	for(int i = 0; i < ARRCNT; i++) {
		DEBUG_OUTPUT << "unit " << i << ": " << (allIndexes[i] ? "used" : "vacant") << endl;
	}
}

Value makeArray(vector<Value> vals, bool useWay = true) {
    if(vals.size() > ARR_GROW) RUN_ERR(" array size out of maximum limit:\n" + parseNum(ARR_GROW) + " < " + parseNum(vals.size()));
    int icIndex = allocMemory();
    if(!useWay) {
    	String name = makeName();
    	DEBUG_OUTPUT << "makeArray detected anonymous array. makeName: " << name << endl;
    	Value val = Value::makeRef(icIndex * ARR_GROW);
    	setVarVal(name, val);
    	DEBUG_OUTPUT << "set anonymous " << val.toString() << endl;
    	usedLocalVar[usedLocalVar.size() - 1].push_back(name);
	}
    DEBUG_OUTPUT << "get ic index = " << icIndex << endl;
    DEBUG_OUTPUT << "current heap size = " << heap.size() << endl;
    for(int i = 0; i < vals.size(); i++) {
        heap[ARR_GROW * icIndex + i] = vals[i];
        DEBUG_OUTPUT << "make array: [0x" << ARR_GROW * icIndex + i << "] : " << vals[i].toString() << endl;
    }

    Value ret = Value::makeRef(icIndex * ARR_GROW);

    DEBUG_OUTPUT << "makeArray returned " << ret.toString() << endl;
    return ret;
}

VS tokenize(String text) {
	VS tmp;
	String cur = "";
	for(int i = 0; i < text.length(); i++) {
		if(text[i] == '(') {
			int top = 1;
			bool inq = false;
			if(cur != "") {
				tmp.push_back(cur);
			}
			cur = "(";
			while(top && i < text.length()) {
				i++;
				if((!isspace(text[i]) && text[i] != '\\') || inq) cur = cur + text[i];
				if(text[i] == '(') top++;
				else if(text[i] == ')') top--;
				if(text[i] == '\\') {
					if(i < text.length() - 1) {
						i++;
						cur = cur + text[i];
					}
				}
				else if(text[i] == '"') {
					inq = !inq;
				}
			}
			tmp.push_back(cur);
			cur = "";
		}
		else if(text[i] == '[') {
			int top = 1;
			bool inq = false;
			if(cur != "") {
				tmp.push_back(cur);
			}
			cur = "[";
			while(top && i < text.length()) {
				i++;
				if((!isspace(text[i]) && text[i] != '\\') || inq) cur = cur + text[i];
				if(text[i] == '[') top++;
				else if(text[i] == ']') top--;
				if(text[i] == '\\') {
					if(i < text.length() - 1) {
						i++;
						cur = cur + text[i];
					}
				}
				else if(text[i] == '"') {
					inq = !inq;
				}
			}
			tmp.push_back(cur);
			cur = "";
		}
		else if(text[i] == '{') {
			int top = 1;
			bool inq = false;
			if(cur != "") {
				tmp.push_back(cur);
			}
			cur = "{";
			while(top && i < text.length()) {
				i++;
				if((!isspace(text[i]) && text[i] != '\\') || inq) cur = cur + text[i];
				if(text[i] == '{') top++;
				else if(text[i] == '}') top--;
				if(text[i] == '\\') {
					if(i < text.length() - 1) {
						i++;
						cur = cur + text[i];
					}
				}
				else if(text[i] == '"') {
					inq = !inq;
				}
			}
			tmp.push_back(cur);
			cur = "";
		}
		else if(text[i] == '"') {
			int top = 1;
			if(cur != "") {
				tmp.push_back(cur);
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
			tmp.push_back(cur);
			cur = "";
		}
		else if(!iscsym(text[i]) && text[i] != '~' && text[i] != '.') {
			if(cur != "") {
				tmp.push_back(cur);
			} 
			if(!isspace(text[i])) {
				cur = "";
				cur += text[i];
				tmp.push_back(cur);
			}
			cur = "";
		}
		else if(iscsym(text[i]) && text[i] != '~' || text[i] == '.') {
			cur += text[i];
		}
	}
	if(cur != "") {
		tmp.push_back(cur);
	}
	VS ret;
	VS final;
	ENUM_VS(tmp, i) {
		if(tmp[i] != "") {
			ret.add(trim(tmp[i]));
		}
	}
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

String parseSubscript(String expr) {
	VS vs = split(expr, '.');
	String result = vs[0];
	ENUM_VS(vs, i) {
		if(i == 0 || vs[i] == ".") continue;
		result += "[\"" + vs[i] + "\"]";
	}
	return result;
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
void registFunc(String func, VS list, VS ptype, VS block);

void gc() {
	BEGINL
	bool canVisit[ARRCNT];
	memset(canVisit, false, sizeof(canVisit));
	for(map<String, Value>::iterator it = scope.begin(); it != scope.end(); it++) {
		Value val = it -> second;
		if(val.type == T_REF) {
			DEBUG_OUTPUT << "collecting: " << val.refVal / ARR_GROW << " can be visited." << endl;
			canVisit[val.refVal / ARR_GROW] = true;
		}
	}
	vector<String> allIndex = usedLocalVar[usedLocalVar.size() - 1];
    DEBUG_OUTPUT << "free indexes: " << allIndex.size() << endl;
    for(int i = 0; i < allIndex.size(); i++) {
    	DEBUG_OUTPUT << "can free: " << allIndex[i] << endl;
    	String local = callStack[callStack.size() - 1] + allIndex[i];
    	DEBUG_OUTPUT << "find localvar " << local << " = " << scope[local].toString() << endl;
    	Value val = scope[local];
    	freeVariable(val);
    	scope.erase(scope.find(local));
	}
	for(int i = 0; i < ARRCNT; i++) {
		if(allIndexes[i] && !canVisit[i]) {
			DEBUG_OUTPUT << "Unit " << i << " cannot be visited but used. Free it." << endl;
			freeVariable(Value::makeRef(i * ARR_GROW));
		}
	}
	lookVariable();
	ENDEDL
}

Value callFunc(Value func, vector<Value> allParameter, bool level) {
#define CHECK_TYPE(a, n) \
	if(allParameter[a].type != T_##n) RUN_ERR("Built in function " + func.funVal + " required a value of '" + typeName[T_##n] + "' for parameter " + parseNum(a) + ", given: " + typeName[allParameter[a].type])
#define CHECK_ARG_CNT(n)\
    if(allParameter.size() != n) SYNTAX_ERR("Wrong argument count: " + parseNum(allParameter.size()) + " , need: " + parseNum(n))
    DEBUG_OUTPUT << "callFunc(): func = " << func.funVal << endl;
    if(func.type != T_FUNCTION) RUN_ERR(" can not call a non-function value, given: " + typeName[func.type]);

    if(func.funVal == "print") {
        DEBUG_OUTPUT << "print function built-in called\n";
        for(int i = 0; i < allParameter.size(); i++) cout << (outputSym ? "" : "") + allParameter[i].toString() << ' ';
        cout << endl;
        return Value::makeNumber(allParameter.size());
    } else if(func.funVal == "readonly") {
        for(int i = 0; i < allParameter.size(); i++) protectVar.add(allParameter[i].toString());
        return Value::makeNumber(allParameter.size());
    } else if(func.funVal == "exit") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        exit(allParameter[0].numVal);
    } else if(func.funVal == "xor") {
        CHECK_ARG_CNT(2);
        return allParameter[0] xor allParameter[1];
    } else if(func.funVal == "warfarin") {
        return Value::makeString(VERSION);
    } else if(func.funVal == "read_number") {
        double number;
        cin >> number;
        return Value::makeNumber(number);
    } else if(func.funVal == "read_string") {
        String str;
        cin >> str;
        return Value::makeString(str);
    } else if(func.funVal == "read_str_line") {
        String str;
        char c = getchar();
        while(isspace(c)) c = getchar();
        while(c != '\n') str += c, c = getchar();
        return Value::makeString(str);
    } else if(func.funVal == "file_read_number") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        return Value::makeNumber(freadNumber(allParameter[0].numVal));
    } else if(func.funVal == "file_read_string") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        return Value::makeString(freadString(allParameter[0].numVal));
    } else if(func.funVal == "file_read_str_line") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        return Value::makeString(freadStrLine(allParameter[0].numVal));
    } else if(func.funVal == "file_write_number") {
        CHECK_ARG_CNT(2);
        CHECK_TYPE(0, NUMBER);
        CHECK_TYPE(1, NUMBER);
        fwriteNumber(allParameter[0].numVal, allParameter[1].numVal);
        return Value::makeNumber(0);
    } else if(func.funVal == "file_write_string") {
        CHECK_ARG_CNT(2);
        CHECK_TYPE(0, NUMBER);
        CHECK_TYPE(1, STRING);
        fwriteString(allParameter[0].numVal, allParameter[1].strVal);
        return Value::makeNumber(0);
    } else if(func.funVal == "file_open") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, STRING);
        return Value::makeNumber(fileOpen(allParameter[0].strVal));
    } else if(func.funVal == "file_eof") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        return Value::makeNumber(feof(filePtr[allParameter[0].numVal]));
    } else if(func.funVal == "file_close") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        fileClose(allParameter[0].numVal);
        return Value::makeNumber(0);
    } else if(func.funVal == "len") {
        CHECK_ARG_CNT(1);
        Value v = allParameter[0];
        if(v.type == T_STRING) return Value::makeNumber(v.strVal.length());
        CHECK_TYPE(0, REF);
		int len = 0;
        int i = v.refVal;
        while(true) {
            if(heap[i].type == T_UNDEF) break;
            i++, len++;
        }
        return Value::makeNumber(len);
    } else if(func.funVal == "alert") {
    	#ifdef WIN32
        CHECK_ARG_CNT(1);
        Value v = allParameter[0];
        return Value::makeNumber(MessageBox(NULL, (LPCSTR)v.toString().c_str(), "", MB_OK));
        #endif
        return Value::makeNumber(0);
    } else if(func.funVal == "system") {
        CHECK_ARG_CNT(1);
        Value v = allParameter[0];
        String s = v.toString();
        return Value::makeNumber(system(s.c_str()));
    } else if(func.funVal == "clock") {
        CHECK_ARG_CNT(0);
        return Value::makeNumber(clock());
    } else if(func.funVal == "math_sin") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        Value v = allParameter[0];
        return Value::makeNumber(sin(v.numVal));
    } else if(func.funVal == "math_cos") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        Value v = allParameter[0];
        return Value::makeNumber(cos(v.numVal));
    } else if(func.funVal == "math_tan") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        Value v = allParameter[0];
        return Value::makeNumber(tan(v.numVal));
    } else if(func.funVal == "math_asin") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        Value v = allParameter[0];
        return Value::makeNumber(asin(v.numVal));
    } else if(func.funVal == "math_acos") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        Value v = allParameter[0];
        return Value::makeNumber(acos(v.numVal));
    } else if(func.funVal == "math_atan") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        Value v = allParameter[0];
        return Value::makeNumber(atan(v.numVal));
    } else if(func.funVal == "math_sqrt") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        Value v = allParameter[0];
        return Value::makeNumber(sqrt(v.numVal));
    } else if(func.funVal == "random") {
        CHECK_ARG_CNT(0);
        return Value::makeNumber(rand());
    } else if(func.funVal == "set_seed") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        Value v = allParameter[0];
        srand(v.numVal);
        return v;
    } else if(func.funVal == "eval") {
        VS tmp;
        VS EMP;
        vector<Value> EMV;
        for(int i = 0; i < allParameter.size(); i++) tmp.add(allParameter[i].toString());
        registFunc("<eval>", EMP, EMP, tmp);
        return callFunc(Value::makeFunction("<eval>"), EMV, true);
    } else if(func.funVal == "tostr") {
        vector<Value> arr;
        for(int i = 0; i < allParameter.size(); i++) {
            String r = "";
            r += (char)(allParameter[i].numVal);
            arr.add(Value::makeString(r));
        }
        return Value::makeRef(makeArray(arr).refVal);
    } else if(func.funVal == "toascii") {
        vector<Value> arr;
        for(int i = 0; i < allParameter.size(); i++) {
            Value v = allParameter[i];
            String str = v.toString();
            for(int j = 0; j < v.toString().length(); j++) arr.add(Value::makeNumber(str[j]));
        }
        return Value::makeRef(makeArray(arr).refVal);
    } else if(func.funVal == "substr") {
        CHECK_ARG_CNT(3);
    	CHECK_TYPE(0, STRING);
    	CHECK_TYPE(1, NUMBER);
    	CHECK_TYPE(2, NUMBER);
    	return Value::makeString(allParameter[0].strVal.substr(allParameter[1].numVal, allParameter[2].numVal));
    } 
	
	else if(func.funVal == "find_window") {
		#ifdef WIN32
    	CHECK_ARG_CNT(1);
        CHECK_TYPE(0, STRING);
    	HWND hwnd = FindWindow(NULL, (LPCSTR)allParameter[0].strVal.c_str());
    	if(hwnd == NULL) return Value::makeNull();
    	return Value::makeHWND(hwnd);
		#endif
		return Value::makeString("unsupported " + func.funVal);
	}
	else if(func.funVal == "get_foreground_window") {
		#ifdef WIN32
    	CHECK_ARG_CNT(0);
    	return Value::makeHWND(GetForegroundWindow());
		#endif
		return Value::makeString("unsupported " + func.funVal);
	}
	else if(func.funVal == "show_window") {
		#ifdef WIN32
    	CHECK_ARG_CNT(2);
        CHECK_TYPE(0, HWND);
    	return Value::makeBool(ShowWindow(allParameter[0].hwndVal, allParameter[1].numVal));
		#endif
		return Value::makeString("unsupported " + func.funVal);
	}
	else if(func.funVal == "set_window_text") {
		#ifdef WIN32
    	CHECK_ARG_CNT(2);
        CHECK_TYPE(0, HWND);
        CHECK_TYPE(1, STRING);
    	return Value::makeBool(SetWindowText(allParameter[0].hwndVal, (LPCSTR)allParameter[1].strVal.c_str()));
		#endif
		return Value::makeString("unsupported " + func.funVal);
	}
	else if(func.funVal == "get_async_key_state") {
		#ifdef WIN32
		CHECK_ARG_CNT(1);
        CHECK_TYPE(0, STRING);
		String key = allParameter[0].strVal;
		bool result = true;
		for(int i = 0; i < key.length(); i++) result = result && GetAsyncKeyState(key[i]);
		return Value::makeBool(result);
		#endif
		return Value::makeString("unsupported " + func.funVal);
	}
	else if(func.funVal == "sleep") {
		#ifdef WIN32
		CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
		Sleep(allParameter[0].numVal);
		return allParameter[0];
		#endif
		return Value::makeString("unsupported " + func.funVal);
	}
	else if(func.funVal == "typeof") {
		CHECK_ARG_CNT(1);
		return Value::makeString(typeName[allParameter[0].type]);
	}
	
    VS list = paramTable[func.funVal];
    VS block = funcTable[func.funVal];
    VS types = ptypeTable[func.funVal];

    DEBUG_OUTPUT << "got list and block of " << func.funVal << endl;

    Value returnVal = Value::makeNull();
    String frame;
	if(func.funVal == "<eval>") frame = "000";
	else frame = makeName();
	
	vector<String> indexTable;
	usedLocalVar.add(indexTable); 
    callStack.add(frame + func.funVal);

    DEBUG_OUTPUT << "ready to set parameters" << endl;
    DEBUG_OUTPUT << "list size: " << list.size() << endl;

    if(callStack.size() > INT_MAX) RUN_ERR(" stack overflow. To much nested-function!");

    for(int i = 0; i < list.size(); i++) {
    	if(types[i] != "<unlimited>") {
    		if(types[i] != typeName[allParameter[i].type]) RUN_ERR("Function '" + func.funVal + "' required '" + types[i] + "' for parameter " + parseNum(i) + ", but received '" + typeName[allParameter[i].type] + "'.");
		}
        setLocalVarVal(trim(list[i]), allParameter[i]);
        DEBUG_OUTPUT << "parameter: " << trim(list[i]) << " -> " << allParameter[i].toString() << endl;
    }

    DEBUG_OUTPUT << "begin to execBlock" << endl;
    if(level) {
        try {
            returnVal = execBlock(block);
        }
        catch(Value retException) {
            returnVal = retException;
            gc();
   			callStack.pop_back();
   			usedLocalVar.pop_back();
            return returnVal;
        }
    }
    else {
        returnVal = execBlock(block);
    }

    DEBUG_OUTPUT << "execBlock done. Return " << returnVal.toString() << endl;
    gc();
    callStack.pop_back();
    usedLocalVar.pop_back();
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

double toHex(String s) {
	int result = 0;
	int parsing[256];
	for(int i = 0; i <= 9; i++) parsing['0' + i] = i;
	for(int i = 'a'; i <= 'f'; i++) parsing[i] = i - 'a' + 10;
	for(int i = 2; i < s.length(); i++) result = result * 16 + parsing[tolower(s[i])];
	return result;
}

Value evalSimpleExpr(String s) {
    if(s[0] == '\'' || s[0] == '\"') return Value::makeString(stringExpr(MIDDLE(s)));
    if(isdigit(s[0])) {
    	if(s[0] == '0' && s[1] == 'x') return Value::makeNumber(toHex(s));
    	return Value::makeNumber(atof(s.c_str()));
	}
    else return getVarVal(s);
}

Value makeLambda(String, String);

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
    if(str.size() == 3 && str[0] == "lambda") {
    	return makeLambda(str[1], str[2]);
	}
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
        DEBUG_OUTPUT << "dealing with " << vs[i] << endl;
        if(st.empty()) DEBUG_OUTPUT << "current top = (nothing)" << endl;
        else DEBUG_OUTPUT << "current top = " << st.top().toString() << endl;
        #define CHECK() if(st.empty()) RUN_ERR("Invalid expression: " + expr)
		#define GET_OP() \
			CHECK();\
            op2 = st.top();\
            st.pop();\
			CHECK();\
            op1 = st.top();\
            st.pop();
		 
        if(vs[i] == "+") {
        	GET_OP();
            st.push(op1 + op2);
        } else if(vs[i] == "-") {
            GET_OP();
            st.push(op1 - op2);
        } else if(vs[i] == "*") {
            GET_OP();
            st.push(op1 * op2);
        } else if(vs[i] == "/") {
            GET_OP();
            st.push(op1 / op2);
        } else if(vs[i] == "%") {
            GET_OP();
            st.push(op2 % op1);
        } else if(vs[i] == ">") {
            GET_OP();
            st.push(op1 > op2);
        } else if(vs[i] == "<") {
            GET_OP();
            st.push(op1 < op2);
        } else if(vs[i] == ">=") {
            GET_OP();
            st.push(op1 >= op2);
        } else if(vs[i] == "<=") {
            GET_OP();
            st.push(op1 <= op2);
        } else if(vs[i] == "!=") {
            GET_OP();
            st.push(op1 != op2);
        } else if(vs[i] == "==") {
            GET_OP();
            st.push(op1 == op2);
        } else if(vs[i] == "!") {
        	CHECK();
            op2 = st.top();
            st.pop();
            st.push(!op2);
        } else if(vs[i] == "&&") {
            GET_OP();
            st.push(op1 && op2);
        } else if(vs[i] == "||") {
            GET_OP();
            st.push(op1 || op2);
        } else if(vs[i] == "&") {
            GET_OP();
            st.push(op1 & op2);
        } else if(vs[i] == "|") {
            GET_OP();
            st.push(op1 | op2);
        } else if(vs[i] == "@") {
            GET_OP();
            st.push(getRefVal(op1, op2));
        } else if(vs[i] == "^") {
            String parameter;
            CHECK();
            op2 = st.top();
            st.pop();
            parameter = op2.toString();
            CHECK();
            op1 = st.top();
            st.pop();
            DEBUG_OUTPUT << "parameter : " + op2.toString() << endl;

            VS args = removeAll(split(parameter, ','), ",");
            ENUM_VS(args, i) DEBUG_OUTPUT << "args : " << args[i] << endl;

            vector<Value> vals;
            ENUM_VS(args, i) vals.add(evalExpr(args[i]));

            st.push(callFunc(op1, vals, true));
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
	setVarRef(var, evalExpr(exp));
}

void setVarRef(String var, Value exp) {
    BEGINL
    ENUM_VS(protectVar, i) {
    	if(var == protectVar[i]) RUN_ERR(" access denied: " + var + " can not be modified.");
	}
	if(findchar(var, '.')) var = parseSubscript(var);
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
	VS mainLocal;
	usedLocalVar.add(mainLocal);
    callStack.add("main");
    heap.resize(ARR_GROW * ARRCNT);
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
    BIND("&", 65, BITAND);
    BIND("|", 64, BITOR);
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
    setVarVal("read_number", Value::makeFunction("read_number"));
    setVarVal("read_string", Value::makeFunction("read_string"));
    setVarVal("read_str_line", Value::makeFunction("read_str_line"));
    setVarVal("file_read_number", Value::makeFunction("file_read_number"));
    setVarVal("file_read_string", Value::makeFunction("file_read_string"));
    setVarVal("file_read_str_line", Value::makeFunction("file_read_str_line"));
    setVarVal("file_write_number", Value::makeFunction("file_write_number"));
    setVarVal("file_write_string", Value::makeFunction("file_write_string"));
    setVarVal("file_open", Value::makeFunction("file_open"));
    setVarVal("file_close", Value::makeFunction("file_close"));
    setVarVal("file_eof", Value::makeFunction("file_eof"));
    setVarVal("math_asin", Value::makeFunction("math_asin"));
    setVarVal("math_acos", Value::makeFunction("math_acos"));
    setVarVal("math_atan", Value::makeFunction("math_atan"));
    setVarVal("math_sqrt", Value::makeFunction("math_sqrt"));
    setVarVal("random", Value::makeFunction("random"));
    setVarVal("set_seed", Value::makeFunction("set_seed"));
    setVarVal("eval", Value::makeFunction("eval"));
    setVarVal("toascii", Value::makeFunction("toascii"));
    setVarVal("tostr", Value::makeFunction("tostr"));
    setVarVal("xor", Value::makeFunction("xor"));
    setVarVal("math_pi", Value::makeNumber(3.14159265358797932384626));
    setVarVal("null", Value::makeNull());
    setVarVal("true", Value::makeBool(true));
    setVarVal("false", Value::makeBool(false));
    setVarVal("readonly", Value::makeFunction("readonly"));
    setVarVal("typeof", Value::makeFunction("typeof"));
    setVarVal("find_window", Value::makeFunction("find_window"));
    setVarVal("show_window", Value::makeFunction("show_window"));
    setVarVal("get_foreground_window", Value::makeFunction("get_foreground_window"));
    setVarVal("set_window_text", Value::makeFunction("set_window_text"));
    setVarVal("sleep", Value::makeFunction("sleep"));
    setVarVal("substr", Value::makeFunction("substr"));
    setVarVal("get_async_key_state", Value::makeFunction("get_async_key_state"));
    
    protectVar.add("null");
    protectVar.add("true");
    protectVar.add("false");
    
}

enum { EXPR, ASSIGN };

Value execute(String stat) {
    BEGINL
    DEBUG_OUTPUT << "execute: " << stat << endl;
    VS vs = tokenize(stat);
    int i = 0;
    int type = EXPR;
    String flag = vs[0];

    bool inq = false;
	
	int x;
    ENUM_VS(vs, i) {
    	if(vs[i] == "=") {
    		type = ASSIGN, x = i;
    		break;
		}
	}
	
	DEBUG_OUTPUT << "stmt = " << stat << " type: " << (type == EXPR ? "Expression" : "Assignment") << endl;
	
	
    String left;
    String right;
    
    if(type == ASSIGN) {
    	for(int i = 0; i < x; i++) left += vs[i];
   		for(int j = x + 1; j < vs.size(); j++) right += vs[j];
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

void registFunc(String func, VS list, VS ptype, VS block) {
    DEBUG_OUTPUT << "Regist function: " << func << endl;
    DEBUG_OUTPUT << "List: " << joinString(list) << " size: " << list.size() << endl;
    funcTable[func] = block;
    paramTable[func] = list;
    ptypeTable[func] = ptype;
}

void readCodeFrom(String fileName) {
    ifstream fcin(fileName.c_str());
    String s;
    while(getline(fcin, s)) {
        if(s == "") continue;
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
        if(s == "") continue;
        VS vs = tokenize(s);
        ENUM_VS(vs, i) DEBUG_OUTPUT << "vs[" << i << "] = " << vs[i] << endl;
        if(vs[0] == "import") {
            String name = "";
            for(int i = 1; i < vs.size(); i++) name += vs[i];
            readCodeFrom(name);
        } else codeStream << s << endl;
        cout << ">>> ";
    }
}

VS tinyBlock(String stat) {
    VS vs;
    vs.add(stat);
    return vs;
}

VS makeBlock() {
    String s;
    VS ret;
    BEGINL
    while(getline(codeStream, s)) {
        VS vs;
        s = trim(s);
        vs = tokenize(s);
        ENUM_VS(vs, k) {
        	DEBUG_OUTPUT << "block statement " << s << " tokenized -> " << "vs[" << k <<"] = " << vs[k] << " length = " << vs[k].length() << endl;
		}
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
            if(vs.size() >= 3 && vs[2][0] == '{') {
                block = makeBlock();
            }
            else {
                String statement;
                for(int i = 2; i < vs.size(); i++) statement += vs[i] + " ";
                block = tinyBlock(statement);
            }
            DEBUG_OUTPUT << "if stat : " << stat << endl;
            registFunc(rname, EMPTY, EMPTY, block);
            ret.add(stat);
        } else if(vs[0] == "while") {
            String expr = MIDDLE(vs[1]);
            String stat = "while (";
            String rname = makeName();
            VS block;
            stat += expr;
            stat += ") ";
            stat += rname;
            if(vs.size() >= 3 && vs[2][0] == '{') {
            	DEBUG_OUTPUT << "normal block: name = " << rname << endl;
                block = makeBlock();
            }
            else {
                String statement;
                for(int i = 2; i < vs.size(); i++) statement += vs[i] + " ";
                block = tinyBlock(statement);
            }
            DEBUG_OUTPUT << "while stat : " << stat << endl;
            registFunc(rname, EMPTY, EMPTY, block);
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
            if(vs.size() >= 3 && vs[2][0] == '{') {
                block = makeBlock();
            }
            else {
                String statement;
                for(int i = 2; i < vs.size(); i++) statement += vs[i] + " ";
                block = tinyBlock(statement);
            }
            DEBUG_OUTPUT << "for stat : " << stat << endl;
            registFunc(rname, EMPTY, EMPTY, block);
            ret.add(stat);
        } else if(vs[0] == "function") {
            String name = vs[1];
            String expr = MIDDLE(vs[2]);
            VS args = removeAll(buildWith(split(expr, ','), ","), ",");
            VS finalArgs;
            VS ptypeArgs;
            ENUM_VS(args, i) {
                if(args[i] != "") {
                	String src = args[i];
                	if(includeChar(src, ':')) {
                		int pos;
                		for(int i = 0; i < src.length(); i++) if(src[i] == ':') {
                			pos = i;
                			break;
						}
                		finalArgs.add(trim(src.substr(0, pos)));
                		ptypeArgs.add(trim(src.substr(pos + 1, src.length())));
					}
                	else {
                		finalArgs.add(trim(args[i]));
                		ptypeArgs.add("<unlimited>");
					}
				}
            }

            String rname = name;
            VS block = makeBlock();

            setVarRef(name, Value::makeFunction(rname));
			ENUM_VS(finalArgs, i) {
				DEBUG_OUTPUT << "argument: " << finalArgs[i] << " (" << ptypeArgs[i] << ") " << endl;
			}
            DEBUG_OUTPUT << "function defined. " << name << " is " << rname << endl;
            registFunc(rname, finalArgs, ptypeArgs, block);
        } else if(vs[0] == "else") {
            String stat = "else (";
            String rname = makeName();
            VS block;
            stat += rname;
            stat += ")";
            if(vs.size() >= 2 && vs[1][0] == '{') {
                block = makeBlock();
            }
            else {
                String statement;
                for(int i = 1; i < vs.size(); i++) statement += vs[i] + " ";
                block = tinyBlock(statement);
            }
            DEBUG_OUTPUT << "else stat branch: " << stat << endl;
            registFunc(rname, EMPTY, EMPTY, block);
            ret.add(stat);
        } else if(vs[0] == "elseif") {
            String expr = MIDDLE(vs[1]);
            String stat = "elseif (";
            String rname = makeName();
            VS block;
            stat += expr;
            stat += ") ";
            stat += rname;
            if(vs.size() >= 3 && vs[2][0] == '{') {
                block = makeBlock();
            }
            else {
                String statement;
                for(int i = 2; i < vs.size(); i++) statement += vs[i] + " ";
                block = tinyBlock(statement);
            }
            DEBUG_OUTPUT << "elseif stat : " << stat << endl;
            registFunc(rname, EMPTY, EMPTY, block);
            ret.add(stat);
        }
        else ret.add(s);
    }
    ENDEDL
    return ret;
}

Value execBlock(VS block);

Value execBlock(VS block) {
    ENUM_VS(block, index) {
        currentCommand = block[index];
        DEBUG_OUTPUT << "execBlock currentCommand = " << block[index] << endl;
        VS token = tokenize(trim(block[index]));
        VS nextLine;
        bool hasNext = false;
        if(index < block.size() - 1) {
            nextLine = tokenize(trim(block[index + 1]));
            hasNext = true;
        }
        if(token[0] == "if") {
            String condition;
            VS branches;
            VS conditions;
            branches.add(token[2]);
            conditions.add(MIDDLE(token[1]));
        	index++;
            while(index < block.size() ) {
                VS curLine = tokenize(block[index]);
                if(curLine[0] == "elseif") {
                    branches.add(curLine[2]);
                    conditions.add(MIDDLE(curLine[1]));
                    index++;
                }
                else if(curLine[0] == "else") {
                    branches.add(MIDDLE(curLine[1]));
                    conditions.add("true");
                    index++;
                    break;
                }
                else break;
            }
            index--;
            for(int i = 0; i < branches.size(); i++) {
                condition = conditions[i];
                if((!evalExpr(condition)).type == T_FALSE) {
                    callFunc(Value::makeFunction(branches[i]), EMPTY_V, false);
                    break;
                }
            }
        } else if(token[0] == "while") {
            String condition = MIDDLE(token[1]);
            while((!evalExpr(condition)).type == T_FALSE) {
            	try {
            		callFunc(Value::makeFunction(token[2]), EMPTY_V, false);
				}
				catch(LoopMessage loop) {
					if(loop.type == BREAK) break;
					else continue;
				}
			}
        } else if(token[0] == "for") {
        	DEBUG_OUTPUT << "running for " << endl;
            String variable = MIDDLE(token[1]);
            String from = MIDDLE(token[2]);
            String condition = MIDDLE(token[3]);
            Value step = evalExpr(MIDDLE(token[4]));
            DEBUG_OUTPUT << "variable = " << variable << " from = " << from << endl; 
            for(
                setLocalVarVal(variable, evalExpr(from));
                !isFalse(evalExpr(condition));
                setLocalVarVal(variable, getVarVal(variable) + step)
            ) {
            	try {
            		callFunc(Value::makeFunction(token[5]), EMPTY_V, false);
				}
				catch(LoopMessage loop) {
					if(loop.type == BREAK) break;
					else continue;
				}
			}
        } else if(token[0] == "return") {
            String r = "";
            for(int i = 1; i < token.size(); i++) r += token[i];
            Value throwable = evalExpr(r);
            throw throwable;
            return throwable;
        } else if(token[0] == "else") {
            ;
        } else if(token[0] == "break") {
            LoopMessage loop;
            loop.type = BREAK;
            throw loop;
        } else if(token[0] == "continue") {
            LoopMessage loop;
            loop.type = CONTINUE;
            throw loop;
        } else if(token[0] == "goto") {
        	
		}
		
		else {
			DEBUG_OUTPUT << "Not structure symbol, execute -> " << trim(block[index]) << endl;
			execute(trim(block[index]));
		}
    }
    return Value::makeNull();
}

String parseArg(String arg) {
    return arg.substr(1, arg.length());
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
        VS tList;
        VS cmdList;
        int paramCnt;
        int lineCnt;
        getline(fcin, funcName);
//      cout << funcName << endl;
        getline(fcin, temp);
        paramCnt = atof(temp.c_str());
        for(int i = 0; i < paramCnt; i++) {
            String pName;
            String finalP;
            String tName = "<unlimited>";
            getline(fcin, pName);
            int x;
            finalP = pName;
            for(int i = 0; i < pName.length(); i++) {
            	if(pName[i] == ':') {
            		finalP = pName.substr(0, i);
            		tName = pName.substr(i + 1, pName.length());
            		break;
				}
			}
            pList.push_back(finalP);
            tList.push_back(tName);
        }
        getline(fcin, temp);
        lineCnt = atof(temp.c_str());
        for(int i = 0; i < lineCnt; i++) {
            String s;
            getline(fcin, s);
            cmdList.push_back(s);
//          cout << "cmd #" << i + 1 << ": " << cmdList[i] << endl;
        }
        registFunc(funcName, pList, tList, cmdList);
        setVarVal(funcName, Value::makeFunction(funcName));
    }
}

String codefileName;

String getOutputFile(String ext = ".ff0") {
    for(int i = codefileName.size() - 1; i >= 0; i--) {
        if(codefileName[i] == '.') {
            return codefileName.substr(0, i) + ext;
        }
    }

}

void compile() {
    ofstream fcout(getOutputFile().c_str());
    map<String, VS>::iterator paramIt = paramTable.begin();
    map<String, VS>::iterator ptypeIt = ptypeTable.begin(); 
    fcout << "ff0" << endl;
    fcout << funcTable.size() << endl;
    for(map<String, VS>::iterator it = funcTable.begin(); it != funcTable.end(); it++) {
        fcout << it -> first << endl;
        fcout << paramIt -> second.size() << endl;
        for(int i = 0; i < paramIt -> second.size(); i++) {
        	fcout << paramIt -> second[i];
        	if(ptypeIt -> second[i] != "<unlimited>") fcout << ":" << ptypeIt -> second[i];
        	fcout << endl;
		}
        fcout << it -> second.size() << endl;
        for(int i = 0; i < it -> second.size(); i++) fcout << it -> second[i] << endl;
        paramIt++;
        ptypeIt++;
    }
}
map<String, bool> uncomped;

String gettab(int x) {
	String s = "";
	for(int i = 0; i < x; i++) s += "    ";
	return s;
}

String uncompile(String func, int tabs) {
	stringstream result("");
	uncomped[func] = true;
	VS block = funcTable[func];
	ENUM_VS(block, index) {
		currentCommand = block[index];
        VS token = tokenize(trim(block[index]));
        VS nextLine;
        bool hasNext = false;
        if(index < block.size() - 1) {
            nextLine = tokenize(trim(block[index + 1]));
            hasNext = true;
        }
        if(token[0] == "if") {
            String condition;
            VS branches;
            VS conditions;
            branches.add(token[2]);
            conditions.add(MIDDLE(token[1]));
        	index++;
            while(index < block.size() ) {
                VS curLine = tokenize(block[index]);
                if(curLine[0] == "elseif") {
                    branches.add(curLine[2]);
                    conditions.add(MIDDLE(curLine[1]));
                    index++;
                }
                else if(curLine[0] == "else") {
                    branches.add(MIDDLE(curLine[1]));
                    conditions.add("true");
                    index++;
                    break;
                }
                else break;
            }
            index--;
            result << gettab(tabs) << "if(" << conditions[0] << ") {" << endl;
            result << uncompile(branches[0], tabs + 1);
            result << gettab(tabs) << "}" << endl;
            for(int i = 1; i < branches.size(); i++) {
                condition = conditions[i];
                result << gettab(tabs) << "else if(" << condition << ") {" << endl;
                result << uncompile(branches[i], tabs + 1);
                result << gettab(tabs) << "}" << endl;
            }
        } else if(token[0] == "while") {
            String condition = MIDDLE(token[1]);
            result << gettab(tabs) << "while(" << condition << ") {" << endl;
            result << uncompile(token[2], tabs + 1);
            result << gettab(tabs) << "}" << endl;
        } else if(token[0] == "for") {
            String variable = MIDDLE(token[1]);
            String from = MIDDLE(token[2]);
            String condition = MIDDLE(token[3]);
            Value step = evalExpr(MIDDLE(token[4]));
            result << gettab(tabs) << "for(var " << variable << " = " << from << "; ";
			result << condition << "; " << variable << " = " << variable << " + " << MIDDLE(token[4]) << ") {" << endl;
			result << uncompile(token[5], tabs + 1);
			result << gettab(tabs) << "}" << endl;
        } else {
        	String stat = block[index];
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
			    for(i = 0; i < stat.length(); i++) {
					if(i > 0 && stat[i - 1] != '\\' && stat[i] == '\"') inq = !inq;
					if(stat[i] == '{' && !inq) {
						stat[i] = '[';
					}
					if(stat[i] == '}' && !inq) {
					    stat[i] = ']';
					}
				}
		    }
        	result << gettab(tabs) << trim(stat) << ";" << endl;
		}
	}
	return result.str();
}

stringstream jscout("");

String javascript() {
	map<String, VS>::iterator paramIt = paramTable.begin();
    for(map<String, VS>::iterator it = funcTable.begin(); it != funcTable.end(); it++) {
        String fun = it -> first;
        if(fun == "<main>") {
        	paramIt++;
        	continue;
		}
        if(!uncomped[fun] && fun[0] != '~') {
        	jscout << "function " << fun << " ( ";
			for(int i = 0; i < paramIt -> second.size(); i++) {
				String s = paramIt -> second[i];
				for(int x = 0; x < s.length(); x++) {
					if(s[x] == ':') {
						jscout << s.substr(0, x - 1);
						break;
					}
				}
				jscout << paramIt -> second[i] << ",";
			}
			String cur = jscout.str();
			cur[cur.length() - 1] = ')';
			jscout.clear(); jscout << cur;
			jscout << " {" << endl;
        	jscout << uncompile(fun, 1) << endl;
        	jscout << "}" << endl;
		}
        	paramIt++;
    }
   	jscout << uncompile("<main>", 0) << endl;
   	return jscout.str();
}

void initUncompile() {
	jscout << "print = function (a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) { if(a) console.log(a);\n"
	"if(b) console.log(b); if(c) console.log(c); if(d) console.log(d); if(e) console.log(e); if(f) console.log(f); \n"
	"if(g) console.log(g); if(h) console.log(h); if(i) console.log(i); if(j) console.log(j); if(k) console.log(k); \n"
	"if(l) console.log(l); if(m) console.log(m); if(n) console.log(n); if(o) console.log(o); if(p) console.log(p); \n}" << endl;
	jscout << "len = function (a) { return a.length; }" << endl;
	jscout << "warfarin = function () { return \"" << VERSION << "\"; }" << endl;
	jscout << "math_sin = function (a) { return Math.sin(a); }" << endl;
	jscout << "math_cos = function (a) { return Math.cos(a); }" << endl;
	jscout << "math_tan = function (a) { return Math.tan(a); }" << endl;
	jscout << "math_asin = function (a) { return Math.asin(a); }" << endl;
	jscout << "math_acos = function (a) { return Math.acos(a); }" << endl;
	jscout << "math_atan = function (a) { return Math.atan(a); }" << endl;
	jscout << "math_sqrt = function (a) { return Math.sqrt(a); }" << endl;
	jscout << "random = function () { return Math.random(); }" << endl;
	jscout << "set_seed = function (a) {}" << endl;
	jscout << "xor = function (a, b) { return a ^ b; }" << endl;
	jscout << "clock = function (a, b) { return new Date().getTime(); }" << endl;
	jscout << "read_number = function () { return 0; }" << endl;
	jscout << "read_string = function () { return \"\"; }" << endl;
	jscout << "read_str_line = function () { return \"\"; }" << endl;
	jscout << "exit = function (a) {}" << endl;
	jscout << "system = function (a) {}" << endl;
	jscout << "readonly = function (a) {}" << endl;
	jscout << 	"toascii = function (a) {\n"
				"	ans = [];\n"
				"	for(var i = 0; i < a.length(); i++) ans[i] = a[i].charCodeAt();\n"
				"	return ans;\n"
				"}" << endl;
	jscout << 	"tostr = function (a) {\n"
				"	ans = [];\n"
				"	for(var i = 0; i < a.length(); i++) ans[i] = String.fromCharCode(a[i]);\n"
				"	return ans;\n"
				"}" << endl;
}

Value makeLambda(String param, String ret) {
	ret = MIDDLE(ret);
    String expr = MIDDLE(param);
    VS args = removeAll(buildWith(split(expr, ','), ","), ",");
    VS finalArgs;
    VS empty;
    ENUM_VS(args, i) {
        if(args[i] != "") finalArgs.add(args[i]);
    }
    
    String rname = makeName();
    VS block;
    block.add("return " + ret);
	
	Value retval = Value::makeFunction(rname);

    DEBUG_OUTPUT << "lambda function defined" << rname << endl;
    registFunc(rname, finalArgs, empty, block);
    return retval;
}

bool mCompile = false;
bool mJavaScript = false;
bool mExecute = true;

int main(int argc, char ** argv) {
    init();
    initUncompile();
    if(argc == 1) {
        cout << VERSION << endl;
        readCode();
        codefileName = "a.ff0";
        try {
        	registFunc("<main>", EMPTY, EMPTY, makeBlock());
		}
		catch(ExecuteException ex) {
            cout << ex.msg << endl;
            exit(1);
		}
        compile();
    } else {
        if(argc == 2) {
            codefileName = argv[1];
        } else {
            codefileName = argv[1];
            mExecute = false;
            String todo = parseArg(argv[2]);
            for(int i = 0; i < todo.length(); i++) {
                if(todo[i] == 'c') mCompile = true;
                if(todo[i] == 'r') mExecute = true;
                if(todo[i] == 'j') mJavaScript = true;
            }
        }
    }
    try {
        if(mCompile) {
            try {
                readCodeFrom(argv[1]);
                registFunc("<main>", EMPTY, EMPTY, makeBlock());
            } catch(ExecuteException ex) {
                cout << ex.msg << endl;
                exit(1);
            }
            compile();

        }
        if(mExecute) {
            readByteCode(getOutputFile());
            try {
                vector<Value> EMPTY_LIST;
                Value starter = Value::makeFunction("<main>");
                callFunc(starter, EMPTY_LIST, true);
            } catch(ExecuteException ex) {
                cout << ex.msg << endl;
                exit(1);
            }
        }
        if(mJavaScript) {
        	ofstream jscoutx(getOutputFile(".js").c_str());
        	jscoutx << javascript() << endl;
		}
    } catch(ExecuteException ex) {
        cout << ex.msg << endl;
        exit(1);
    }
    lookVariable();
    return 0;
}
