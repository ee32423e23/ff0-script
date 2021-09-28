#include <bits/stdc++.h>

using namespace std;

const string VM_NAME = "FF0Script";
const string VM_VERSION = "4.01";
bool debug = false;

#define STEP (void(printf("Step: %d\n", __LINE__))) 
#define OPCODE const unsigned char

const int MAX_LENGTH = 1024 * 1024;
const int MAX_CPOOL = 1024 * 64;
const int MAX_FUNC_CNT = 1024 * 4;
const int MAX_REG_CNT = 1024 * 128;
const int MAX_HEAP_SIZE = 1024 * 1024;

const char NUM_TYPE = '0';
const char STR_TYPE = 'a';
const char FUNC_TYPE = 'F';
const char REF_TYPE = '@';
const char NUL_TYPE = 'x';
const char RNG_TYPE = '[';
const char UNDEFINED_TYPE = '?'; 

typedef vector<unsigned char> ByteList;
map<string, int> bind_power;

string tostr(double n) {
	stringstream ss("");
	ss << n;
	return ss.str();
}

const int END_RANGE = -1;

struct Range {
	int from, to;
	Range() {
		from = 0, to = END_RANGE;
	}
	Range(int f, int t) {
		from = f, to = t;
		if(to < from) swap(to, from);
	}
};

Range make_range(int a, int b) {
	return Range(a, b);
}

Range make_front_range(int x) {
	return Range(0, x);
}

Range make_back_range(int x) {
	return Range(0, END_RANGE);
}

Range make_range() {
	return Range();
}

struct Val {
	int ref;
	double num;
	string str;
	string func;
	char type;
	Range rng;
	string to_str() {
		if(type == NUM_TYPE) return tostr(num);
		if(type == STR_TYPE) return str;
		if(type == FUNC_TYPE) return '<' + func + '>';
		if(type == REF_TYPE) return '$' + tostr(ref);
		if(type == RNG_TYPE) return "[" + tostr(rng.from) + ", " + tostr(rng.to) + "]";
		if(type == UNDEFINED_TYPE) return "undefined";
		return "null"; 
	}
	
	Val(double a) {
		type = NUM_TYPE, num = a;
	}
	Val(string a) {
		type = STR_TYPE, str = a;
	}
	Val() {
		type = UNDEFINED_TYPE;
	}
	Val(Range r) {
		type = RNG_TYPE;
		rng = r;
	}
};

Val func_val(string f) {
	Val v;
	v.func = f;
	v.type = FUNC_TYPE;
	return v;
}

Val null_val() {
	Val v;
	v.type = NUL_TYPE;
	return v;
}

Val ref_val(int f) {
	Val v;
	v.type = REF_TYPE;
	v.ref = f;
	return v;
}

Val make_num(double n) {
	Val v;
	v.type = NUM_TYPE, v.num = n;
	return v;
}

Val make_str(string n) {
	Val v;
	v.type = STR_TYPE, v.str = n;
	return v;
}

const string TIP = "FF0Script Virtual Machine has caught an error.\n";

void error(string msg, string type = "VerifyError") {
	cout << TIP << type << ": \"" << msg << "\"" << endl;
	exit(0);
}

bool include_char(string, char);

const int MAX_FILE_CNT = 512;
struct FileManager {
	FILE * file_ptrs[MAX_FILE_CNT];
	int size;
	FileManager() {
		size = 0;
	}
	int file_open(string file, string mode = "r") {
		bool write = false;
		if(include_char(mode, 'w')) write = true;
		FILE * ptr = fopen(file.c_str(), mode.c_str());
		if(ptr == NULL) {
			if(write) {
				ofstream fcout(file.c_str());
				fcout.close();
			}
			else error("file doesn't exist: " + file);
		}
		file_ptrs[size++] = ptr;
		return size - 1;
	}
	
	void check_handle(int handle) {
		if(handle < 0 || handle >= size) error("invalid handle: " + tostr(handle));
	}
	
	void file_close(int handle) {
		check_handle(handle);
		fclose(file_ptrs[handle]);
	}
	
	bool file_eof(int handle) {
		check_handle(handle);
		return feof(file_ptrs[handle]);
	}
	
	char fbuffer[2048];
	string fread_string(int handle) {
		check_handle(handle);
		fscanf(file_ptrs[handle], "%s", fbuffer);
		return fbuffer;
	}
	string fread_line(int handle) {
		check_handle(handle);
		string result = "";
		char c;
		while(isspace(c) && !feof(file_ptrs[handle])) c = fgetc(file_ptrs[handle]);
		c = fgetc(file_ptrs[handle]);
		while(c != '\n' && !feof(file_ptrs[handle])) {
			result += c;
			c = fgetc(file_ptrs[handle]);
		}
		return result;
	}
	void fwrite_string(int handle, string text) {
		check_handle(handle);
		fprintf(file_ptrs[handle], "%s", text.c_str());
		fflush(file_ptrs[handle]);
	}
	double fread_number(int handle) {
		check_handle(handle);
		double x;
		return atof(fread_string(handle).c_str());
	}
	void fwrite_number(int handle, double x) {
		check_handle(handle);
		fprintf(file_ptrs[handle], "%.14g", x);
		fflush(file_ptrs[handle]);
	}
};

FileManager file_manager;

#define DEFINE_OP(x) \
Val operator x (Val a, Val b) {\
	Val c;\
	if(a.type == NUM_TYPE && b.type == NUM_TYPE) {\
		c = make_num(a.num x b.num);\
	}\
	else {\
		error("type doesn't match");\
	}\
	return c;\
}

#define DEFINE_IOP(x) \
Val operator x (Val a, Val b) {\
	Val c;\
	if(a.type == NUM_TYPE && b.type == NUM_TYPE) {\
		c = make_num((int)a.num x (int)b.num);\
	}\
	else {\
		error("type doesn't match");\
	}\
	return c;\
}

Val operator + (Val a, Val b) {
	Val c;
	if(a.type == STR_TYPE || b.type == STR_TYPE) {
		c = make_str(a.to_str() + b.to_str());
	}
	else if(a.type == NUM_TYPE && b.type == NUM_TYPE) {
		c = make_num(a.num + b.num);
	}
	else {
		error("type doesn't match");
	}
	return c;
}

Val operator % (Val a, Val b) {
	Val c;
	if(a.type == NUM_TYPE && b.type == NUM_TYPE) {
		c = fmod(a.num, b.num);
	}
	else {
		error("type doesn't match");
	}
	return c;
}

Val operator ! (Val a) {
	if(a.type == STR_TYPE) return a.str == "";
	if(a.type == NUM_TYPE) return a.num == 0;
	return false;
}

Val operator ~ (Val a) {
	if(a.type == NUM_TYPE) {
		return ~(int)a.num;
	}
	else error("type doesn't match");
}

DEFINE_OP(-) 
DEFINE_OP(*)
DEFINE_OP(/) 
DEFINE_OP(&&)
DEFINE_OP(||) 
DEFINE_OP(>=) 
DEFINE_OP(<=) 
DEFINE_OP(>) 
DEFINE_OP(<) 
DEFINE_OP(!=) 
DEFINE_OP(==)
DEFINE_IOP(>>) 
DEFINE_IOP(<<) 
DEFINE_IOP(&) 
DEFINE_IOP(|)
DEFINE_IOP(^) 

Val cpool[MAX_CPOOL];
int cpool_pos;

int pos;
int start[MAX_FUNC_CNT];
int callcnt;

void add_const(Val x) {
	cpool[cpool_pos++] = x;
}

Val runtime_stack[100000 + 5];
struct RunStack {
	int t;
	Val pop() {
		Val r = runtime_stack[t - 1];
		t--;
		return r;
	}
	void push(Val val) {
		runtime_stack[t++] = val;
	}
	Val top() {
		return runtime_stack[t - 1];
	}
	bool empty() {
		return t == 0;
	}
	RunStack() {
		t = 0;
	}
	RunStack(int offset) {
		t = offset;
	}
};

struct Register {
	Val r[MAX_REG_CNT];
};

OPCODE LDC0 = 0x80;
OPCODE LDC1 = 0x81;
OPCODE LDC2 = 0x82;
OPCODE LDC3 = 0x83;
OPCODE LDC4 = 0x84;
OPCODE LDC5 = 0x85;
OPCODE LDC6 = 0x86;
OPCODE LDC7 = 0x87;
OPCODE LD0 = 0x90;
OPCODE LD1 = 0x91;
OPCODE LD2 = 0x92;
OPCODE LD3 = 0x93;
OPCODE LD4 = 0x94;
OPCODE LD5 = 0x95;
OPCODE LD6 = 0x96;
OPCODE LD7 = 0x97;
OPCODE LD8 = 0x98;
OPCODE LD9 = 0x99;
OPCODE LD10 = 0x9a;
OPCODE LD11 = 0x9b;
OPCODE LD12 = 0x9c;
OPCODE LD13 = 0x9d;
OPCODE LD14 = 0x9e;
OPCODE LD15 = 0x9f;
OPCODE ADD = 0xa0;
OPCODE SUB = 0xa1;
OPCODE MUL = 0xa2;
OPCODE DIV = 0xa3;
OPCODE NOT = 0xa4;
OPCODE MOD = 0xa5;
OPCODE LAND = 0xa6;
OPCODE LOR = 0xa7;
OPCODE BAND = 0xa8;
OPCODE BOR = 0xa9;
OPCODE XOR = 0xaa;
OPCODE BIG = 0xab;
OPCODE BIGE = 0xac;
OPCODE SML = 0xad;
OPCODE SMLE = 0xae;
OPCODE EQL = 0xaf;
OPCODE NEQL = 0xb0;
OPCODE LSHF = 0xb1;
OPCODE RSHF = 0xb2;
OPCODE NEG = 0xb3;
OPCODE LDNUL = 0xb4;
OPCODE BNOT = 0xb5;
OPCODE MKRG = 0xb6;
OPCODE LOOP = 0xc0;
OPCODE POP = 0xc1;
OPCODE MKARR = 0xc2;
OPCODE JMP = 0xc3;
OPCODE JMPN = 0xc4;
OPCODE LDC = 0xc5;
OPCODE RET = 0xc6;
OPCODE DUP = 0xc7;
OPCODE CALL = 0xc8;
OPCODE STR = 0xc9;
OPCODE LOAD = 0xca;
OPCODE STORE = 0xcb; 
OPCODE LDN = 0xcc;
OPCODE MGET = 0xcd;
OPCODE MSET = 0xce;
OPCODE GETSUB = 0xcf;
OPCODE LOOPIF = 0xd0;
OPCODE FUNC = 0xd1;
OPCODE LSTORE = 0xd2;
OPCODE BREAK = 0xd3;
OPCODE CNTN = 0xd4;
OPCODE RESET = 0xd5;
OPCODE LRMV = 0xd6;
OPCODE NOP = 0xd7;
OPCODE END = 0xff;

string opname[256];

void init() {
opname[LDC0] = "LDC0";
opname[LDC1] = "LDC1";
opname[LDC2] = "LDC2";
opname[LDC3] = "LDC3";
opname[LDC4] = "LDC4";
opname[LDC5] = "LDC5";
opname[LDC6] = "LDC6";
opname[LDC7] = "LDC7";
opname[LD0] = "LD0";
opname[LD1] = "LD1";
opname[LD2] = "LD2";
opname[LD3] = "LD3";
opname[LD4] = "LD4";
opname[LD5] = "LD5";
opname[LD6] = "LD6";
opname[LD7] = "LD7";
opname[LD8] = "LD8";
opname[LD9] = "LD9";
opname[LD10] = "LD10";
opname[LD11] = "LD11";
opname[LD12] = "LD12";
opname[LD13] = "LD13";
opname[LD14] = "LD14";
opname[LD15] = "LD15";
opname[ADD] = "ADD";
opname[SUB] = "SUB";
opname[MUL] = "MUL";
opname[DIV] = "DIV";
opname[NOT] = "NOT";
opname[MOD] = "MOD";
opname[LAND] = "LAND";
opname[LOR] = "LOR";
opname[BAND] = "BAND";
opname[BOR] = "BOR";
opname[XOR] = "XOR";
opname[BIG] = "BIG";
opname[BIGE] = "BIGE";
opname[SML] = "SML";
opname[SMLE] = "SMLE";
opname[EQL] = "EQL";
opname[NEQL] = "NEQL";
opname[LSHF] = "LSHF";
opname[RSHF] = "RSHF";
opname[NEG] = "NEG";
opname[BNOT] = "BNOT";
opname[MKRG] = "MKRG";
opname[LDNUL] = "LDNUL";
opname[LOOP] = "LOOP";
opname[POP] = "POP";
opname[MKARR] = "MKARR";
opname[JMP] = "JMP";
opname[JMPN] = "JMPN";
opname[LDC] = "LDC";
opname[RET] = "RET";
opname[DUP] = "DUP";
opname[CALL] = "CALL";
opname[STR] = "STR";
opname[LOAD] = "LOAD";
opname[STORE] = "STORE";
opname[LDN] = "LDN";
opname[MGET] = "MGET";
opname[MSET] = "MSET";
opname[GETSUB] = "GETSUB";
opname[LOOPIF] = "LOOPIF";
opname[FUNC] = "FUNC";
opname[LSTORE] = "LSTORE";
opname[BREAK] = "BREAK";
opname[CNTN] = "CNTN";
opname[RESET] = "RESET";
opname[LRMV] = "LRMV";
opname[NOP] = "NOP";
opname[END] = "END";
}

int itop;
Register reg;

const int ARR_GROW = 1024 * 4;
const int MAX_ARR_CNT = 256;

Val heap[ARR_GROW * MAX_ARR_CNT];
bool used_loc[MAX_ARR_CNT];

void gc();

const double GC_START = 0.8;
const int MAX_LOCAL_CNT = 1024;
inline void free_used(int x) { used_loc[x] = false; }
inline void mark_used(int x) { used_loc[x] = true; }
inline int get_free() {
	int ret = -1;
	int sum = 0;
	for(int i = 1; i < MAX_ARR_CNT; i++) {
		if(!used_loc[i]) {
			if(ret == -1) ret = i;
		}
		else sum++;
	}
	if((double) sum / MAX_ARR_CNT >= GC_START) gc();
	if(ret == -1) error("out of memory");
	return ret;
}

Val get_sub(Val, Val);

Val get_heap(int loc) {
	if(loc < 0 || loc >= MAX_HEAP_SIZE) error("heap index out of bound.");
	if(loc < ARR_GROW) return Val((string)"" + (char)loc);
	return heap[loc];
}

void set_heap(int loc, Val v) {
	if(loc < ARR_GROW || loc >= MAX_HEAP_SIZE) error("set heap index out of bound.");
	heap[loc] = v;
}

void local_store(int pos, Val v) {
	reg.r[itop * MAX_LOCAL_CNT + pos] = v;
}

void store(int pos, Val v) {
	for(int i = itop - 1; i >= 0; i --) {
		Val v2 = reg.r[i * MAX_LOCAL_CNT + pos];
		if(v2.type != UNDEFINED_TYPE) {
			reg.r[i * MAX_LOCAL_CNT + pos] = v;
			return;
		}
	}
	reg.r[itop * MAX_LOCAL_CNT + pos] = v;
}

bool not_check = true;

Val load(int pos) {
	if(!not_check && pos > MAX_LOCAL_CNT) error("Checker: Loading " + tostr(pos) + " that doesn't initialized.");
	for(int i = itop; i >= 0; i--) {
		Val v = reg.r[i * MAX_LOCAL_CNT + pos];
		if(v.type != UNDEFINED_TYPE) return v;
	}
	error("Loading " + tostr(pos) + " that doesn't initialized.");
}

inline void release() {
	itop--;
}

inline void new_frame() {
	itop++;
}

bool visible[MAX_ARR_CNT];

void mark_visible(Val v) {
	if(v.type != REF_TYPE) return;
	else if(!visible[v.ref]) {
		visible[v.ref / ARR_GROW] = true;
		for(int i = 0; i < ARR_GROW; i++) {
			Val v2 = get_heap(get_sub(v, i).ref);
			if(v2.type != UNDEFINED_TYPE) mark_visible(v2);
		}
	}
}

void check_visible() {
	memset(visible, false, sizeof(visible));
	for(int i = itop; i >= 0; i--) {
		for(int pos = 0; pos < MAX_LOCAL_CNT; pos++) {
			Val v = reg.r[i * MAX_LOCAL_CNT + pos]; 
			if(v.type == REF_TYPE) {
				mark_visible(v);
			}
		}
	} 
}

void gc() {
	check_visible();
	for(int i = 0; i < MAX_ARR_CNT; i++) {
		if(!visible[i] && used_loc[i]) {
			free_used(i);
		}
	}
}

int hash_cnt;
map<string, int> hash_value;
int get_hash(string s) {
    if(hash_value.find(s) == hash_value.end()) hash_cnt++, hash_value[s] = hash_cnt;
    return hash_value[s] + ARR_GROW * 3;
}

Val clone(Val);
int get_len(Val);
Val make_array(vector<Val>);
Val get_sub(Val a, Val b) {
	if(b.type == STR_TYPE) b = get_hash(b.str);
	else if(b.type != NUM_TYPE && !(b.type == RNG_TYPE && a.type == STR_TYPE)) error("invalid subscript");
	if(a.type == STR_TYPE) {
		if(b.type == RNG_TYPE) {
			return a.str.substr(b.rng.from, b.rng.to - b.rng.from);
		}
		return (string)"" + a.str[(int)b.num];
	}
	else if(a.type == REF_TYPE) return ref_val(a.ref + b.num);
	else if(a.type == RNG_TYPE) return a.rng.from + b.num;
	else error("subscript type doesn't match");
}

Val make_array(vector<Val> arr) {
	int index = get_free();
	mark_used(index);
	int offset = index * ARR_GROW;
	for(int i = 0; i < arr.size(); i++) {
		set_heap(i + offset, arr[i]);
	}
	return ref_val(offset);
}

void check_arg_type(string func, Val * args, int pcnt) {
	;
}

int get_id(string);

unsigned char byte_map[MAX_FUNC_CNT][10000 + 5];
int func_cnt;
map<string, int> func_map;
int get_func_id(string func) {
	return get_id(func);
}
map<int, vector<int> > reg_map;
set<string> builtin;
inline bool is_builtin(string func) {
	return(builtin.find(func) != builtin.end());
}

Val clone(Val a) {
	if(a.type != REF_TYPE) return a;
	else {
		vector<Val> vals;
		for(int i = 0; i < ARR_GROW; i++) {
			vals.push_back(clone(get_heap(get_sub(a, i).ref)));
		}
		return make_array(vals);
	}
}

Val range_sub(Val a, Val b) {
	if(b.type == RNG_TYPE) {
		int from = b.rng.from;
		int to;
		if(b.rng.to == -1) to = get_len(a);
		else to = b.rng.to;
		if(a.type == STR_TYPE) {
			if(to > a.str.length() || from < 0) error("string index out of bound");
			return a.str.substr(from, to - from); 
		}
		else if(a.type == REF_TYPE) {
			if(to >= ARR_GROW || from < 0) error("array index out of bound");
			vector<Val> vals;
			for(int i = from; i < to; i++) vals.push_back(clone(get_heap(get_sub(a, i).ref)));
			Val ret = make_array(vals);
			return ret;
		}
		else error("invalid arguments");
	}
}

int get_len(Val p) {
	int len = 0;
	if(p.type == STR_TYPE) {
		len = p.str.length();
		return len;
	}
	else if(p.type == REF_TYPE) {
		for(int i = 0; i < ARR_GROW; i++) {
			if(heap[(int)p.ref + i].type == UNDEFINED_TYPE) break;
			len++;
		}
		return len;
	}
	else if(p.type == RNG_TYPE) {
		return p.rng.to - p.rng.from;
	}
	else error("invalid arguments");
}
Val call_builtin(string func, Val * params, int pcnt) {
	#define CNT(a) if(pcnt != a) error("wrong count of arguments")
	#define TYPE(a, t) if(pcnt <= a || params[a].type != t##_TYPE) error("invalid arguments")
	#define CHECK_MATH(x, m) \
	else if(func == (string)"math_" + x) {\
		TYPE(0, NUM);\
		return m(params[0].num);\
	}
	#define CHECK_FILE(x, m, t) \
	else if(func == (string)"file_" + x) {\
		TYPE(0, t);\
		return file_manager.m(params[0].num);\
	}
	if(func == "print") {
		for(int i = 0; i < pcnt; i++) cout << params[i].to_str() << ' ';
		cout << endl;
		return pcnt;
	}
	else if(func == "len") {
		CNT(1);
		return get_len(params[0]);
	}
	else if(func == "sub") {
		CNT(2);
		return range_sub(params[0], params[1]);
	}
	else if(func == "time") {
		return time(NULL);
	}
	else if(func == "clock") {
		return clock();
	}
	else if(func == "system") {
		CNT(1);
		TYPE(0, STR);
		return system(params[0].str.c_str());
	}
	CHECK_MATH("sin", sin)
	CHECK_MATH("cos", cos)
	CHECK_MATH("sin", tan)
	CHECK_MATH("asin", asin)
	CHECK_MATH("acos", acos)
	CHECK_MATH("atan", atan)
	CHECK_MATH("sqrt", sqrt)
	CHECK_MATH("floor", floor)
	CHECK_MATH("ceil", ceil)
	CHECK_MATH("log", log)
	CHECK_MATH("log10", log10)
	CHECK_MATH("exp", exp)
	CHECK_MATH("abs", fabs)
	else if(func == "file_open") {
		TYPE(0, STR);
		if(pcnt == 1) return file_manager.file_open(params[0].str);
		else if(pcnt == 2) {
			TYPE(1, STR);
			return file_manager.file_open(params[0].str, params[1].str);
		}
		else error("wrong count of arguments");
	}
	else if(func == "file_close") {
		CNT(1);
		TYPE(0, NUM);
		file_manager.file_close(params[0].num);
		return null_val();
	}
	else if(func == "file_read_string") {
		CNT(1);
		TYPE(0, NUM);
		return file_manager.fread_string(params[0].num);
	}
	else if(func == "file_read_number") {
		CNT(1);
		TYPE(0, NUM);
		return file_manager.fread_number(params[0].num);
	}
	else if(func == "file_read_line") {
		CNT(1);
		TYPE(0, NUM);
		return file_manager.fread_line(params[0].num);
	}
	else if(func == "read_string") {
		CNT(0);
		string s;
		cin >> s;
		return s;
	}
	else if(func == "read_number") {
		CNT(0);
		double x;
		cin >> x;
		return x;
	}
	else if(func == "read_line") {
		CNT(0);
		string s;
		getline(cin, s);
		return s;
	}
	else if(func == "file_write_string") {
		CNT(2);
		TYPE(0, NUM);
		TYPE(1, STR);
		file_manager.fwrite_string(params[0].num, params[1].str);
		return null_val();
	}
	else if(func == "file_write_number") {
		CNT(2);
		TYPE(0, NUM);
		TYPE(1, NUM);
		file_manager.fwrite_number(params[0].num, params[1].num);
		return null_val();
	}
	else if(func == "file_eof") {
		CNT(1);
		TYPE(0, NUM);
		return file_manager.file_eof(params[0].num);
	}
	else if(func == "str2ascii") {
		CNT(1);
		TYPE(0, STR);
		vector<Val> vals;
		for(int i = 0; i < params[0].str.length(); i++) vals.push_back(params[0].str[i]);
		return make_array(vals);
	}
	else if(func == "ascii2str") {
		if(pcnt <= 0) error("wrong count of arguments");
		if(params[0].type == REF_TYPE) {
			CNT(1);
			string str = "";
			int len = get_len(params[0]);
			for(int i = 0; i < len; i++) {
				Val v = get_heap(get_sub(params[0], i).ref);
				if(v.type == NUM_TYPE) str += (char)v.num;
				else str += "NaN";
			}
			return str;
		}
		else {
			string str = "";
			for(int i = 0; i < pcnt; i++) {
				Val v = params[i];
				if(v.type == NUM_TYPE) str += (char)v.num;
				else str += "NaN";
			}
			return str;
		}
	}
}
int x;
string random_name() {
	return "fvm_" + tostr(x++);
}

vector<string> all_func;
bool has_func(string func) {
	for(int i = 0; i < all_func.size(); i++) if(all_func[i] == func) return true;
	return false;
}
Val run_byte(string func, int stk_offset, Val * params, int pcnt) {
	reverse(params, params + pcnt);
	int stkoff = stk_offset;
	if(is_builtin(func)) return call_builtin(func, params, pcnt);
	if(!has_func(func)) error("function " + func + " not defined.");
	int index = get_id(func);
	RunStack rstack(stk_offset);
	Val op1, op2;
	int itop_back = itop;
	new_frame();
	#define MATH(x, a) \
		case x: {\
			op1 = rstack.pop(), op2 = rstack.pop();\
			rstack.push(op2 a op1);\
			break;\
		}
	#define IMATH(x, a) \
		case x: {\
			op1 = rstack.pop(), op2 = rstack.pop();\
			rstack.push(op2 a op1);\
			break;\
		}
	if(pcnt != reg_map[index].size()) error("wrong argument count.");
	for(int i = 0; i < pcnt; i++) {
		local_store(reg_map[index][i], params[i]);
	}
	for(int ip = 0; ; ip++) {
		unsigned char byte = byte_map[index][ip];
//		cout << ip << " " << (int)byte << " " << opname[byte] << endl;
		switch(byte) {
			MATH(ADD, +)
			MATH(SUB, -)
			MATH(MUL, *)
			MATH(DIV, /)
			MATH(LAND, &&)
			MATH(LOR, ||)
			MATH(BIG, >)
			MATH(BIGE, >=)
			MATH(SML, <)
			MATH(SMLE, <=)
			MATH(EQL, ==)
			MATH(NEQL, !=)
			case NOT: {
				op1 = rstack.pop();
				rstack.push(!op1);
			    break;
			}
			case BNOT: {
				op1 = rstack.pop();
				rstack.push(~op1);
			    break;
			}
			case NEG: {
				op1 = rstack.pop();
				rstack.push(0 - op1);
			    break;
			}
			IMATH(BAND, &)
			IMATH(BOR, |)
			IMATH(XOR, ^)
			IMATH(LSHF, <<)
			IMATH(RSHF, >>)
			IMATH(MOD, %)
			case LOOP: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int loop = l_l + l_h * 256;
				int newip = ip - loop;
				ip = newip;
			    break;
			}
			case LOOPIF: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int loop = l_l + l_h * 256;
				int newip = ip - loop;
				if((!rstack.pop()).num) {
					ip = newip;
				}
			    break;
			}
			case POP: {
				rstack.pop();
			    break;
			}
			case GETSUB: {
				rstack.push(get_sub(rstack.pop(), rstack.pop()));
			    break;
			}
			case MKARR: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int len = l_l + l_h * 256;
				vector<Val> arr;
				for(int i = 0; i < len; i++) arr.push_back(rstack.pop());
				reverse(arr.begin(), arr.end());
				rstack.push(make_array(arr));
				break;
			    break;
			}
			case JMP: case BREAK: case CNTN: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int loop = l_l + l_h * 256;
				int newip = ip + loop;
				ip = newip;
			    break;
			}
			case JMPN: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int loop = l_l + l_h * 256;
				int newip = ip + loop;
				if((!rstack.pop()).num) {
					ip = newip;
				}
			    break;
			}
			case LDC: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int ldc = l_l + l_h * 256;
				rstack.push(cpool[ldc]);
			    break;
			}
			case LDN: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int ldc = l_l + l_h * 256;
				rstack.push(ldc);
			    break;
			}
			case RET: {
				release();
				itop = itop_back;
				return rstack.top();
			    break;
			}
			case DUP: {
				rstack.push(rstack.top());
			    break;
			}
			case CALL: {
				int cnt = rstack.pop().num;
				Val * par = new Val[cnt];
				for(int i = 0; i < cnt; i++) par[i] = rstack.pop();
				if(rstack.top().type != FUNC_TYPE) error("Can not call a non-function value.");
				string addr = rstack.pop().func;
				Val ret = run_byte(addr, rstack.t, par, cnt);
				rstack.push(ret);
			    break;
			}
			case LDC0: case LDC1: case LDC2: case LDC3: 
			case LDC4: case LDC5: case LDC6: case LDC7: {
				rstack.push(cpool[byte - LDC0]);
				break;
			}
			case END: {
				return Val("End");
			    break;
			}
			case LOAD: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int ldc = l_l + l_h * 256;
				rstack.push(load(ldc));
			    break;
			}
			case STORE: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int ldc = l_l + l_h * 256;
				store(ldc, rstack.pop());
			    break;
			}
			case LSTORE: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int ldc = l_l + l_h * 256;
				local_store(ldc, rstack.pop());
			    break;
			}
			case MGET: {
				Val v = rstack.pop();
				if(v.type == STR_TYPE) rstack.push(v.str);
				else if(v.type == NUM_TYPE) rstack.push(v.num);
				else rstack.push(get_heap(v.ref));
			    break;
			}
			case MSET: {
				Val v2 = rstack.pop();
				if(v2.type != REF_TYPE) error("wrong type of subscript setting");
				int addr = v2.ref;
				Val v = rstack.pop();
				set_heap(addr, v);
			    break;
			}
			case STR: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int len = l_l + l_h * 256;
				string s = "";
				for(int i = 0; i < len; i++) s += (char)rstack.pop().num;
				reverse(s.begin(), s.end());
				rstack.push(s);
				break;
			}
			case FUNC: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int len = l_l + l_h * 256;
				string s = "";
				for(int i = 0; i < len; i++) s += (char)rstack.pop().num;
				reverse(s.begin(), s.end());
				rstack.push(func_val(s));
				break;
			}
			case LRMV: {
				int l_h = byte_map[index][++ip];
				int l_l = byte_map[index][++ip];
				int loc = l_l + l_h * 256;
				store(loc, Val());
				break;
			}
			case NOP: {
				;
				break;
			}
			case RESET: {
				rstack.t = stkoff;
				break;
			}
			case LD0: case LD1: case LD2: case LD3:
			case LD4: case LD5: case LD6: case LD7: 
			case LD8: case LD9: case LD10: case LD11:
			case LD12: case LD13: case LD14: case LD15: {
				rstack.push(byte - LD0);
				break;
			}
			case LDNUL: {
				rstack.push(null_val());
				break;
			}
			case MKRG: {
				Val l = rstack.pop();
				Val r = rstack.pop();
				if(l.type == NUM_TYPE && r.type == NUM_TYPE) {
					rstack.push(make_range(l.num, r.num));
				}
				else error("range left/right value must be number");
				break;
			}
			default: {
				error("uncompatible byte " + tostr(byte));
				break;
			}
		}
	}
}

const string opchar = "+-*/!=><%&|@^~:";
const string spliter = "+-*/=><;!&|%@^,~#:";
string trim(string s) {
    int x = 0, y = s.length() - 1;
    while(isspace(s[x]) || s[x] == 0) x++;
    while(isspace(s[y]) || s[y] == 0) y--;
    return s.substr(x, y - x + 1);
}
#define MIDDLE(str) (str.length() >= 2 ? str.substr(1, str.length() - 2) : str)

bool include_char(string s, char c) {
    for(int i = 0; i < s.length(); i++) if(s[i] == c) return true;
    return false;
}
string join_string(vector<string> vs, string spaces = "") {
    string r = "";
    for(int i = 0; i < vs.size(); i++) r += vs[i] + spaces;
    return r;
}
vector<string> remove_all(vector<string> vs, string remv) {
    vector<string> result;
    for(int i = 0; i < vs.size(); i++) {
        if(remv != vs[i]) result.push_back(vs[i]);
    }
    return result;
}
vector<string> tokenize(string text) {
	vector<string> tmp;
	string cur = "";
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
				else if(text[i] == '"' || text[i] == '\'') {
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
				else if(text[i] == '"' || text[i] == '\'') {
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
				else if(text[i] == '"' || text[i] == '\'') {
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
		else if(text[i] == '\'') {
			int top = 1;
			if(cur != "") {
				tmp.push_back(cur);
			}
			cur = "\'";
			while(i < text.length()) {
				i++;
				if(text[i] == '\\') {
					cur = cur + text[i];
					if(i < text.length() - 1) {
						i++;
						cur = cur + text[i];
					}
				}
				else if(text[i] == '\'') {
					cur = cur + text[i];
					break;
				}
				else cur = cur + text[i];
			}
			tmp.push_back(cur);
			cur = "";
		}
		else if(!iscsym(text[i])) {
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
		else if(iscsym(text[i])) {
			cur += text[i];
		}
	}
	if(cur != "") {
		tmp.push_back(cur);
	}
	vector<string> ret;
	vector<string> final;
	for(int i = 0; i < tmp.size(); i++) {
		if(tmp[i] != "") {
			ret.push_back(trim(tmp[i]));
		}
	}
    for(int i = 0; i < ret.size(); i++) {
        if(i > 0) {
            if(ret[i] == "=") {
                if(ret[i - 1] == ">" || ret[i - 1] == "<" || ret[i - 1] == "!" || ret[i - 1] == "=" ||
				ret[i - 1] == "+" || ret[i - 1] == "-" || ret[i - 1] == "*" || ret[i - 1] == "/" ||
				ret[i - 1] == "%" || ret[i - 1] == ">>" || ret[i - 1] == "<<" || ret[i - 1] == "&" ||
				ret[i - 1] == "|") {
					final[final.size() - 1] += "=";
				}
                else final.push_back(ret[i]);
            } else if(ret[i] == ".") {
            	if(i >= ret.size() - 1) error("need numbers or an identifier after '.' .");
            	if(isdigit(ret[i - 1][0]) || ret[i - 1][0] == '-') {
            		final[final.size() - 1] += ret[i] + ret[i + 1];
            		i++;
				}
				else final.push_back(ret[i]);
			} else if(ret[i] == "&") {
                if(ret[i - 1] == "&") final[final.size() - 1] += "&";
                else final.push_back(ret[i]);
            } else if(ret[i] == "|") {
                if(ret[i - 1] == "|") final[final.size() - 1] += "|";
                else final.push_back(ret[i]);
            } else if(ret[i] == ">") {
                if(ret[i - 1] == ">") final[final.size() - 1] += ">";
                else final.push_back(ret[i]);
            } else if(ret[i] == "<") {
                if(ret[i - 1] == "<") final[final.size() - 1] += "<";
                else final.push_back(ret[i]);
            } else if(ret[i] == "+") {
                if(ret[i - 1] == "+") final[final.size() - 1] += "+";
                else final.push_back(ret[i]);
            } else if(ret[i] == "-") {
                if(ret[i - 1] == "-") final[final.size() - 1] += "-";
                else final.push_back(ret[i]);
            }else final.push_back(ret[i]);
        } else final.push_back(ret[i]);
    }
	return final;
}

vector<string> split(string text, char c, bool space = true) {
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

string standard_call(string str) {
    vector<string> vs = tokenize(str);
    vector<string> result;
    string ret = "";
    for(int i = 0; i < vs.size(); i++) {
        if(i > 0 && !(include_char(opchar + "[{}]", vs[i - 1][0])) && vs[i][0] == '(') {
            string allParam = MIDDLE(vs[i]);
            result.push_back("^");
            result.push_back("[" + allParam + "]");
        } else result.push_back(vs[i]);
    }

    ret = join_string(result);
    return ret;
}
string standard_subscript(string str) {
    vector<string> vs = tokenize(str);
    vector<string> result;
    string ret = "";
    for(int i = 0; i < vs.size(); i++) {
        if(vs[i][0] == '[') {
            string subExpr = MIDDLE(vs[i]);
            result.push_back("@");
            result.push_back("(" + subExpr + ")");
        } else result.push_back(vs[i]);
    }

    ret = join_string(result);
    return ret;
}
string string_expression(string str) {
    string result = "";
    for(int i = 0; i < str.length(); i++) {
        if(str[i] == '\\') {
            i++;
            if(i > str.length()) error(" need an escape character after '\\'.");
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
                    string msg = " unsupported escape character: '\\";
                    msg += str[i];
                    msg += "'.";
                    error(msg);
                    break;
                }
            }
        } else result += str[i];
    }
    return result;
}
vector<unsigned char> load_int(double x) {
	vector<unsigned char> rets;
	if(ceil(x) == x && x >= 0 && x <= 15) {
		rets.push_back(LD0 + x);
	}
	else if(ceil(x) != x || x < 0 || x >= 256 * 256) {
		add_const(x);
		int siz = cpool_pos - 1;
		if(siz >= 0 && siz <= 7) rets.push_back(LDC0 + siz);
		else {
			rets.push_back(LDC);
			rets.push_back(siz / 256);
			rets.push_back(siz % 256);
		}
	}
	else {
		int ax = (int)x;
		rets.push_back(LDN);
		rets.push_back(ax / 256);
		rets.push_back(ax % 256);
	}
	return rets;
}
double to_hex(string s) {
	int result = 0;
	int parsing[256];
	for(int i = 0; i <= 9; i++) parsing['0' + i] = i;
	for(int i = 'a'; i <= 'f'; i++) parsing[i] = i - 'a' + 10;
	for(int i = 2; i < s.length(); i++) result = result * 16 + parsing[tolower(s[i])];
	return result;
}
map<string, int> var_id;
int varcnt;
int get_id(string var) {
	if(var_id.find(var) == var_id.end()) var_id[var] = varcnt, varcnt++;
	return var_id[var];
}
vector<unsigned char> get_var_val(string name) {
	vector<unsigned char> rets;
	rets.push_back(LOAD);
	int id = get_id(name); 
	rets.push_back(id / 256);
	rets.push_back(id % 256);
	return rets;
}
vector<unsigned char> load_func(string s) {
	vector<unsigned char> rets;
	string str = string_expression(s);
    for(int i = 0; i < str.length(); i++) {
    	rets.push_back(LDN);
    	rets.push_back(0);
    	rets.push_back(str[i]);
	}
	int siz = str.length();
	rets.push_back(FUNC);
	rets.push_back(siz / 256);
	rets.push_back(siz % 256);
	return rets;
}
vector<unsigned char> compile_literal(string s) {
	vector<unsigned char> rets;
    if(s[0] == '\"'|| s[0] == '\'') {
    	string str = string_expression(MIDDLE(s));
    	for(int i = 0; i < str.length(); i++) {
    		rets.push_back(LDN);
    		rets.push_back(0);
    		rets.push_back(str[i]);
		}
		int siz = str.length();
		rets.push_back(STR);
		rets.push_back(siz / 256);
		rets.push_back(siz % 256);
	}
    else if(isdigit(s[0])) {
    	if(s[0] == '0' && s[1] == 'x') {
    		vector<unsigned char> rets2 = load_int(to_hex(s));
    		for(int i = 0; i < rets2.size(); i++) rets.push_back(rets2[i]);
		}
    	else {
    		// if(s.length() >= 13) return Value::makeBigInteger(s.c_str());
	    	double d = atof(s.c_str());
	    	/*
	    	if(d > MAX_NUMBER) {
	    		return Value::makeBigInteger(parseNum(d).c_str());
			}
			else
			*/
    		vector<unsigned char> rets2 = load_int(d);
    		for(int i = 0; i < rets2.size(); i++) rets.push_back(rets2[i]);
		}
	}
    else return get_var_val(s);
    return rets;
}

bool has_range(vector<string> vs) {
	if(vs.size() >= 4) return false;
	if(vs.size() == 3) return vs[1] == ":";
	if(vs.size() == 2) return vs[0] == ":" || vs[1] == ":";
	if(vs.size() == 1) return vs[0] == ":";
	return false;
}

void get_range_param(vector<string> vs, string & left, string & right) {
	if(vs.size() == 3) left = vs[0], right = vs[2];
	if(vs.size() == 2) {
		if(vs[0] == ":") left = "0", right = vs[1];
		else left = vs[0], right = "-1";
	}
	if(vs.size() == 1) left = "0", right = "-1";
}

string parse_subscript(string);
vector<unsigned char> execute(string);
bool is_self(string);
vector<unsigned char> make_lambda(string, string);
vector<unsigned char> compile_expression(string expr) {
  	vector<string> vs;
    stack<string> stk;
    vector<string> str;
	string backup = expr;
	vector<unsigned char> bytecodes;
	
    expr = standard_call(standard_subscript(parse_subscript(expr)));

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
    if(str[0] == "lambda") {
    	if(str.size() != 3) error("lambda syntax must be '(lambda {<params>} {<expr})'");
    	return make_lambda(str[2], str[1]);
	}
	else if(str.size() == 2 && (str[1] == "++" || str[1] == "--")) {
		vector<unsigned char> ret = compile_expression(str[0]);
		vector<unsigned char> exret = execute(backup);
		for(int i = 0; i < exret.size(); i++) ret.push_back(exret[i]);
		return ret;
	}
	else if(has_range(str)) {
		string left, right;
		get_range_param(str, left, right);
		vector<unsigned char> ret = compile_expression(left);
		vector<unsigned char> ret2 = compile_expression(right);
		for(int i = 0; i < ret2.size(); i++) ret.push_back(ret2[i]);
		ret.push_back(MKRG);
		return ret;
	}
	else if(str.size() == 2 && (str[0] == "++" || str[0] == "--")) {
		return execute(str[1] + str[0]);
	}
	else  {
		for(int i = 0; i < str.size(); i++) {
	    	if((str[i].length() == 1 && str[i] == "=") || (str[i].length() <= 3 && (str[i][str[i].length() - 1] == '=' && is_self(str[i].substr(0, str[i].length() - 1))))) {
				return execute(backup);
	    		break;
			}
		}
	}
	
    for(int i = 0; i < str.size(); i++) {
        string tmp = "";
        string a = "";
        if(include_char(opchar, str[i][0])) {
            string c = str[i];
            if(stk.empty() || stk.top() == "(") {
                stk.push(c);
            } else {
                while(!stk.empty() && bind_power[stk.top()] >= bind_power[c] ) {
                    tmp += stk.top();
                    vs.push_back(tmp);
                    stk.pop();
                    tmp = "";
                }
                stk.push(c);
            }
        } else vs.push_back(str[i]);
    }
    while(!stk.empty()) {
        string tmp = "";
        tmp += stk.top();
        vs.push_back(tmp);
        stk.pop();
    }
	
	#define PUSH(x) bytecodes.push_back(x)
	#define CHECK_PUSH(op, push) \
	else if(vs[i] == op) {\
		PUSH(push);\
	}
    for(int i = 0; i < vs.size(); i++) {
        string tmp = vs[i];
        if(vs[i] == "+") {
        	PUSH(ADD);
        }
		CHECK_PUSH("-", SUB)
		CHECK_PUSH("*", MUL)
		CHECK_PUSH("/", DIV)
		CHECK_PUSH("%", MOD)
		CHECK_PUSH(">", BIG)
		CHECK_PUSH("<", SML)
		CHECK_PUSH(">=", BIGE)
		CHECK_PUSH("<=", SMLE)
		CHECK_PUSH("==", EQL)
		CHECK_PUSH("!=", NEQL)
		CHECK_PUSH("!", NOT)
		CHECK_PUSH("&&", LAND)
		CHECK_PUSH("||", LOR)
		CHECK_PUSH(">>", RSHF)
		CHECK_PUSH("<<", LSHF)
		CHECK_PUSH("&", BAND)
		CHECK_PUSH("|", BOR)
		CHECK_PUSH("~", BNOT)
		else if(vs[i] == "@") {
            PUSH(GETSUB);
            PUSH(MGET);
        } else if(vs[i] == "^") {
            PUSH(CALL);
        } else {
            string simpleExpr = vs[i];
            if(vs[i][0] == '(') {
                simpleExpr = MIDDLE(vs[i]);
            	vector<unsigned char> rets = compile_expression(simpleExpr);
            	for(int i = 0; i < rets.size(); i++) PUSH(rets[i]);
            } else if(vs[i][0] == '[') {
                simpleExpr = MIDDLE(vs[i]);
                string parameter = simpleExpr;
                vector<string> args = remove_all(split(parameter, ','), ",");
	            for(int i = 0; i < args.size(); i++) {
	            	vector<unsigned char> rets = compile_expression(args[i]);
	            	for(int j = 0; j < rets.size(); j++) PUSH(rets[j]);
				}
				int siz = args.size();
				vector<unsigned char> rets2 = load_int(siz);
				for(int i = 0; i < rets2.size(); i ++) PUSH(rets2[i]);
            } else if(vs[i][0] == '{') {
                simpleExpr = MIDDLE(vs[i]);
                vector<string> args = remove_all(split(simpleExpr, ','), ",");
                for(int i = 0; i < args.size(); i++) {
                	vector<unsigned char> rets = compile_expression(args[i]);
	            	for(int j = 0; j < rets.size(); j++) PUSH(rets[j]);
				}
				int siz = args.size();
                PUSH(MKARR);
                PUSH(siz / 256);
                PUSH(siz % 256);
            } else {
            	vector<unsigned char> rets = compile_literal(vs[i]);
            	for(int i = 0; i < rets.size(); i++) PUSH(rets[i]);
			}
        }
    }
    return bytecodes;
}
enum { EXPR, ASSIGN };
string parse_subscript(string expr) {
	vector<string> vs = tokenize(expr);
	string result = "";
	for(int i = 0; i < vs.size(); i++) {
		if(i < vs.size() - 1 && vs[i] == ".") {
			i++;
			result += "[\"" + vs[i] + "\"]";
		}
		else result += vs[i];
	}
	return result;
}
vector<unsigned char> set_var_val(string name, string exp) {
	vector<unsigned char> vc;
	vector<unsigned char> rets = compile_expression(exp);
	for(int i = 0; i < rets.size(); i++) vc.push_back(rets[i]); 
	vc.push_back(STORE);
	int id = get_id(name);
	vc.push_back(id / 256);
	vc.push_back(id % 256); 
	return vc;
}
enum { SUB_SET, VAR_SET };
vector<unsigned char> set_var_ref(string var, string exp) {
	vector<unsigned char> bytecodes;
	vector<unsigned char> rets2;
	/*
    for(int i = 0; i < protectVar.size(); i++) {
    	if(var == protectVar[i]) RUN_ERR(" access denied: " + var + " can not be modified.", 16);
	}
	*/
	if(include_char(var, '.')) var = parse_subscript(var);
    vector<string> vs;
    stack<string> stk;
    vector<string> str;
    string expr = var;
    int type = VAR_SET;

    for(int i = 0; i < var.length(); i++) {
        if(var[i] == '[') type = SUB_SET;
    }

    if(type == VAR_SET) {
        return set_var_val(var, exp);
    }
	
	rets2 = compile_expression(exp);
	for(int i = 0; i < rets2.size(); i++) PUSH(rets2[i]);
	
    expr = standard_call(standard_subscript(expr));

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
        if(include_char(opchar, str[i][0])) do {
                string c = str[i];
                if(stk.empty() || stk.top() == "(") {
                    stk.push(c);
                } else {
                    while(!stk.empty() && bind_power[stk.top()] >= bind_power[c] ) {
                        tmp += stk.top();
                        vs.push_back(tmp);
                        stk.pop();
                        tmp = "";
                    }
                    stk.push(c);
                }
            } while(0);
        else vs.push_back(str[i]);
    }

    while(!stk.empty()) {
        string tmp = "";
        tmp += stk.top();
        vs.push_back(tmp);
        stk.pop();
    }

    for(int i = 0; i < vs.size() - 1; i++) {
        if(vs[i] == "@") {
            PUSH(GETSUB);
            PUSH(MGET);
        } else {
            string simpleExpr = vs[i];
            if(vs[i][0] == '(') {
                simpleExpr = MIDDLE(vs[i]);
            	vector<unsigned char> rets = compile_expression(simpleExpr);
            	for(int i = 0; i < rets.size(); i++) PUSH(rets[i]);
            } else if(vs[i][0] == '[') {
                simpleExpr = MIDDLE(vs[i]);
                string parameter = simpleExpr;
                vector<string> args = remove_all(split(parameter, ','), ",");
	            for(int i = 0; i < args.size(); i++) {
	            	vector<unsigned char> rets = compile_expression(args[i]);
	            	for(int j = 0; j < rets.size(); j++) PUSH(rets[j]);
				}
				int siz = args.size();
				PUSH(LDN);
				PUSH(siz / 256);
				PUSH(siz % 256);
            } else if(vs[i][0] == '{') {
                simpleExpr = MIDDLE(vs[i]);
                vector<string> args = remove_all(split(simpleExpr, ','), ",");
                for(int i = 0; i < args.size(); i++) {
                	vector<unsigned char> rets = compile_expression(args[i]);
	            	for(int j = 0; j < rets.size(); j++) PUSH(rets[j]);
				}
				int siz = args.size();
                PUSH(MKARR);
                PUSH(siz / 256);
                PUSH(siz % 256);
            } else {
            	vector<unsigned char> rets = compile_literal(vs[i]);
            	for(int i = 0; i < rets.size(); i++) PUSH(rets[i]);
			}
        }
    }

    PUSH(GETSUB);
    PUSH(MSET);
    
    return bytecodes;
}
const int SELF_CNT = 9;
const string SELF_OPS[SELF_CNT] = {
	"+", "-", "*", "/", "%", ">>", "<<", "&", "|"
};

bool is_self(string op) {
	for(int i = 0; i < SELF_CNT; i++) if(SELF_OPS[i] == op) return true;
	return false; 
} 
vector<unsigned char> execute(string stat) {
    vector<string> vs = tokenize(stat);
    int i = 0;
    int type = EXPR;
    string flag = vs[0];

    bool inq = false;
	int x;
	if(vs.size() == 2 && (vs[1] == "++" || vs[1] == "--")) {
		string newstat = vs[0] + " = (" + vs[0] + ") ";
		if(vs[1] == "++") {
			newstat += "+";
		}
		else if(vs[1] == "--") {
			newstat += "-";
		}
		newstat += "1";
		return execute(newstat);
	}
	else {
		for(int i = 0; i < vs.size(); i++) {
	    	if((vs[i].length() == 1 && vs[i] == "=") || (vs[i].length() <= 3 && (vs[i][vs[i].length() - 1] == '=' && is_self(vs[i].substr(0, vs[i].length() - 1))))) {
				type = ASSIGN, x = i;
	    		break;
			}
		}
	}
    string left;
    string right;
    vector<unsigned char> ret;
    if(type == ASSIGN) {
    	for(int i = 0; i < x; i++) left += vs[i];
   		for(int j = x + 1; j < vs.size(); j++) right += vs[j];
        
        string op = "X";
		if(vs[x] != "=") op = vs[x].substr(0, vs[x].length() - 1);
		
		if(op != "X") right = "(" + left + ")" + op + "(" + right + ")";
		ret = set_var_ref(left, right);
    } else if(flag == "stop") {
    } else {
        ret = compile_expression(stat);
    }
    return ret;
}
void init2() {
	new_frame();
#define BIND(c, rbp, code) bind_power[(c)] = (rbp)
    BIND("+", 60, ADD);
    BIND("-", 60, DEC);
    BIND("*", 70, MUL);
    BIND("/", 70, DIV);
    BIND("%", 70, MOD);
    BIND("+=", 10, ADD_E);
    BIND("-=", 10, DEC_E);
    BIND("*=", 10, MUL_E);
    BIND("/=", 10, DIV_E);
    BIND("%=", 10, MOD_E);
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
    BIND(">>", 65, RIGHTSHIFT);
    BIND("<<", 65, LEFTSHIFT);
    BIND("&=", 10, BITAND_E);
    BIND("|=", 10, BITOR_E);
    BIND(">>=", 10, RIGHTSHIFT_E);
    BIND("<<=", 10, LEFTSHIFT_E);
    BIND("@", 90, SUB);
    BIND("^", 90, CALL);
    BIND("~", 90, BITNOT);
    BIND(":", 15, MKRANGE);
    
    #define BUILTIN(a) store(get_id(a), func_val(a)), builtin.insert(a)
    
    BUILTIN("print");
    BUILTIN("len");
    BUILTIN("sub");
    BUILTIN("clock");
    BUILTIN("time");
    BUILTIN("system");
    
    BUILTIN("math_sin");
    BUILTIN("math_cos");
    BUILTIN("math_tan");
    BUILTIN("math_asin");
    BUILTIN("math_acos");
    BUILTIN("math_atan");
    BUILTIN("math_floor");
    BUILTIN("math_ceil");
    BUILTIN("math_sqrt");
    BUILTIN("math_exp");
    BUILTIN("math_log");
    BUILTIN("math_log10");
    BUILTIN("math_abs");
    
    
    BUILTIN("file_open");
    BUILTIN("file_close");
    BUILTIN("file_read_number");
    BUILTIN("file_read_string");
    BUILTIN("file_read_line");
    BUILTIN("file_write_string");
    BUILTIN("file_write_number");
    BUILTIN("file_eof");
    
    BUILTIN("read_number");
    BUILTIN("read_string");
    BUILTIN("read_line");
    
    BUILTIN("str2ascii");
    BUILTIN("ascii2str");
}

vector<string> tokenstream;
int tok;
string next_token() {
	if(tok == tokenstream.size()) return "#END_TOKEN#";
	return tokenstream[tok++];
}

bool has_next() {
	return tok < tokenstream.size();
}

void read_token(string expect) {
	if(tok == tokenstream.size()) error("expected " + expect + ".");
	if(tokenstream[tok++] != expect) error("expected " + expect + ".");
}

inline bool next_token_is(string next) {
	if(!has_next()) return false;
	return tokenstream[tok] == next;
}

void add_token(string token) {
	tokenstream.push_back(token);
}
const string BEGIN_LABEL = "{";
const string END_LABEL = "}";
const string EOL_LABEL = "<EOL>"; 
vector<unsigned char> compile_block();

int loop_stack[10000 + 5];
int top;

int pop_loop() {
	return loop_stack[top--];
}

void add_loop(int addr) {
	loop_stack[++top] = addr;
}

void replace_holder(int loop_break, int loop_cntn, int loop_head, vector<unsigned char> & rets) {
	for(int i = loop_break - 1; i >= loop_head; i--) {
		if(rets[i] == BREAK) {
			if(rets[i + 1] == 0x00 && rets[i + 2] == 0x00) { // Not Replaced Break
				int jmp = loop_break - (i + 3);
				rets[i + 1] = jmp / 256; // Replace Break
				rets[i + 2] = jmp % 256;
			}
		}
		if(rets[i] == CNTN) {
			if(rets[i + 1] == 0x00 && rets[i + 2] == 0x00) { // Not Replaced Continue
				int lop = loop_cntn - (i + 3);
				cout << "Loop back " << lop << endl;
				rets[i + 1] = lop / 256; // Replace Continue
				rets[i + 2] = lop % 256;
			}
		}
	}
}
int tabcnt = 0;
string gettab() {
	string s = "";
	for(int i = 0; i < tabcnt; i++) s += "    ";
	return s;
}
vector<unsigned char> compile_statement() {
	vector<unsigned char> rets;
	string flag = next_token();
	tabcnt++;
	if(flag == BEGIN_LABEL) {
		vector<unsigned char> rets2 = compile_block();
		for(int i = 0; i < rets2.size(); i++) rets.push_back(rets2[i]);
		read_token(END_LABEL);
	}
	else if(flag == "if") {
		string expr = next_token();
		vector<unsigned char> rets2 = compile_expression(expr);
		for(int i = 0; i < rets2.size(); i++) rets.push_back(rets2[i]);
		vector<unsigned char> if_branch;
	//	if_branch.push_back(FRM);
		vector<unsigned char> rets3 = compile_statement();
		for(int i = 0; i < rets3.size(); i++) if_branch.push_back(rets3[i]);
	//	if_branch.push_back(DFRM);
		if_branch.push_back(JMP);
		if_branch.push_back(0x00); // Place Holder
		if_branch.push_back(0x00);
		int jump_loc = if_branch.size();
		rets.push_back(JMPN);
		rets.push_back(jump_loc / 256);
		rets.push_back(jump_loc % 256);
		vector<unsigned char> else_branch;
		if(next_token_is("else")) {
			read_token("else");
			vector<unsigned char> rets4 = compile_statement();
			for(int i = 0; i < rets4.size(); i++) else_branch.push_back(rets4[i]);
			int jumploc2 = else_branch.size();
			if_branch[if_branch.size() - 2] = jumploc2 / 256; // Replace
			if_branch[if_branch.size() - 1] = jumploc2 % 256;			
		}
		for(int i = 0; i < if_branch.size(); i++) rets.push_back(if_branch[i]);
		for(int i = 0; i < else_branch.size(); i++) rets.push_back(else_branch[i]);
	}
	else if(flag == "function") {
		string func_name = next_token();
		all_func.push_back(func_name);
		string param_list = next_token();
		param_list = MIDDLE(param_list);
		vector<string> args = remove_all(split(param_list, ','), ",");
		int func_id = get_id(func_name);
		vector<unsigned char> func = compile_statement();
		vector<int> regs;
		func.push_back(LDN);
		func.push_back(0);
		func.push_back(0);
		func.push_back(RET); 
		for(int i = 0; i < func.size(); i++) {
			byte_map[func_id][i] = func[i];
		}
		byte_map[func_id][func.size()] = END;
		for(int i = 0; i < args.size(); i++) regs.push_back(get_id(args[i]));
		reg_map[func_id] = regs;
		int id = func_id;
		// Set func_name = func_id
		vector<unsigned char> lint = load_func(func_name);
		for(int i = 0; i < lint.size(); i++) rets.push_back(lint[i]);
		rets.push_back(STORE);
		rets.push_back(id / 256);
		rets.push_back(id % 256);
	}
	else if(flag == "return") {
		string str = "";
		string cur = next_token();
		while(cur != EOL_LABEL) str += cur, cur = next_token();
		vector<unsigned char> rets2 = compile_expression(str);
		rets2.push_back(RET);
		for(int i = 0; i < rets2.size(); i++) rets.push_back(rets2[i]);
	}
	else if(flag == "while") {
		string expr = next_token();
		int loop_head = rets.size();
		vector<unsigned char> rets2 = compile_expression(expr);
		rets2.push_back(JMPN);
		rets2.push_back(0x00); // Place Holder
		rets2.push_back(0x00);
		vector<unsigned char> loop_body;
		vector<unsigned char> rets3 = compile_statement();
		for(int i = 0; i < rets3.size(); i++) loop_body.push_back(rets3[i]);
		
		vector<unsigned char> suf_work; 
		suf_work.push_back(RESET);
		suf_work.push_back(LOOP);
		suf_work.push_back(0x00);
		suf_work.push_back(0x00); // Place Holder
		
		int jump_loc = loop_body.size() + suf_work.size();
		int loop_loc = loop_body.size() + suf_work.size() + rets2.size();
		suf_work[suf_work.size() - 2] = loop_loc / 256; // Replace Loop
		suf_work[suf_work.size() - 1] = loop_loc % 256;
		rets2[rets2.size() - 2] = jump_loc / 256; // Replace Checker
		rets2[rets2.size() - 1] = jump_loc % 256;
		
		for(int i = 0; i < rets2.size(); i++) rets.push_back(rets2[i]);
		for(int i = 0; i < loop_body.size(); i++) rets.push_back(loop_body[i]);
		int loop_cntn = rets.size();
		for(int i = 0; i < suf_work.size(); i++) rets.push_back(suf_work[i]);
		int loop_break = rets.size();
		
		replace_holder(loop_break, loop_cntn, loop_head, rets);
	}
	else if(flag == "local") {
		string t = "";
		while((t = next_token()) != EOL_LABEL) {
			int id = get_id(t);
			if(next_token_is("=")) {
				next_token();
				string str = "";
				while(!next_token_is(",") && !next_token_is(EOL_LABEL)) str += next_token();
				vector<unsigned char> rets2 = compile_expression(str);
				for(int i = 0; i < rets2.size(); i++) rets.push_back(rets2[i]);
			}
			else {
				rets.push_back(LDN);
				rets.push_back(0);
				rets.push_back(0);
			}
			rets.push_back(LSTORE);
			rets.push_back(id / 256);
			rets.push_back(id % 256);
			if(!next_token_is(EOL_LABEL)) read_token(",");
		}
	}
	else if(flag == "for") {
		string cond = next_token();
		cond = MIDDLE(cond);
		vector<string> vars = remove_all(split(cond, ','), ",");
		string var = vars[0];
		string step = vars[3];
		string first = "(" + vars[1] + ")";
		string expr = vars[2];
		int var_id = get_id(var);
		vector<unsigned char> fret = compile_expression(first);
		for(int i = 0; i < fret.size(); i++) rets.push_back(fret[i]);
		rets.push_back(LSTORE);
		rets.push_back(var_id / 256);
		rets.push_back(var_id % 256);
		
		int loop_head = rets.size();
		vector<unsigned char> rets2 = compile_expression(expr); // Condition
		rets2.push_back(JMPN);
		rets2.push_back(0x00); // Place Holder
		rets2.push_back(0x00);
		
		vector<unsigned char> loop_body;
		vector<unsigned char> part = compile_statement();
		
		for(int i = 0; i < part.size(); i++) loop_body.push_back(part[i]); // Loop body
		
		vector<unsigned char> loop_body2;
		
		vector<unsigned char> stepr = compile_expression(step);
		for(int i = 0; i < stepr.size(); i++) loop_body2.push_back(stepr[i]); // Step Modifying
		loop_body2.push_back(LOAD);
		loop_body2.push_back(var_id / 256);
		loop_body2.push_back(var_id % 256);
		loop_body2.push_back(ADD); 
		loop_body2.push_back(LSTORE);
		loop_body2.push_back(var_id / 256);
		loop_body2.push_back(var_id % 256);
		
		loop_body2.push_back(RESET); // End of loop
		loop_body2.push_back(LOOP);
		loop_body2.push_back(0x00);
		loop_body2.push_back(0x00); // Place Holder
		
		int jump_loc = loop_body.size() + loop_body2.size();
		int loop_loc = loop_body.size() + loop_body2.size() + rets2.size();
		
		loop_body2[loop_body2.size() - 2] = loop_loc / 256; // Replace Loop
		loop_body2[loop_body2.size() - 1] = loop_loc % 256;
		
		rets2[rets2.size() - 2] = jump_loc / 256; // Replace Checker
		rets2[rets2.size() - 1] = jump_loc % 256;
		
		for(int i = 0; i < rets2.size(); i++) rets.push_back(rets2[i]); // Initializing
		for(int i = 0; i < loop_body.size(); i++) rets.push_back(loop_body[i]); // Loop Body
		int loop_cntn = rets.size();
		for(int i = 0; i < loop_body2.size(); i++) rets.push_back(loop_body2[i]); // Step Modifying
		int loop_break = rets.size();
		
		replace_holder(loop_break, loop_cntn, loop_head, rets);
	}
	else if(flag == "foreach") {
		string cond = next_token();
		cond = MIDDLE(cond);
		vector<string> vars = remove_all(split(cond, ','), ",");
		string real_index = vars[0];
		string var = vars[0] + "__iterator";
		string first = "0";
		string expr = var + " < len(" + vars[1] + ")";
		string step = "1";
		string append = real_index + " = " + vars[1] + "[" + var + "]";
		int var_id = get_id(var);
		int read_id = get_id(real_index);
		vector<unsigned char> fret = compile_expression(first);
		for(int i = 0; i < fret.size(); i++) rets.push_back(fret[i]);
		rets.push_back(LSTORE);
		rets.push_back(var_id / 256);
		rets.push_back(var_id % 256);
		
		int loop_head = rets.size();
		vector<unsigned char> rets2 = compile_expression(expr);
		rets2.push_back(JMPN);
		rets2.push_back(0x00); // Place Holder
		rets2.push_back(0x00);
		vector<unsigned char> loop_body;
		
		vector<unsigned char> part = compile_expression(append); // Index
		
		vector<unsigned char> rets3 = compile_statement();
		for(int i = 0; i < rets3.size(); i++) part.push_back(rets3[i]);
		for(int i = 0; i < part.size(); i++) loop_body.push_back(part[i]); // Loop body
		
		vector<unsigned char> loop_body2;
		vector<unsigned char> stepr = compile_expression(step); // Step Modifying
		for(int i = 0; i < stepr.size(); i++) loop_body2.push_back(stepr[i]);
		loop_body2.push_back(LOAD);
		loop_body2.push_back(var_id / 256);
		loop_body2.push_back(var_id % 256);
		loop_body2.push_back(ADD); 
		loop_body2.push_back(LSTORE);
		loop_body2.push_back(var_id / 256);
		loop_body2.push_back(var_id % 256);
		
		loop_body2.push_back(RESET);
		loop_body2.push_back(LOOP);
		loop_body2.push_back(0x00);
		loop_body2.push_back(0x00); // Place Holder
		
		int jump_loc = loop_body.size() + loop_body2.size();
		int loop_loc = loop_body.size() + loop_body2.size() + rets2.size();
		loop_body2[loop_body2.size() - 2] = loop_loc / 256; // Replace Loop
		loop_body2[loop_body2.size() - 1] = loop_loc % 256;
		rets2[rets2.size() - 2] = jump_loc / 256; // Replace Checker
		rets2[rets2.size() - 1] = jump_loc % 256;
		for(int i = 0; i < rets2.size(); i++) rets.push_back(rets2[i]);
		for(int i = 0; i < loop_body.size(); i++) rets.push_back(loop_body[i]);
		int loop_cntn = rets.size();
		for(int i = 0; i < loop_body2.size(); i++) rets.push_back(loop_body2[i]);
		int loop_break = rets.size();
		
		replace_holder(loop_head, loop_cntn, loop_break, rets);
	}
	else if(flag == "break") {
		rets.push_back(BREAK);
		rets.push_back(0x00);
		rets.push_back(0x00);
		read_token(EOL_LABEL); // Check EOL
	}
	else if(flag == "continue") {
		rets.push_back(CNTN);
		rets.push_back(0x00);
		rets.push_back(0x00);
		read_token(EOL_LABEL); // Check EOL
	}
	else if(flag == "#END_TOKEN#") {
		;
	}
	else {
		string str = flag;
		string cur = next_token();
		while(cur != EOL_LABEL) str += cur, cur = next_token();
		vector<unsigned char> rets2 = execute(str);
		for(int i = 0; i < rets2.size(); i++) rets.push_back(rets2[i]);
	}
	tabcnt--;
	return rets;
}
vector<unsigned char> compile_block() {
	vector<unsigned char> rets;
	vector<unsigned char> rets2;
	while(has_next()) {
		rets2 = compile_statement();
		for(int i = 0; i < rets2.size(); i++) rets.push_back(rets2[i]);
		if(next_token_is(END_LABEL)) break;
	}
	return rets;
}

int main_id;
vector<unsigned char> make_lambda(string expr, string param_list) {
	vector<unsigned char> rets;
	expr = MIDDLE(expr);
	string func_name = random_name();
	all_func.push_back(func_name);
	param_list = MIDDLE(param_list);
	vector<string> args = remove_all(split(param_list, ','), ",");
	int func_id = get_id(func_name);
	vector<unsigned char> func = compile_expression(expr);
	vector<int> regs;
	func.push_back(RET);
	func.push_back(LDN);
	func.push_back(0);
	func.push_back(0);
	func.push_back(RET); 
	for(int i = 0; i < func.size(); i++) {
		byte_map[func_id][i] = func[i];
	}
	byte_map[func_id][func.size()] = END;
	for(int i = 0; i < args.size(); i++) regs.push_back(get_id(args[i]));
	reg_map[func_id] = regs;
	int id = func_id;
	// Set func_name = func_id
	vector<unsigned char> lint = load_func(func_name);
	for(int i = 0; i < lint.size(); i++) rets.push_back(lint[i]);
	rets.push_back(STORE);
	rets.push_back(id / 256);
	rets.push_back(id % 256);
	rets.push_back(LOAD);
	rets.push_back(id / 256);
	rets.push_back(id % 256);
	return rets;
}
void save_bytecode(string file) {
	ofstream fcout(file.c_str());
	fcout << all_func.size() << ' ' << main_id << ' ';
	fcout << cpool_pos << ' ';
	for(int i = 0; i < cpool_pos; i++) fcout << cpool[i].to_str() << ' ';
	for(int i = 0; i < all_func.size(); i++) {
		string func = all_func[i];
		int id = get_id(func);
		fcout << func << " ";
		fcout << id << " ";
		fcout << reg_map[id].size() << " ";
		for(int j = 0; j < reg_map[id].size(); j++) fcout << reg_map[id][j] << " ";
		stringstream sfcout("");
		int len = 0;
		for(int j = 0; byte_map[id][j] != END; j++) {
			sfcout << byte_map[id][j];
			len++;
		}
		fcout << len << " " << sfcout.str() << " ";
	}
}
void read_bytecode(string file) {
	memset(byte_map, END, sizeof(byte_map));
	ifstream fcin(file.c_str());
	int all;
	fcin >> all >> main_id;
	int ccnt;
	fcin >> ccnt;
	for(int i = 0; i < ccnt; i++) {
		double d;
		fcin >> d;
		add_const(d);
	}
	for(int i = 0; i < all; i++) {
		string func;
		int id;
		int pcnt;
		int len;
		fcin >> func >> id >> pcnt;
		var_id[func] = id;
		for(int i = 0; i < pcnt; i++) {
			int pname;
			fcin >> pname;
			reg_map[id].push_back(pname);
		}
		fcin >> len;
		fcin.get();
		for(int j = 0; j < len; j++) {
			char a;
			fcin.get(a);
			byte_map[id][j] = a;
		}
		all_func.push_back(func);
	}
}
stringstream all_source("");
void read_file_source(string file) {
	ifstream fcin(file.c_str());
	if(!fcin) {
		error("file " + file + " not found.");
	}
	string str;
	while(getline(fcin, str)) {
		if(str == "") continue;
		str = trim(str);
		vector<string> tokens = tokenize(str);
		if(tokens[0] == "import") {
			string f = "";
			for(int i = 1; i < tokens.size(); i++) {
				f += tokens[i];
			}
			read_file_source(f);
		}
		else all_source << str << endl;
	}
}
void read_cli_source() {
	string str;
	cout << ">>> ";
	while(getline(cin, str)) {
		if(str == "") continue;
		str = trim(str);
		vector<string> tokens = tokenize(str);
		if(tokens[0] == "import") {
			string f = "";
			for(int i = 1; i < tokens.size(); i++) {
				f += tokens[i];
			}
			read_file_source(f);
		}
		else all_source << str << endl;
		cout << ">>> ";
	}	
}

void cli_read() {
	read_cli_source();
}

const string LAUNCH_BLOCK = "main";
void start_compile() {
	memset(byte_map, END, sizeof(byte_map));
	all_func.push_back(LAUNCH_BLOCK);
	string str;
	main_id = get_id(LAUNCH_BLOCK);
	while(getline(all_source, str)) {
		str = trim(str);
		vector<string> tokens = tokenize(str);
		for(int i = 0; i < tokens.size(); i++) tokens[i] = trim(tokens[i]);
		for(int i = 0; i < tokens.size(); i++) {
			add_token(tokens[i]);
		}
		if(tokens[tokens.size() - 1] != BEGIN_LABEL && tokens[tokens.size() - 1] != END_LABEL) {
			add_token(EOL_LABEL);
		}
	}
	vector<unsigned char> r = compile_block();
	for(int i = 0; i < r.size(); i++) {
		byte_map[main_id][i] = r[i];
	}
	byte_map[main_id][r.size()] = END;
}

void launch() {
	run_byte(LAUNCH_BLOCK, 0, NULL, 0).to_str();
} 

const string HEX_CHAR = "0123456789ABCDEF";

string to_hex(unsigned char x) {
	string str = "";
	while(x) str += HEX_CHAR[x % 16], x /= 16;
	reverse(str.begin(), str.end());
	if(str.length() == 0) str = "00";
	if(str.length() == 1) str = "0" + str;
	return str;
}

void debug_output() {
	for(int f = 0; f < all_func.size(); f++) {
		cout << "function " << all_func[f] << endl;
		int id = get_id(all_func[f]);
		cout << "\tid " << id << endl;
		for(int i = 0; byte_map[id][i] != END; i++) {
			unsigned char b = byte_map[id][i];
			cout << "\t#" << i << "\t" << to_hex((int)b) << "\t" << opname[b];
			if(b == JMP || b == JMPN|| b == BREAK || b == CNTN) {
				cout << "\tJUMP TO " << i + 2 + byte_map[id][i + 1] * 256 + byte_map[id][i + 2];
			}
			if(b == LOOP || b == LOOPIF) {
				cout << "\tLOOP TO " << i + 2 - byte_map[id][i + 1] * 256 - byte_map[id][i + 2];
			}
			cout << endl;
		}
	}
}

bool compile = false;
bool run = false;
bool cli = true;
bool save = false;
string input_file = "";
string output_file = "";
int main(int argc, char ** argv) {
	cout << VM_NAME << ' ' << VM_VERSION << endl;
	init();
	init2();
	if(argc == 1) {
		cli_read();
		start_compile();
		if(debug) debug_output();
		launch();
		return 0;
	}
	if(argc == 2) {
		read_bytecode(argv[1]);
		launch();
		return 0;
	}
	for(int i = 1; i < argc; i++) {
		string p = argv[i];
		for(int j = 0; j < p.length(); j++) p[i] = tolower(p[i]);
		if(p == "-i") {
			i++;
			cli = false;
			if(i >= argc) error("need a file name after option '-i'");
			input_file = argv[i];
		}
		else if(p == "-o") {
			i++;
			save = true;
			if(i >= argc) error("need a file name after option '-o'");
			output_file = argv[i];
		}
		else if(p[0] == '-') {
			for(int j = 1; j < p.length(); j++) {
				if(p[j] == 'c') compile = true;
				if(p[j] == 'r') run = true;
				if(p[j] == 'd') debug = true;
			}
		}
	}
	
	if(cli) {
		cli_read();
	}
	else {
		read_file_source(input_file);
	}
	if(compile) {
		start_compile();
	}
	if(debug) {
		debug_output();
	}
	if(save) {
		save_bytecode(output_file);
	}
	launch();
	return 0;
}
