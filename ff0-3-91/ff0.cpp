#include <bits/stdc++.h>
#ifdef WIN32
#include <windows.h>
#endif
using namespace std;

const string CUR_VER = "3-91";
const string VERSION = "FScript 3.91";

bool mParse = false;
bool mCompile = false;
bool mJavaScript = false;
bool mExecute = true;
bool mAutoUpdate = true;
bool zeroTrue = false;
bool cli = false;
bool outputSym = true;
bool outputExpr = false;
string currentCommand;

struct ExecuteException {
    string msg;
};

struct DenyExecute {
    string msg;
};

#ifdef DEBUG
#define cout << allTab() << __LINE__ << " "
#else
#define DEBUG_OUTPUT debugMsg << allTab() << __LINE__ << " "
#endif

#define NEXTL DEBUG_OUTPUT << "======================" << __LINE__ << " run done ============================" << endl;
#define BEGINL DEBUG_OUTPUT << __func__ << " {" << endl; tabcnt++;
#define ENDEDL tabcnt--; DEBUG_OUTPUT << "}" << endl;

#define SYNTAX_ERR(msg, code) \
    do {\
    	cout << "FF0Script Virtual Machine has spotted an error has occured. ERROR CODE: " << code << endl;\
        cout << "SyntaxError: " << msg << endl;\
        cout << currentCommand << endl << endl;\
        if(mParse) cout << "P.S: spotted that you are using FF0Parser, please check your pseudocode." << endl;\
        if(mParse) cout << "If it can not work as well, please check the user introduction or contact developer WarfarinBloodanger." << endl;\
        throw ExecuteException();\
    } while(0)

#define RUN_ERR(msg, code) \
    do {\
    	cout << "FF0Script Virtual Machine has spotted an error has occured. ERROR CODE: " << code << endl;\
        cout << "RuntimeError: " << msg << endl;\
        cout << currentCommand << endl << endl;\
        if(mParse) cout << "P.S: spotted that you are using FF0Parser, please check your pseudocode." << endl;\
        if(mParse) cout << "If it can not work as well, please check the user introduction or contact developer WarfarinBloodanger." << endl;\
        throw ExecuteException();\
    } while(0)

#define STOP_EXEC(msg, code) \
    do {\
    	cout << "FF0Script Virtual Machine has spotted an error has occured. ERROR CODE: " << code << endl;\
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

class BigInteger
{
    void mknum(const char *s, int len = -1)
    {
        sign = 0;
        if(*s == '-')
        {
            mknum(s+1);
            sign = 1;
            return;
        }
        int l;
        if(len == -1)
            l = strlen(s);
        else
            l = len;
        l = strlen(s);
        bits.clear();
        bits.resize(l);
        for(int i = l-1; i >= 0; i--)
            bits[l-i-1] = s[i] - '0';
        maintain();
    }
    void mknum(string &s)
    {
        mknum(s.c_str(), s.length());
    }
    void us_addto(BigInteger &b)
    {
        int mlen = max(b.bits.size(), bits.size());
        int slen = bits.size();
        int olen = b.bits.size();
        bits.resize(mlen);
        for(int i = 0; i < mlen; i++)
        {
            int s = 0;
            if(i < slen)s += bits[i];
            if(i < olen)s += b.bits[i];
            bits[i] = s;
        }
        maintain();
    }
    class FFTer
    {
        class Complex
        {
        public:
            double real, image;
            Complex(double a = 0, double b = 0)
            {
                real = a;
                image = b;
            }
            Complex operator + (const Complex &o){return Complex(real+o.real, image+o.image);}
            Complex operator - (const Complex &o){return Complex(real-o.real, image-o.image);}
            Complex operator * (const Complex &o){return Complex(real*o.real-image*o.image, real*o.image+o.real*image);}
            Complex operator * (double k){return Complex(real*k, image*k);}
            Complex operator / (double k){return Complex(real/k, image/k);}
        };
        public:
        vector<Complex> a;
        int n;
        FFTer(vector<int> &vec)
        {
            a.resize(vec.size());
            for(int i = 0; i < vec.size(); i++)
                a[i].real = vec[i];
            n = vec.size();
        }
        void transform()
        {
            int j = 0;
            int k;
            for(int i = 0; i < n; i++)
            {
                if(j > i)swap(a[i], a[j]);
                k = n;
                while(j & (k >>= 1))j &= ~k;
                j |= k;
            }
        }
        void FFT(bool IDFT = false)
        {
            const double Pi = IDFT ? -acos(-1.0) : acos(-1.0);
            transform(); 
            for(int s = 1; s < n; s <<= 1)
            {
                for(int t = 0; t < n; t += s<<1)
                {
                    double x = Pi/s;
                    Complex omgn(cos(x), sin(x));
                    Complex omg(1.0, 0.0);
                    for(int m = 0; m < s; m++)
                    {      
                        int a1 = m + t;
                        int a2 = m + t + s;
                        Complex comm = omg * a[a2];
                        a[a2] = a[a1] - comm;
                        a[a1] = a[a1] + comm; 
                        omg = omg * omgn;
                    }
                }
            }
            if(IDFT)
                for(int i = 0; i < n; i++)
                    a[i] = a[i] / n;
        }
        void mul(FFTer &o)
        {
            int s = 1;
            while(s < n + o.n)s <<= 1;
            n = o.n = s;
            a.resize(s);
            o.a.resize(s);

            FFT(false);
            o.FFT(false);
            for(int i = 0; i < n; i++)
                a[i] = a[i] * o.a[i];
            FFT(true);
        }
    };
    void us_multo(BigInteger &b)
    {
        FFTer x(bits);
        FFTer y(b.bits);
        x.mul(y);
        bits.clear();
        bits.resize(x.a.size());
        for(int i = 0; i < x.n; i++)
            bits[i] = (int)(x.a[i].real+0.5);
        maintain();
    }
    void us_multo_simu(BigInteger &b)
    {
        vector<int> r;
        r.resize(max(length(),b.length())<<1);
        for(int i = 0; i < length(); i++)
            for(int j = 0; j < b.length(); j++)
                r[i+j] += bits[i] * b.bits[j];
        *(&(this -> bits)) = r;
        maintain();
    }
    void us_subto(BigInteger &b)
    {
        int mlen = length();
        int olen = b.length();
        for(int i = 0; i < mlen; i++)
        {
            int s = bits[i];
            if(i < olen)s -= b.bits[i];
            bits[i] = s;
            if(bits[i] < 0)
            {
                bits[i] += 10;
                bits[i+1] -= 1;
            }
        }
        for(int i = bits.size() - 1; !bits[i] && i >= 1; i--)bits.pop_back();
        if(bits.size() == 1 && bits[0] == 0)sign = 0;
    }
    void us_divto(BigInteger &b)
    {
        if(length() == 1 && bits[0] == 0)return;
        BigInteger L("0");
        L.sign = 0;
        BigInteger R(*this);
        R.sign = 0;
        BigInteger two("2");
        R *= two;
        BigInteger one("1");
        one.sign = 0;
        while(L + one != R)
        {
            BigInteger M = L+R;
            M.divto2();
            BigInteger t = M*b;
            if(t > *this)
            {
                R = M;
            }else if(t < *this)
            {
                L = M;
            }else
            {
                *this = M;
                L = M;
                break;
            }
        }
        *this = L;
    }
public:
    int sign;
    vector<int> bits;
    int length()
    {
        return bits.size();
    }
    void maintain()
    {
        for(int i = 0; i < bits.size(); i++)
        {
            if(i + 1 < bits.size())
                bits[i+1] += bits[i]/10;
            else if(bits[i] > 9)
                bits.push_back(bits[i]/10);
            bits[i] %= 10;
        }
        if(bits.size() == 0)
        {
            bits.push_back(0);
            sign = 0;
        }
        for(int i = bits.size() - 1; !bits[i] && i >= 1; i--)bits.pop_back();
    }

    BigInteger(string &s)
    {
        BigInteger();
        mknum(s);
    }
    BigInteger(const char *s)
    {
        BigInteger();
        mknum(s);
    }
    BigInteger(int n)
    {
        BigInteger();
        char buf[15];
        sprintf(buf, "%d", n);
        mknum(buf);
    }
    BigInteger()
    {
        sign = 0;
        bits.push_back(0);
    }
    BigInteger(const BigInteger& b) 
    {
        copy(b);
    }
    void copy(const BigInteger& b)
    {
        sign = b.sign;
        bits = b.bits;
    }
    bool us_cmp(BigInteger &b)  
    {
        if(length() != b.length())return false;
        int l = length();
        for(int i = 0; i < l; i++)
            if(bits[i] != b.bits[i])
                return false;
        return true;
    }
    bool us_larger(BigInteger &b)
    {
        if(length() > b.length())return true;
        else if(length() < b.length())return false;
        int l = length();
        for(int i = l-1; i >= 0; i--)
            if(bits[i] > b.bits[i])
                return true;
            else if(bits[i] < b.bits[i])
                return false;
        return false;
    }
    bool operator== (BigInteger &o)
    {
        if(sign != o.sign)
            return false;
        return us_cmp(o);
    }
    bool operator!= (BigInteger &o)
    {
        return !(*this == o);
    }
    bool operator> (BigInteger &o)
    {
        if(sign == 0 && o.sign == 1)return true;
        if(sign == 1 && o.sign == 0)return false;
        if(sign == o.sign && sign)return !us_larger(o);
        return us_larger(o);
    }
    bool operator< (BigInteger &o)
    {
        return !(*this == o || *this > o);
    }
    bool operator<= (BigInteger &o)
    {
        return *this < o || *this == o;
    }
    bool operator>= (BigInteger &o)
    {
        return *this > o || *this == o;
    }
    BigInteger& operator+= (BigInteger &o)
    {
        if(!sign && !o.sign)
        {
            us_addto(o);
            sign = 0;
        }
        else if(sign && o.sign)
        {
            us_addto(o);
            sign = 1;
        }
        else if(sign && !o.sign)
        {
            if(o.us_larger(*this))
            {
                BigInteger t(o);
                t.us_subto(*this);
                *this = t;
                sign = 0;
            }else
            {
                us_subto(o);
                sign = 1;
                if(bits.size() == 1 && bits[0] == 0)sign = 0;
            }
        }else if(!sign && o.sign)
        {
            if(us_larger(o))
            {
                us_subto(o);
                sign = 0;
            }else
            {
                BigInteger t(o);
                t.us_subto(*this);
                *this = t;
                sign = 1;
                if(bits.size() == 1 && bits[0] == 0)sign = 0;
            }
        }
        return *this;
    }
    BigInteger operator+ (BigInteger &o)
    {
        BigInteger t(*this);
        t += o;
        return t;
    }
    BigInteger& operator*= (BigInteger &o)
    {
        if(length() + o.length() > 800)
            us_multo(o);                
        else
            us_multo_simu(o);             
        if(sign == o.sign)sign = 0;
        else sign = 1;
        return *this;
    }
    BigInteger operator* (BigInteger &o)
    {
        BigInteger t(*this);
        t *= o;
        return t;
    }
    BigInteger& operator-= (BigInteger &o)
    {
        if(!sign && !o.sign) 
        {
            if(us_larger(o))
            {
                us_subto(o);
                sign = 0;
            }
            else
            {
                BigInteger t(o);
                t.us_subto(*this);
                *this = t;
                sign = 1;
                if(bits.size() == 1 && bits[0] == 0)sign = 0;
            }
        }else if(sign && o.sign)
        {
            if(us_larger(o))
            {
                us_subto(o);
                sign = 1;
                if(bits.size() == 1 && bits[0] == 0)sign = 0;
            }else
            {
                BigInteger t(o);
                t.us_subto(*this);
                *this = t;
                sign = 0;
            }
        }else if(!sign && o.sign)
        {
            us_addto(o);
            sign = 0;
        }else if(sign && !o.sign)
        {
            us_addto(o);
            sign = 1;
        }
        return *this;
    }
    BigInteger operator- (BigInteger &o)
    {
        BigInteger t(*this);
        t -= o;
        return t;
    }
    BigInteger& divto2()
    {
        if(!bits.size())return *this;
        bits[0] >>= 1;
        int i;
        for(i = 1; i < bits.size(); i++)
        {
            if(bits[i] & 1)bits[i-1] += 5;
            bits[i] >>= 1;
        }
        if(bits[i-1] == 0)bits.pop_back();
        return *this;
    }
    BigInteger& operator/= (BigInteger &o)
    {
    	BigInteger zero = "0";
    	if(o == zero) RUN_ERR("Can not devide by zero.", 25);
        us_divto(o);
        if(sign == o.sign)sign = 0;
        else sign = 1;
        return *this;
    }
    BigInteger operator/ (BigInteger &o)
    {
        BigInteger t(*this);
        t /= o;
        return t;
    }
    BigInteger abs()
    {
        BigInteger t(*this);
        t.sign = 0;
        return t;
    }


    BigInteger sqrt()
    {
        BigInteger L("0"), R(*this);
        BigInteger one("1");
        BigInteger m, t;
        while(L + one != R)
        {
            m = L+R;
            m.divto2();
            BigInteger t = m*m;
            if(t == *this)return m;
            else if(t > *this)R = m;
            else L = m;
        }
        return L;
    }

    BigInteger pow(BigInteger &e)
    {
        if(e.sign)return 1;
        BigInteger ans("1");
        BigInteger base(*this);
        BigInteger zero("0");
        BigInteger exp(e);
        while(exp > zero)
        {
            if(exp.bits[0] & 1)
            {
                ans *= base;
            }
            base *= base;
            exp.divto2();
        }
        if(sign && e.bits[0] & 1)ans.sign = 1;
        return ans;
    }

    BigInteger log(BigInteger &base)
    {
        if(sign)return 0;
        if(length() == 1 && bits[0] == 1)return 0;
        if(*this <= base)return 1;
        BigInteger one("1");

        BigInteger r("1");
        BigInteger c("0");
        while(r < *this)
        {
            r *= base;
            c += one;
        }
        if(r != *this)c -= one; 
        return c;
    }
    BigInteger lg()
    {
        BigInteger ten("10");
        return log(ten);
    }

    BigInteger factorial()
    {
        BigInteger r("1");
        BigInteger zero("0");
        BigInteger one("1");
        BigInteger t(*this);
        while(t > zero)
        {
            r *= t;
            t -= one;
        }
        return r;
    }
  
    friend istream& operator>>(istream &is, BigInteger &b)
    {
        string s;
        is >> s;
        b.mknum(s);
        return is;
    }
    friend ostream& operator<<(ostream &os, BigInteger b)
    {
        if(b.sign)os << '-';
        for(int i = b.bits.size()-1; i >= 0; i--)os << b.bits[i];
        return os;
    }

    string to_string()
    {
        int sz = length();
        string s;
        if(sign)
            s.resize(sz+1);
        else
            s.resize(sz);
        int i = 0;
        if(sign)s[i++] = '-';
        for(int j = sz-1; i < sz+sign; i++, j--)
            s[i] = bits[j] + '0';
        return s;
    }    

};

void checkUpdate() {
	string command;
	string ver;
	
	system("curl https://warfarinbloodanger.github.io/ff0-script/readme.md -o version.txt -s");
	
	ifstream fcin("version.txt");
	if(!fcin) return;
	
	getline(fcin, ver);
	if(ver != CUR_VER) {
		cout << "New FF0 script version availdable." << endl;
		cout << "Current: " << CUR_VER << endl;
		cout << "Newest: " << ver << endl;
		ifstream newest(("ff0-" + ver + ".cpp").c_str());
		if(newest) {
			cout << "You have downloaded the newest version of ff0 script." << endl;
		}
		else {
			cout << endl;
			cout << "Downloading source code from github..." << endl;
			for(int i = 0; i < ver.length(); i++) if(ver[i] == ' ') ver[i] == '-';
			system(("curl https://warfarinbloodanger.github.io/ff0-script/ff0-" + ver + "/FF0.cpp -o ff0-" + ver + ".cpp").c_str());
			cout << "Update Finished." << endl;
			cout << "The new source file: ff0-" << ver << ".cpp." << endl; 
		}
	}
	
	fcin.close();
	system("del version.txt"); 
}

typedef int Ref;
typedef int ValueType;

vector<string> EMPTY;

vector<string> protectVar;

const int ARR_GROW = 1024 * 256;

map<string, int> bindPower;

vector<vector<string> > usedLocalVar;
vector<string> callStack;
map<string, vector<string> > funcTable;
map<string, vector<string> > paramTable;
map<string, vector<string> > ptypeTable;

const string opchar = "+-*/!=><%&|@^";
const string spliter = "+-*/=><;!&|%@^,~#";

int tabcnt = 0;

stringstream codeStream("");

string trim(string s) {
    int x = 0, y = s.length() - 1;
    while(isspace(s[x])) x++;
    while(isspace(s[y])) y--;
    return s.substr(x, y - x + 1);
}

string allTab() {
    string r = "";
    for(int i = 0; i < tabcnt; i++) r += "    ";
    return r;
}

int hashCnt;
map<string, int> hashValue;
int getHash(string s) {
    if(hashValue.find(s) == hashValue.end()) hashCnt++, hashValue[s] = hashCnt;
    return hashValue[s] + 1024 * 252;
}

string parseNum(double num) {
    char buffer[205];
    sprintf(buffer, "%.14g", num);
    return buffer;
}

string parseNumD(long long num) {
    stringstream ss("");
    ss << num;
    return ss.str();
}

string parseNum(BigInteger num) {
	stringstream ss("");
	ss << num;
	return ss.str();
}

stringstream debugMsg("");

struct PseudocodeParser {
	#define PARSE_ERR(msg, code) \
		do {\
			SYNTAX_ERR(msg, 20);\
			throw ExecuteException();\
		} while(false)
	
	map<string, string> operators;
	
	string findReflect(string reflect) {
		if(operators.find(reflect) != operators.end()) return operators[reflect];
		else return reflect; 
	}
	
	vector<vector<string> > tokenize(string text) {
		vector<vector<string> > vses;
		vector<string> vs;
		string cur = "";
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
		vector<vector<string> > ret;
		for(int i = 0; i < vses.size(); i++) {
			vector<string> retvs;
			for(int j = 0; j < vses[i].size(); j++) {
				if(vses[i][j] == "to") {
					if(j > 0 && (vses[i][j - 1] == "equals" || vses[i][j - 1] == "not-equals")) retvs[retvs.size() - 1] += " to";
					else PARSE_ERR("Need an 'equals' or a 'not-equals' before 'to'.", 1);
				}
				else if(vses[i][j] == "is") {
					retvs.push_back("is");
					if(j < vses[i].size() - 1 && (vses[i][j + 1] == "true" || vses[i][j + 1] == "false")) {
						retvs[retvs.size() - 1] += " " + vses[i][++j];
						continue;
					}
					else if(j < vses[i].size() - 1 && (vses[i][j + 1] == "assigned")) {
						if(j < vses[i].size() - 2 && (vses[i][j + 2] == "with")) retvs[retvs.size() - 1] += " assigned with";
						else PARSE_ERR("Need a 'with' after assigned.", 4);
						j++; j++;
						continue;
					}
					else if(j < vses[i].size() - 1 && (vses[i][j + 1] == "bigger" || vses[i][j + 1] == "not-bigger" || vses[i][j + 1] == "smaller" || vses[i][j + 1] == "not-smaller")) retvs[retvs.size() - 1] += " " + vses[i][++j];
					else PARSE_ERR("Need a compare-operator or a 'true' or a 'false' after 'is'.", 2);
					if(j < vses[i].size() - 1 && vses[i][j + 1] == "than") retvs[retvs.size() - 1] += " " + vses[i][++j];
					else PARSE_ERR("Need a 'than' after the compare-operator.", 3);
				}
				else retvs.push_back(vses[i][j]);
			}
			ret.push_back(retvs);
		}
		return ret;
	}
	
	int tabs;
	string getTab() {
		string s = "";
		for(int i = 0; i < tabs; i++) s += "    ";
		return s;
	}
	
	string translate(vector<string> vs) {
		stringstream result("");
		string flag = vs[0];
		if(vs[0] == "If") {
			result << getTab();
			result << "if (";
			for(int i = 1; i < vs.size() - 3; i++) {
				result << ' ' << findReflect(vs[i]);
			}
			if(vs[vs.size() - 3] != "is true") PARSE_ERR("Need an 'is true' after the expression.", 4);
			if(vs[vs.size() - 2] != ",") PARSE_ERR("Need a comma after 'is true'.", 5);
			if(vs[vs.size() - 1] != "do:") PARSE_ERR("Need a 'then:' after the if-statement condition.", 6);
			result << " ) {";
			tabs++;
		}
		else if(vs[0] == "Or") {
			result << getTab();
			result << "elseif (";
			for(int i = 1; i < vs.size() - 3; i++) {
				result << ' ' << findReflect(vs[i]);
			}
			if(vs[vs.size() - 3] != "is true") PARSE_ERR("Need an 'is true' after the expression.", 4);
			if(vs[vs.size() - 2] != ",") PARSE_ERR("Need a comma after 'is true'.", 5);
			if(vs[vs.size() - 1] != "do:") PARSE_ERR("Need a 'do:' after the or-statement condition.", 6);
			result << " ) {";
			tabs++;
		}
		else if(vs[0] == "Otherwise") {
			if(vs.size() <= 1 || vs[1] != ",") PARSE_ERR("Need a comma after 'Otherwise'.", 4);
			if(vs.size() <= 2 || vs[2] != "do:") PARSE_ERR("Need a 'do:' after the comma.", 4);
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
			if(vs.size() <= 1 || vs[1] != "until") PARSE_ERR("Need an 'until' after 'Repeat'.", 4);
			for(int i = 2; i < vs.size() - 3; i++) {
				result << ' ' << findReflect(vs[i]);
			}
			if(vs[vs.size() - 3] != "is false") PARSE_ERR("Need an 'is false' after the expression.", 4);
			if(vs[vs.size() - 2] != ",") PARSE_ERR("Need a comma after 'is false'.", 5);
			if(vs[vs.size() - 1] != "do:") PARSE_ERR("Need a 'do:' after the repeat-statement condition.", 6);
			result << " ) {";
			tabs++;
		}
		else if(vs[0] == "Function") {
			result << getTab();
			result << "function ";
			if(vs.size() <= 1 || vs[1] != ",") PARSE_ERR("Need a comma after 'Function'.", 4);
			if(vs.size() <= 2 || vs[2] != "called") PARSE_ERR("Need a 'called' after the comma.", 4);
			if(vs.size() <= 3) PARSE_ERR("Need a name after the 'called'.", 4);
			else result << vs[3] << " (";
			if(vs.size() <= 4 || vs[4] != ",") PARSE_ERR("Need a comma after the name of function.", 4);
			if(vs.size() <= 5 || vs[5] != "needs") PARSE_ERR("Need a 'needs' after the comma.", 4);
			int cnt;
			if(vs.size() <= 6) PARSE_ERR("Need a number after the comma.", 4);
			else {
				if(vs[6] == "no") cnt = 0;
				else cnt = atoi(vs[6].c_str());
				if(cnt < 0) PARSE_ERR("The count of parameters can not be lower than zero.", 8);
			}
			if(vs.size() <= 7) PARSE_ERR("Need a " + (string)(cnt == 1 ? "'parameter'" : "'parameters'") + " after the number.", 4);
			else {
				if(cnt == 1) {
					if(vs[7] != "parameter") PARSE_ERR("Need a 'parameter' after the number.", 4);
				}
				else if(vs[7] != "parameters") PARSE_ERR("Need a 'parameters' after the number.", 4);
			}
			if(vs.size() <= 8 || vs[8] != ",") PARSE_ERR("Need a comma after " + (string)(cnt == 1 ? "'parameter'" : "'parameters'") + ".", 4);
			if(cnt != 0) {
				if(vs.size() <= 9 || vs[9] != "called") PARSE_ERR("Need a 'called' after the comma.", 4);
				int x = 10;
				while(cnt--) {
					if(vs.size() > x) {
						result << ' ' << vs[x];
						x++;
					}
					else PARSE_ERR("Need a name.", 4);
					if(vs.size() <= x || vs[x] != ",") {
						PARSE_ERR("Need a comma after the name.", 4);
					}
					else x++;
					if(vs.size() > x) {
						if(vs[x] == "must") {
							x++;
							if(vs.size() <= x || vs[x] != "be") {
								PARSE_ERR("Need a 'be' after 'must'.", 4);
							}
							else x++;
							if(vs.size() > x) result << " : " << vs[x];
							else PARSE_ERR("Need a type name after 'be'.", 4);
							x++;	
							if(vs.size() <= x || vs[x] != ",") PARSE_ERR("Need a comma after the type name.", 4);
							else x++;
						}
					}
					if(cnt != 0) result << " ,"; 
				}
				if(vs.size() <= x || vs[x] != "do:") PARSE_ERR("Need a 'do:' after the parameter list.", 4);
				result << " ) {";
			}
			else {
				if(vs.size() <= 9 || vs[9] != "do:") PARSE_ERR("Need a 'do:' after the parameter list.", 4);
				result << " ) {";
			}
			tabs++;
		}
		else if(vs[0] == "For") {
			result << getTab();
			result << "for (";
			
			string variable, from, to;
			if(vs.size() <= 1) PARSE_ERR("Need a name after 'For'.", 7);
			variable = vs[1];
			result << ' ' << vs[1] << " ,";
			if(vs.size() <= 2 || vs[2] != "from") PARSE_ERR("Need a name after 'From'.", 8);
			
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
			
			
			if(vs[vs.size() - 2] != ",") PARSE_ERR("Need a comma after the expression.", 5);
			if(vs[vs.size() - 1] != "do:") PARSE_ERR("Need a 'do:' after the for-statement condition.", 6);
			tabs++;
		}
		else {
			result << getTab();
			if(vs.size() > 0) result << findReflect(vs[0]);
			for(int i = 1; i < vs.size(); i++) result << ' ' << findReflect(vs[i]);
		}
		return result.str();
	}
	
	string translateBlock(vector<vector<string> > vses) {
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
	
	string main(string source) {
		initialize();
		string s;
		stringstream code("");
		stringstream buffer("");
		buffer << source << endl;
		while(getline(buffer, s)) {
			code << s << ";" << endl;	
		}
		try {
			vector<vector<string> > vses = tokenize(code.str());
			return translateBlock(vses);
		}
		catch(ExecuteException exc) {
			cout << "Translate Failed." << endl;
		}
		return "# Not reached.";
	}
};

enum { BREAK, CONTINUE };
struct LoopMessage {
	int type;
};
const int ARRCNT = 64;
bool allIndexes[ARRCNT];

bool checkName(string s) {
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
		return i;
	}
}

void freeMemory(int index) {
	allIndexes[index] = false;
}

vector<FILE *> filePtr;
int fileOpen(string file, string mode = "rw") {
	FILE * ptr = fopen(file.c_str(), mode.c_str());
	if(ptr == NULL) {
		RUN_ERR("File doesn't exist: " + file, 1);
	}
	filePtr.add(ptr);
	return filePtr.size() - 1;
}

void checkHandle(int handle) {
	if(handle < 0 || handle >= filePtr.size()) RUN_ERR("Invalid handle: " + parseNum(handle), 2);
}

void fileClose(int handle) {
	checkHandle(handle);
	fclose(filePtr[handle]);
}

char fbuffer[2048];
string freadString(int handle) {
	checkHandle(handle);
	string result = "";
	char c;
	while(isspace(c) && !feof(filePtr[handle])) c = fgetc(filePtr[handle]);
	c = fgetc(filePtr[handle]);
	while(!isspace(c) && !feof(filePtr[handle])) {
		result += c;
		c = fgetc(filePtr[handle]);
	}
	return result;
}
string freadStrLine(int handle) {
	checkHandle(handle);
	string result = "";
	char c;
	while(isspace(c) && !feof(filePtr[handle])) c = fgetc(filePtr[handle]);
	c = fgetc(filePtr[handle]);
	while(c != '\n' && !feof(filePtr[handle])) {
		result += c;
		c = fgetc(filePtr[handle]);
	}
	return result;
}
void fwriteString(int handle, string text) {
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
    T_REF, T_TRUE, T_FALSE, T_NULL, T_UNKNOWN, T_UNDEF, T_BIGNUM, T_HWND
};

string typeName[] = {
    "Number", "String", "Object", "Closure",
    "Reference", "Bool", "Bool", "Null", "Unknown", "Undefined", "BigInteger", "HWND"
};


enum {
    PUBLIC, PRIVATE
};

struct Object {
    string toString() {
        return "[Unknown Object]";
    }
};

struct Closure {
    string toString() {
        return "[Unknown Closure]";
    }
};

string parseNum(double);
string parseNum(BigInteger);

int nameCnt;
string makeName() {
    string ret = "_inner";
    ret += parseNum(nameCnt++);
    return ret;
}

#ifdef WIN32
string parseHWND(HWND hwnd) {
	stringstream ss("");
	ss << "[HWND ";
	ss << hwnd;
	ss << "]";
	return ss.str();
}
#endif

BigInteger ZERO("0"); 

const int MAX_NUMBER = 2147483647;

struct Value {
    double numVal;
    string strVal;
    Object objVal;
    string funVal;
    BigInteger bigVal;
    Ref refVal;
    #ifdef WIN32
    HWND hwndVal;
    #endif
    
    Value() {
        type = T_UNDEF;
    }

    ValueType type;
    string toString() {
        if(type == T_NUMBER) return parseNum(numVal);
        else if(type == T_STRING) return strVal;
        else if(type == T_OBJECT) return objVal.toString();
        else if(type == T_FUNCTION) return "[function " + funVal + "]";
        else if(type == T_REF) return "$" + parseNum(refVal);
        else if(type == T_TRUE) return "true";
        else if(type == T_FALSE) return "false";
        else if(type == T_NULL) return "null";
        else if(type == T_BIGNUM) return parseNum(bigVal);
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
        long long inum = num;
        if(inum > MAX_NUMBER) {
    		return Value::makeBigInteger(parseNumD(inum).c_str());
		}
		else {
       		v.type = T_NUMBER, v.numVal = num;
		}
        return v;
    }

    static Value makeString(string str) {
        Value v;
        v.type = T_STRING, v.strVal = str;
        return v;
    }

    static Value makeObject(Object obj) {
        Value v;
        v.type = T_OBJECT, v.objVal = obj;
        return v;
    }

    static Value makeFunction(string cls) {
        Value v;
        v.type = T_FUNCTION, v.funVal = cls;
        return v;
    }
    
    static Value makeBigInteger(BigInteger big) {
    	Value v;
		v.type = T_BIGNUM, v.bigVal = big;
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
    
    Value toBigInteger(Value val) {
    	return Value::makeBigInteger(parseNumD(val.numVal).c_str());
	}

    Value operator + (Value & v) {
        if(type == T_STRING || v.type == T_STRING) return Value::makeString(strVal + v.toString());
        else if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not add " + typeName[v.type] + " to a number.", 3);
            return Value::makeNumber(v.numVal + numVal);
        } else if(type == T_BIGNUM) {
        	if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger(bigVal + b);
			}
            if(v.type != T_BIGNUM) RUN_ERR(" can not add " + typeName[v.type] + " to a big integer.", 3);
           	return Value::makeBigInteger(v.bigVal + bigVal);
        } else if(type == T_REF) {
            if(v.type != T_NUMBER) RUN_ERR(" can not add " + typeName[v.type] + " to an reference.", 3);
            return Value::makeRef(v.numVal + refVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '+'.", 3);
    }

    Value operator - (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not substract " + typeName[v.type] + " to a number.", 3);
            return Value::makeNumber(numVal - v.numVal);
        } else if(type == T_BIGNUM) {
			if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger(bigVal - b);
			}
            if(v.type != T_BIGNUM) RUN_ERR(" can not substract " + typeName[v.type] + " to a big integer.", 3);
            return Value::makeBigInteger(v.bigVal - bigVal);
        }  else if(type == T_REF) {
            if(v.type != T_NUMBER) RUN_ERR(" can not substract " + typeName[v.type] + " to an reference.", 3);
            return Value::makeRef(numVal - v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '-'.", 3);
    }

    Value operator * (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not multiply " + typeName[v.type] + " to a number.", 3);
            return Value::makeNumber(numVal * v.numVal);
        } else if(type == T_BIGNUM) {
			if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger(bigVal * b);
			}
            if(v.type != T_BIGNUM) RUN_ERR(" can not multiply " + typeName[v.type] + " to a big integer.", 3);
            return Value::makeBigInteger(v.bigVal * bigVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '*'.", 3);
    }

    Value operator / (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not divide " + typeName[v.type] + " to a number.", 3);
            return Value::makeNumber(numVal / v.numVal);
        } else if(type == T_BIGNUM) {
			if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger(bigVal / b);
			}
            if(v.type != T_BIGNUM) RUN_ERR(" can not divide " + typeName[v.type] + " to a big integer.", 3);
            return Value::makeBigInteger(v.bigVal / bigVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '/'.", 3);
    }
    
    Value operator & (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not bit-and " + typeName[v.type] + " to a number.", 3);
            return Value::makeNumber((int)numVal & (int)v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '&'.", 3);
    }
    
    Value operator << (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not bit-and " + typeName[v.type] + " to a number.", 3);
            return Value::makeNumber((int)numVal << (int)v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '&'.", 3);
    }
    
    Value operator >> (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not bit-and " + typeName[v.type] + " to a number.", 3);
            return Value::makeNumber((int)numVal >> (int)v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '&'.", 3);
    }
    
    Value operator xor (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not bit-xor " + typeName[v.type] + " to a number.", 3);
            return Value::makeNumber((int)numVal xor (int)v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation xor.", 3);
    }

    Value operator | (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not bit-or " + typeName[v.type] + " to a number.", 3);
            return Value::makeNumber((int)numVal | (int)v.numVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '|'.", 3);
    }

    Value operator > (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.", 3);
            return Value::makeBool(numVal > v.numVal);
        } else if(type == T_BIGNUM) {
			if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger(bigVal > b);
			}
            if(v.type != T_BIGNUM) RUN_ERR(" can not compare " + typeName[v.type] + " with a big integer.", 3);
            return Value::makeBool(bigVal > v.bigVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '>'.", 3);
    }

    Value operator < (Value & v) {
        if(type == T_NUMBER) {
			if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger(bigVal < b);
			}
            if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.", 3);
            return Value::makeBool(numVal < v.numVal);
        } else if(type == T_BIGNUM) {
            if(v.type != T_BIGNUM) RUN_ERR(" can not compare " + typeName[v.type] + " with a big integer.", 3);
            return Value::makeBool(bigVal < v.bigVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '<'.", 3);
    }

    Value operator >= (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.", 3);
            return Value::makeBool(numVal >= v.numVal);
        } else if(type == T_BIGNUM) {
			if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger(bigVal >= b);
			}
            if(v.type != T_BIGNUM) RUN_ERR(" can not compare " + typeName[v.type] + " with a big integer.", 3);
            return Value::makeBool(bigVal >= v.bigVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '>='.", 3);
    }

    Value operator <= (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.", 3);
            return Value::makeBool(numVal <= v.numVal);
        } else if(type == T_BIGNUM) {
			if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger(bigVal <= b);
			}
            if(v.type != T_BIGNUM) RUN_ERR(" can not compare " + typeName[v.type] + " with a big integer.", 3);
            return Value::makeBool(bigVal <= v.bigVal);
        } else RUN_ERR(" type " + typeName[type] + " doesn't support operation '<='.", 3);
    }

    Value operator % (Value & v) {
        if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not mod " + typeName[v.type] + " with a number.", 3);
            return Value::makeNumber(fmod(v.numVal, numVal));
        } /*else if(type == T_BIGNUM) {
            if(v.type != T_BIGNUM) RUN_ERR(" can not mod " + typeName[v.type] + " with a big integer.", 3);
            return Value::makeBigInteger(0); // There is something to do...
        }*/ else RUN_ERR(" type " + typeName[type] + " doesn't support operation '%'.", 3);
    }

    Value operator == (Value & v) {
        if(type == T_STRING || v.type == T_STRING) {
            return Value::makeBool(toString() == v.toString());
        } else if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.", 3);
            return Value::makeBool(numVal == v.numVal);
        } else if(type == T_BIGNUM) {
			if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger(bigVal == b);
			}
            if(v.type != T_BIGNUM) RUN_ERR(" can not compare " + typeName[v.type] + " with a big integer.", 3);
            return Value::makeBool(v.bigVal == bigVal);
        } else if(type == T_REF) {
            return Value::makeBool(refVal == v.refVal);
        } else return Value::makeBool(toString() == v.toString() && type == v.type);
    }

    Value operator != (Value & v) {
        if(type == T_STRING || v.type == T_STRING) {
            return Value::makeBool(toString() != v.toString());
        } else if(type == T_NUMBER) {
            if(v.type != T_NUMBER) RUN_ERR(" can not compare " + typeName[v.type] + " with a number.", 3);
            return Value::makeBool(numVal != v.numVal);
        } else if(type == T_BIGNUM) {
			if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger(bigVal != b);
			}
            if(v.type != T_BIGNUM) RUN_ERR(" can not compare " + typeName[v.type] + " with a big integer.", 3);
            return Value::makeBool(v.bigVal != bigVal);
        } else if(type == T_REF) {
            return Value::makeBool(refVal != v.refVal);
        } else return Value::makeBool(toString() != v.toString() || type != v.type);
    }

    Value operator && (Value & v) {
        if(type == T_NUMBER) {
            if(v.type == T_TRUE) return Value::makeBool(true);
            else if(v.type == T_NUMBER) return Value::makeBool(v.numVal && numVal);
        } else if(type == T_BIGNUM) {
			if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger((bigVal != ZERO) && (b != ZERO));
			}
            if(v.type != T_BIGNUM) RUN_ERR(" can not compare " + typeName[v.type] + " with a big integer.", 3);
            return Value::makeBool((v.bigVal != ZERO) && (bigVal != ZERO));
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
        } else if(type == T_BIGNUM) {
			if(v.type == T_NUMBER) {
        		BigInteger b = toBigInteger(v).bigVal;
        		return Value::makeBigInteger((bigVal != ZERO) || (b != ZERO));
			}
            if(v.type != T_BIGNUM) RUN_ERR(" can not compare " + typeName[v.type] + " with a big integer.", 3);
            return Value::makeBool((v.bigVal != ZERO) || (bigVal != ZERO));
        } else if(type == T_TRUE) {
            return Value::makeBool(true);
        } else return Value::makeBool(false);
        return Value::makeBool(false);
    }

    Value operator ! () {
        if(type == T_NULL) return Value::makeBool(true);
        else if(type == T_FALSE) return Value::makeBool(true);
        else if(type == T_NUMBER && numVal == 0) return Value::makeBool(true);
        else if(type == T_BIGNUM) return Value::makeBool(bigVal == ZERO);
        else return Value::makeBool(false);
    }
};

typedef Value (*NativeFF0)(vector<Value>);
vector<NativeFF0> nativeFunc;
vector<string> nativeNames;

bool isFalse(Value v) {
    return (!v).type == T_TRUE;
}

typedef vector<Value> Heap;
typedef map<string, Value> Scope;

Heap EMPTY_V;
Heap heap;
Scope scope;

void setRefVal(Value, Value);

bool findchar(string s, char c) {
	for(int i = 0; i < s.length(); i++) {
		if(s[i] == c) return true;
	}
	return false;
}

string parseSubscript(string);
Value evalExpr(string expr);

Value getVarVal(string name) {
	if(findchar(name, '.')) return evalExpr(parseSubscript(name));
	if(!checkName(name)) RUN_ERR("invalid variable name: " + name, 4);
    for(int i = callStack.size() - 1; i >= 0; i--) {
        string newname = callStack[i] + name;
        if(scope.find(newname) == scope.end()) continue;
        else {
        	return scope[newname];
		}
    }
    SYNTAX_ERR("variable " + name + " is not declared.", 5);
    scope[callStack[callStack.size() - 1] + name] = Value();
    return scope[callStack[callStack.size() - 1] + name];
}

void setLocalVarVal(string name, Value v) {
    usedLocalVar[usedLocalVar.size() - 1].add(name);
    scope[callStack[callStack.size() - 1] + name] = v;
}

void setVarVal(string name, Value v) {
	if(!checkName(name)) RUN_ERR("invalid variable name: " + name, 4);
    for(int i = callStack.size() - 1; i >= 0; i--) {
        string newname = callStack[i] + name;
        if(scope.find(newname) != scope.end()) {
        	scope[newname] = v;
        	return;
		}
        else continue;
    }
    usedLocalVar[usedLocalVar.size() - 1].add(name);
    scope[callStack[callStack.size() - 1] + name] = v;
}
Value getRefVal(Value, Value);
void freeVariable(Value v) {
	if(v.type != T_REF) return;
	int len = 0;
    int i = v.refVal;
    while(true) {
        if(heap[i].type == T_UNDEF) break;
        i++, len++;
    }
    freeMemory(v.refVal / ARR_GROW);
	if(v.type == T_REF) {
		for(int x = 0; x < len; x++) {
			freeVariable(getRefVal(v, Value::makeNumber(x)));
		}
	}
}

bool includeChar(string s, char c) {
    for(int i = 0; i < s.length(); i++) if(s[i] == c) return true;
    return false;
}

string joinString(vector<string> vs, string spaces = "") {
    string r = "";
    for(int i = 0; i < vs.size(); i++) r += vs[i] + spaces;
    return r;
}

vector<string> subVS(vector<string> vs, int from, int to) {
    vector<string> result;
    to = (to > vs.size()) ? vs.size() : to;
    for(int i = from; i < to; i++) result.add(vs[i]);
    return result;
}

Value getRefVal(Value v, Value locate) {
    if(v.type != T_STRING && v.type != T_REF) RUN_ERR(" can not get reference of '" + typeName[v.type] + "'", 6);
    if(locate.type != T_NUMBER && locate.type != T_STRING) RUN_ERR(" can not use '" + typeName[v.type] + "' as position", 7);

    if(locate.type == T_STRING) locate = Value::makeNumber(getHash(locate.strVal));

    if(v.type == T_STRING) {
        int loc = locate.numVal;
        if(loc < 0 || loc >= v.strVal.length()) RUN_ERR(" index " + locate.toString() + " out of memory size", 8);
        string ret = "";
        ret += v.strVal[loc];
        return Value::makeString(ret);
    } else {
        int position = v.refVal + locate.numVal;
        if(position < 0 || position >= heap.size()) RUN_ERR(" index " + locate.toString() + " out of memory size", 8);
        return heap[position];
    }

    return Value::makeNull();
}

void setRefVal(Value v, Value setVal) {
    if(v.type != T_REF) SYNTAX_ERR(" can not set reference of '" + typeName[v.type] + "'", 6);

    heap[v.refVal] = setVal;
}

int currentIC = 0;

vector<string> removeAll(vector<string> vs, string remv) {
    vector<string> result;
    for(int i = 0; i < vs.size(); i++) {
        if(remv != vs[i]) result.add(vs[i]);
    }
    return result;
}

void lookVariable() {
	
}

Value makeArray(vector<Value> vals, bool useWay = true) {
    if(vals.size() > ARR_GROW) RUN_ERR(" array size out of maximum limit:\n" + parseNum(ARR_GROW) + " < " + parseNum(vals.size()), 9);
    int icIndex = allocMemory();
    if(!useWay) {
    	string name = makeName();
    	Value val = Value::makeRef(icIndex * ARR_GROW);
    	setVarVal(name, val);
    	usedLocalVar[usedLocalVar.size() - 1].push_back(name);
	}
    for(int i = 0; i < vals.size(); i++) {
        heap[ARR_GROW * icIndex + i] = vals[i];
    }

    Value ret = Value::makeRef(icIndex * ARR_GROW);
    return ret;
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
	vector<string> ret;
	vector<string> final;
	for(int i = 0; i < tmp.size(); i++) {
		if(tmp[i] != "") {
			ret.add(trim(tmp[i]));
		}
	}
    for(int i = 0; i < ret.size(); i++) {
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
            } else if(ret[i] == ">") {
                if(ret[i - 1] == ">") final[final.size() - 1] += ">";
                else final.add(ret[i]);
            } else if(ret[i] == "<") {
                if(ret[i - 1] == "<") final[final.size() - 1] += "<";
                else final.add(ret[i]);
            } else final.add(ret[i]);
        } else final.add(ret[i]);
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

string parseSubscript(string expr) {
	vector<string> vs = split(expr, '.');
	string result = vs[0];
	for(int i = 0; i < vs.size(); i++) {
		if(i == 0 || vs[i] == ".") continue;
		result += "[\"" + vs[i] + "\"]";
	}
	return result;
}

string standardSubscript(string str) {
    vector<string> vs = tokenize(str);
    vector<string> result;
    string ret = "";
    for(int i = 0; i < vs.size(); i++) {
        if(vs[i][0] == '[') {
            string subExpr = MIDDLE(vs[i]);
            result.add("@");
            result.add("(" + subExpr + ")");
        } else result.add(vs[i]);
    }

    ret = joinString(result);
    return ret;
}

vector<string> buildWith(vector<string> vs, string builder) {
    string r = "";
    vector<string> ret;
    for(int i = 0; i < vs.size(); i++) {
        if(vs[i] == builder) {
            ret.add(r);
            r = "";
            ret.add(builder);
        } else r += vs[i] + " ";
    }
    ret.add(r);
    return ret;
}

string standardFunctionCall(string str) {
    vector<string> vs = tokenize(str);
    vector<string> result;
    string ret = "";
    for(int i = 0; i < vs.size(); i++) {
        if(i > 0 && !(includeChar(opchar + "[{}]", vs[i - 1][0])) && vs[i][0] == '(') {
            string allParam = MIDDLE(vs[i]);
            result.add("^");
            result.add("[" + allParam + "]");
        } else result.add(vs[i]);
    }

    ret = joinString(result);
    return ret;
}

Value cloneRef(Value ref) {
	vector<Value> val;
	if(ref.type != T_REF) return ref;
	for(int i = 0; i < ARR_GROW; i++) {
		val.push_back(cloneRef(heap[ref.refVal + i]));
	}
	return makeArray(val);
}

Value execBlock(vector<string>);
void setVarRef(string, Value);
void registFunc(string func, vector<string> list, vector<string> ptype, vector<string> block);

void gc(Value v) {
	BEGINL
	bool canVisit[ARRCNT];
	memset(canVisit, false, sizeof(canVisit));
	for(map<string, Value>::iterator it = scope.begin(); it != scope.end(); it++) {
		Value val = it -> second;
		if(val.type == T_REF) {
			canVisit[val.refVal / ARR_GROW] = true;
		}
	}
	if(v.type == T_REF) canVisit[v.refVal / ARR_GROW] = true;
	vector<string> allIndex = usedLocalVar[usedLocalVar.size() - 1];
    for(int i = 0; i < allIndex.size(); i++) {
    	string local = callStack[callStack.size() - 1] + allIndex[i];
    	Value val = scope[local];
    	freeVariable(val);
    	scope.erase(scope.find(local));
	}
	for(int i = 0; i < ARRCNT; i++) {
		if(allIndexes[i] && !canVisit[i]) {
			freeVariable(Value::makeRef(i * ARR_GROW));
		}
	}
	lookVariable();
	ENDEDL
}

struct ExitException {
	Value val;
};

#ifdef WIN32
Value loadNativeFF0(string, string);
#endif

Value callFunc(Value func, vector<Value> allParameter, bool level) {
#define CHECK_TYPE(a, n) \
	if(allParameter[a].type != T_##n) RUN_ERR("Built in function " + func.funVal + " required a value of '" + typeName[T_##n] + "' for parameter " + parseNum(a) + ", given: " + typeName[allParameter[a].type], 12)
#define CHECK_ARG_CNT(n)\
    if(allParameter.size() != n) SYNTAX_ERR("Wrong argument count: " + parseNum(allParameter.size()) + " , need: " + parseNum(n), 11)
    if(func.type != T_FUNCTION) RUN_ERR(" can not call a non-function value, given: " + typeName[func.type], 10);

    if(func.funVal == "print") {
        for(int i = 0; i < allParameter.size(); i++) cout << (outputSym ? "" : "") + allParameter[i].toString() << ' ';
        cout << endl;
        return Value::makeNumber(allParameter.size());
    } else if(func.funVal == "readonly") {
        for(int i = 0; i < allParameter.size(); i++) protectVar.add(allParameter[i].toString());
        return Value::makeNumber(allParameter.size());
    } else if(func.funVal == "exit") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        ExitException exitEvent;
        exitEvent.val = allParameter[0];
        throw exitEvent;
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
        string str;
        cin >> str;
        return Value::makeString(str);
    } else if(func.funVal == "read_str_line") {
        string str;
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
        string s = v.toString();
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
    } else if(func.funVal == "set_local") {
        for(int i = 0; i < allParameter.size(); i++) {
        	setLocalVarVal(allParameter[i].strVal, Value::makeNull());
		}
        return Value::makeNumber(allParameter.size());
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
    } else if(func.funVal == "math_floor") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        Value v = allParameter[0];
        return Value::makeNumber(floor(v.numVal));
    } else if(func.funVal == "math_ceil") {
        CHECK_ARG_CNT(1);
        CHECK_TYPE(0, NUMBER);
        Value v = allParameter[0];
        return Value::makeNumber(ceil(v.numVal));
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
        vector<string> tmp;
        vector<string> EMP;
        vector<Value> EMV;
        for(int i = 0; i < allParameter.size(); i++) tmp.add(allParameter[i].toString());
        registFunc("<eval>", EMP, EMP, tmp);
        return callFunc(Value::makeFunction("<eval>"), EMV, true);
    } else if(func.funVal == "tostr") {
        vector<Value> arr;
        for(int i = 0; i < allParameter.size(); i++) {
            string r = "";
            r += (char)(allParameter[i].numVal);
            arr.add(Value::makeString(r));
        }
        return Value::makeRef(makeArray(arr).refVal);
    } else if(func.funVal == "toascii") {
        vector<Value> arr;
        for(int i = 0; i < allParameter.size(); i++) {
            Value v = allParameter[i];
            string str = v.toString();
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
	else if(func.funVal == "big") {
        CHECK_ARG_CNT(1);
    	CHECK_TYPE(0, STRING);
    	return Value::makeBigInteger(allParameter[0].strVal);
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
		string key = allParameter[0].strVal;
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
	
	else if(func.funVal == "load_dll_func") {
		#ifdef WIN32
		CHECK_ARG_CNT(2);
		CHECK_TYPE(0, STRING); CHECK_TYPE(1, STRING);
		return loadNativeFF0(allParameter[0].strVal, allParameter[1].strVal);
		#endif
		return Value::makeFunction("print");
	}
	else if(func.funVal == "clone") {
		CHECK_ARG_CNT(1);
		CHECK_TYPE(0, REF);
		return cloneRef(allParameter[0]);
	}
	
	for(int i = 0; i < nativeFunc.size(); i++) if(nativeNames[i] == func.funVal) return nativeFunc[i](allParameter);
	
    vector<string> list = paramTable[func.funVal];
    vector<string> block = funcTable[func.funVal];
    vector<string> types = ptypeTable[func.funVal];


    Value returnVal = Value::makeNull();
    string frame;
	if(func.funVal == "<eval>") frame = "000";
	else frame = makeName();
	
	vector<string> indexTable;
	usedLocalVar.add(indexTable); 
    callStack.add(frame + func.funVal);

    if(callStack.size() > INT_MAX) RUN_ERR("Stack overflow. To much recursive function!", 13);
	
	if(list.size() != allParameter.size()) RUN_ERR("Function '" + func.funVal + "' required " + parseNum(list.size()) + " parameter(s), but received " + parseNum(allParameter.size()) + ".", 12);

    for(int i = 0; i < list.size(); i++) {
    	if(types[i] != "<unlimited>") {
    		if(types[i] != typeName[allParameter[i].type]) RUN_ERR("Function '" + func.funVal + "' required '" + types[i] + "' for parameter " + parseNum(i + 1) + ", but received '" + typeName[allParameter[i].type] + "'.", 12);
		}
        setLocalVarVal(trim(list[i]), allParameter[i]);
    }

    if(level) {
        try {
            returnVal = execBlock(block);
        }
        catch(Value retException) {
            returnVal = retException; 
            gc(returnVal);
            
   			callStack.pop_back();
   			usedLocalVar.pop_back();
            return returnVal;
        }
    }
    else {
        try {
        	returnVal = execBlock(block);
		}
		catch(Value retException) {
			gc(retException);
   			callStack.pop_back();
   			usedLocalVar.pop_back();
			throw retException;
		}
    }

    gc(returnVal);
    callStack.pop_back();
    usedLocalVar.pop_back();
    return returnVal;
}

string stringExpr(string str) {
    string result = "";
    for(int i = 0; i < str.length(); i++) {
        if(str[i] == '\\') {
            i++;
            if(i > str.length()) RUN_ERR(" need an escape character after '\\'.", 14);
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
                    RUN_ERR(msg, 14);
                    break;
                }
            }
        } else result += str[i];
    }
    return result;
}

double toHex(string s) {
	int result = 0;
	int parsing[256];
	for(int i = 0; i <= 9; i++) parsing['0' + i] = i;
	for(int i = 'a'; i <= 'f'; i++) parsing[i] = i - 'a' + 10;
	for(int i = 2; i < s.length(); i++) result = result * 16 + parsing[tolower(s[i])];
	return result;
}

Value evalSimpleExpr(string s) {
    if(s[0] == '\"') return Value::makeString(stringExpr(MIDDLE(s)));
    else if(s[0] == '\'') {
    	return Value::makeString(stringExpr(MIDDLE(s)));
	}
    else if(isdigit(s[0])) {
    	if(s[0] == '0' && s[1] == 'x') return Value::makeNumber(toHex(s));
    	if(s.length() >= 13) return Value::makeBigInteger(s.c_str());
    	double d = atof(s.c_str());
    	if(d > MAX_NUMBER) {
    		return Value::makeBigInteger(parseNum(d).c_str());
		}
		else return Value::makeNumber(d);
	}
    else return getVarVal(s);
}

Value makeLambda(string, string);

Value evalExpr(string expr) {
	if(expr == ".") RUN_ERR("Shouldn't use the subscript like object.method().member. Did you forget 'object[method]()[member]'?", 24);
    BEGINL
    stack<Value> st;
    Value num, op1, op2;
    vector<string> vs;
    stack<string> stk;
    vector<string> str;
	
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

    for(int i = 0; i < vs.size(); i++) {
        string tmp = vs[i];
        #define CHECK() if(st.empty()) RUN_ERR("Invalid expression: " + expr, 15)
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
        } else if(vs[i] == ">>") {
            GET_OP();
            st.push(op1 >> op2);
        } else if(vs[i] == "<<") {
            GET_OP();
            st.push(op1 << op2);
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
            string parameter;
            CHECK();
            op2 = st.top();
            st.pop();
            parameter = op2.toString();
            CHECK();
            op1 = st.top();
            st.pop();

            vector<string> args = removeAll(split(parameter, ','), ",");

            vector<Value> vals;
            for(int i = 0; i < args.size(); i++) vals.add(evalExpr(args[i]));

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
                vector<string> args = removeAll(split(simpleExpr, ','), ",");
                vector<Value> vals;
                for(int i = 0; i < args.size(); i++) vals.add(evalExpr(args[i]));
                num = makeArray(vals);
            } else num = evalSimpleExpr(simpleExpr);
            st.push(num);
        }
    }
    if(st.size() != 1) SYNTAX_ERR("invalid expression: " + expr, 15);
    ENDEDL
    return st.top();
}

enum { SUB_SET, VAR_SET };

void setVarRef(string var, string exp) {
	setVarRef(var, evalExpr(exp));
}

void setVarRef(string var, Value exp) {
    BEGINL
    for(int i = 0; i < protectVar.size(); i++) {
    	if(var == protectVar[i]) RUN_ERR(" access denied: " + var + " can not be modified.", 16);
	}
	if(findchar(var, '.')) var = parseSubscript(var);
    stack<Value> st;
    Value num, op1, op2;
    vector<string> vs;
    stack<string> stk;
    vector<string> str;
    string expr = var;
    int type = VAR_SET;

    for(int i = 0; i < var.length(); i++) {
        if(var[i] == '[') type = SUB_SET;
    }

    if(type == VAR_SET) {
        Value v;
        v = exp;
        setVarVal(var, v);
        ENDEDL
        return;
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

    for(int i = 0; i < vs.size() - 1; i++) {
        string tmp = vs[i];
        if(vs[i] == "@") {
            op2 = st.top();
            st.pop();
            op1 = st.top();
            st.pop();
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
                vector<string> args = removeAll(split(simpleExpr, ','), ",");
                vector<Value> vals;
                for(int i = 0; i < args.size(); i++) vals.add(evalExpr(args[i]));
                num = makeArray(vals);
            } else num = evalSimpleExpr(simpleExpr);
            if(num.type == T_STRING) {
                num = Value::makeNumber(getHash(num.strVal));
            }
            st.push(num);
        }
    }

    op1 = st.top();
    st.pop();
    op2 = st.top();
    st.pop();
    Value result = Value::makeRef(op2.refVal + op1.numVal);
    st.push(result);

    Value v2 = exp;
    setRefVal(st.top(), v2);
    ENDEDL
}

void init() {
	vector<string> mainLocal;
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
    BIND(">>", 65, RIGHTSHIFT);
    BIND("<<", 65, LEFTSHIFT);
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
    setVarVal("math_floor", Value::makeFunction("math_floor"));
    setVarVal("math_ceil", Value::makeFunction("math_ceil"));
    setVarVal("random", Value::makeFunction("random"));
    setVarVal("set_seed", Value::makeFunction("set_seed"));
    setVarVal("eval", Value::makeFunction("eval"));
    setVarVal("toascii", Value::makeFunction("toascii"));
    setVarVal("tostr", Value::makeFunction("tostr"));
    setVarVal("xor", Value::makeFunction("xor"));
    setVarVal("clone", Value::makeFunction("clone"));
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
    setVarVal("set_local", Value::makeFunction("set_local"));
    setVarVal("get_async_key_state", Value::makeFunction("get_async_key_state"));
    setVarVal("load_dll_func", Value::makeFunction("load_dll_func"));
	setVarVal("big", Value::makeFunction("big"));    
    
	protectVar.add("null");
    protectVar.add("true");
    protectVar.add("false");
    protectVar.add("args");
}

enum { EXPR, ASSIGN };

Value execute(string stat) {
    BEGINL
    vector<string> vs = tokenize(stat);
    int i = 0;
    int type = EXPR;
    string flag = vs[0];

    bool inq = false;
	
	int x;
    for(int i = 0; i < vs.size(); i++) {
    	if(vs[i] == "=") {
    		type = ASSIGN, x = i;
    		break;
		}
	}
    string left;
    string right;
    
    if(type == ASSIGN) {
    	for(int i = 0; i < x; i++) left += vs[i];
   		for(int j = x + 1; j < vs.size(); j++) right += vs[j];
        setVarRef(left, right);

    } else if(flag == "stop") {
        STOP_EXEC(joinString(vs), 17);
    } else {
        evalExpr(stat);
    }
    ENDEDL
    return Value::makeNumber(0);
}

void registFunc(string func, vector<string> list, vector<string> ptype, vector<string> block) {
    funcTable[func] = block;
    paramTable[func] = list;
    ptypeTable[func] = ptype;
}

void readCodeFrom(string fileName) {
    ifstream fcin(fileName.c_str());
    string s;
    while(getline(fcin, s)) {
        if(s == "") continue;
        vector<string> vs = tokenize(s);
        if(vs[0] == "#") {
        	;
		}
        else if(vs[0] == "import") {
            string name = "";
            for(int i = 1; i < vs.size(); i++) name += vs[i];
            readCodeFrom(name);
        } else codeStream << s << endl;
    }
}

void readCode() {
    string s;
    cout << ">>> ";
    while(getline(cin, s)) {
        if(s == "") continue;
        vector<string> vs = tokenize(s);
        if(vs[0] == "#") {
        	;
		}
        else if(vs[0] == "import") {
            string name = "";
            for(int i = 1; i < vs.size(); i++) name += vs[i];
            readCodeFrom(name);
        } else codeStream << s << endl;
        cout << ">>> ";
    }
}

vector<string> tinyBlock(string stat) {
    vector<string> vs;
    vs.add(stat);
    return vs;
}

vector<string> makeBlock() {
    string s;
    vector<string> ret;
    BEGINL
    while(getline(codeStream, s)) {
        vector<string> vs;
        s = trim(s);
        vs = tokenize(s);
        if(s == "") continue;
        if(s == "}") break;
        if(vs[0] == "if") {
            string expr = MIDDLE(vs[1]);
            string stat = "if (";
            string rname = makeName();
            vector<string> block;
            stat += expr;
            stat += ") ";
            stat += rname;
            if(vs.size() >= 3 && vs[2][0] == '{') {
                block = makeBlock();
            }
            else {
                string statement;
                for(int i = 2; i < vs.size(); i++) statement += vs[i] + " ";
                block = tinyBlock(statement);
            }
            registFunc(rname, EMPTY, EMPTY, block);
            ret.add(stat);
        } else if(vs[0] == "while") {
            string expr = MIDDLE(vs[1]);
            string stat = "while (";
            string rname = makeName();
            vector<string> block;
            stat += expr;
            stat += ") ";
            stat += rname;
            if(vs.size() >= 3 && vs[2][0] == '{') {
                block = makeBlock();
            }
            else {
                string statement;
                for(int i = 2; i < vs.size(); i++) statement += vs[i] + " ";
                block = tinyBlock(statement);
            }
            registFunc(rname, EMPTY, EMPTY, block);
            ret.add(stat);
        } else if(vs[0] == "for") {
            string expr = MIDDLE(vs[1]);
            string stat = "for ";
            vector<string> condition = buildWith(split(expr, ','), ",");
            if(condition.size() != 7) SYNTAX_ERR("for statement syntax: for('name', expression, expression, expression) {block}", 18);

            for(int i = 0; i < 7; i++) {
                if(condition[i] == ",") continue;
                string tmp = "(";
                tmp += condition[i];
                tmp += ")";
                stat += tmp + " ";
            }

            string rname = makeName();
            vector<string> block;
            stat += rname;
            if(vs.size() >= 3 && vs[2][0] == '{') {
                block = makeBlock();
            }
            else {
                string statement;
                for(int i = 2; i < vs.size(); i++) statement += vs[i] + " ";
                block = tinyBlock(statement);
            }
            registFunc(rname, EMPTY, EMPTY, block);
            ret.add(stat);
        } else if(vs[0] == "foreach") {
            string expr = MIDDLE(vs[1]);
            string stat = "for ";
            vector<string> condition = tokenize(expr);
            if(condition.size() != 3) SYNTAX_ERR("foreach statement syntax: foreach('name' : expression) {block}", 18);

            string var = condition[0];
            string ref = condition[2];
            string it = makeName();
            
            stat += "(" + it + ") ";
            stat += "(0) ";
            stat += "(" + it + " < len(" + ref + ")) ";
            stat += "(1) ";

            string rname = makeName();
            vector<string> block;
            stat += rname;
            if(vs.size() >= 3 && vs[2][0] == '{') {
                block = makeBlock();
            }
            else {
                string statement;
                for(int i = 2; i < vs.size(); i++) statement += vs[i] + " ";
                block = tinyBlock(statement);
            }
            vector<string> fblock;
            fblock.push_back(var + " = " + ref + "[" + it + "]");
            for(int i = 0; i < block.size(); i++) fblock.push_back(block[i]);
            registFunc(rname, EMPTY, EMPTY, fblock);
            ret.add(stat);
        } else if(vs[0] == "function") {
            string name = vs[1];
            string expr = MIDDLE(vs[2]);
            vector<string> args = removeAll(buildWith(split(expr, ','), ","), ",");
            vector<string> finalArgs;
            vector<string> ptypeArgs;
            for(int i = 0; i < args.size(); i++) {
                if(args[i] != "") {
                	string src = args[i];
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

            string rname = name;
            vector<string> block = makeBlock();

            setVarRef(name, Value::makeFunction(rname));
            registFunc(rname, finalArgs, ptypeArgs, block);
        } else if(vs[0] == "else") {
            string stat = "else (";
            string rname = makeName();
            vector<string> block;
            stat += rname;
            stat += ")";
            if(vs.size() >= 2 && vs[1][0] == '{') {
                block = makeBlock();
            }
            else {
                string statement;
                for(int i = 1; i < vs.size(); i++) statement += vs[i] + " ";
                block = tinyBlock(statement);
            }
            registFunc(rname, EMPTY, EMPTY, block);
            ret.add(stat);
        } else if(vs[0] == "elseif") {
            string expr = MIDDLE(vs[1]);
            string stat = "elseif (";
            string rname = makeName();
            vector<string> block;
            stat += expr;
            stat += ") ";
            stat += rname;
            if(vs.size() >= 3 && vs[2][0] == '{') {
                block = makeBlock();
            }
            else {
                string statement;
                for(int i = 2; i < vs.size(); i++) statement += vs[i] + " ";
                block = tinyBlock(statement);
            }
            registFunc(rname, EMPTY, EMPTY, block);
            ret.add(stat);
        } else if(vs[0] == "local") {
        	string sugar = "set_local(";
        	vector<string> localVars;
        	for(int i = 1; i < vs.size(); i += 2) {
        		localVars.push_back(vs[i]);
			}
			for(int i = 0; i < localVars.size() - 1; i++) sugar += "\"" + localVars[i] + "\",";
			sugar += "\"" + localVars[localVars.size() - 1] + "\")";
			ret.add(sugar);
		}
        else {
			string res = "";
        	for(int i = 0; i < vs.size(); i++) {
        		if(vs[i] == "function") {
        			if(vs.size() <= i + 1) RUN_ERR("invalid function definition. Must be 'function (param-list) {'.", 22);
		            string expr = MIDDLE(vs[i + 1]);
		            if(vs.size() <= i + 2) RUN_ERR("invalid function definition. Where is your '{' at the end of the line?", 23);
		            vector<string> args = removeAll(buildWith(split(expr, ','), ","), ",");
		            vector<string> finalArgs;
		            vector<string> ptypeArgs;
		            for(int i = 0; i < args.size(); i++) {
		                if(args[i] != "") {
		                    string src = args[i];
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
		
		            string rname = makeName();
		            vector<string> block = makeBlock();
		            
		            registFunc(rname, finalArgs, ptypeArgs, block);
		            res += rname;
		            break;
				}
				else res += vs[i] + ' ';
			}
			ret.add(res);
		}
    }
    ENDEDL
    return ret;
}

Value execBlock(vector<string> block);

Value execBlock(vector<string> block) {
    for(int index = 0; index < block.size(); index++) {
        currentCommand = block[index];
        vector<string> token = tokenize(trim(block[index]));
        vector<string> nextLine;
        bool hasNext = false;
        if(index < block.size() - 1) {
            nextLine = tokenize(trim(block[index + 1]));
            hasNext = true;
        }
        if(token[0] == "if") {
            string condition;
            vector<string> branches;
            vector<string> conditions;
            branches.add(token[2]);
            conditions.add(MIDDLE(token[1]));
        	index++;
            while(index < block.size() ) {
                vector<string> curLine = tokenize(block[index]);
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
            string condition = MIDDLE(token[1]);
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
            string variable = MIDDLE(token[1]);
            string from = MIDDLE(token[2]);
            string condition = MIDDLE(token[3]);
            Value step = evalExpr(MIDDLE(token[4]));
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
            string r = "";
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
        } 
		else {
			execute(trim(block[index]));
		}
    }
    return Value::makeNull();
}

string parseArg(string arg) {
    return arg.substr(1, arg.length());
}

void readByteCode(string bytefile) {
    ifstream fcin(bytefile.c_str());
    int funcCount;
    string temp;
    getline(fcin, temp);
    if(temp != "ff0") {
        RUN_ERR("Invalid bytecode file : " + bytefile, 19);
    }
    getline(fcin, temp);
    funcCount = atof(temp.c_str());
    for(int func = 0; func < funcCount; func++) {
        string funcName;
        vector<string> pList;
        vector<string> tList;
        vector<string> cmdList;
        int paramCnt;
        int lineCnt;
        getline(fcin, funcName);
        getline(fcin, temp);
        paramCnt = atof(temp.c_str());
        for(int i = 0; i < paramCnt; i++) {
            string pName;
            string finalP;
            string tName = "<unlimited>";
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
            string s;
            getline(fcin, s);
            cmdList.push_back(s);
        }
        registFunc(funcName, pList, tList, cmdList);
        setVarVal(funcName, Value::makeFunction(funcName));
    }
}

string codefileName;

string getOutputFile(string ext = ".ff0") {
    for(int i = codefileName.size() - 1; i >= 0; i--) {
        if(codefileName[i] == '.') {
            return codefileName.substr(0, i) + ext;
        }
    }

}

void compile() {
    ofstream fcout(getOutputFile().c_str());
    map<string, vector<string> >::iterator paramIt = paramTable.begin();
    map<string, vector<string> >::iterator ptypeIt = ptypeTable.begin(); 
    fcout << "ff0" << endl;
    fcout << funcTable.size() << endl;
    for(map<string, vector<string> >::iterator it = funcTable.begin(); it != funcTable.end(); it++) {
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
map<string, bool> uncomped;

string gettab(int x) {
	string s = "";
	for(int i = 0; i < x; i++) s += "    ";
	return s;
}

string uncompile(string func, int tabs) {
	stringstream result("");
	uncomped[func] = true;
	vector<string> block = funcTable[func];
	for(int index = 0; index < block.size(); index++) {
		currentCommand = block[index];
        vector<string> token = tokenize(trim(block[index]));
        vector<string> nextLine;
        bool hasNext = false;
        if(index < block.size() - 1) {
            nextLine = tokenize(trim(block[index + 1]));
            hasNext = true;
        }
        if(token[0] == "if") {
            string condition;
            vector<string> branches;
            vector<string> conditions;
            branches.add(token[2]);
            conditions.add(MIDDLE(token[1]));
        	index++;
            while(index < block.size() ) {
                vector<string> curLine = tokenize(block[index]);
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
            string condition = MIDDLE(token[1]);
            result << gettab(tabs) << "while(" << condition << ") {" << endl;
            result << uncompile(token[2], tabs + 1);
            result << gettab(tabs) << "}" << endl;
        } else if(token[0] == "for") {
            string variable = MIDDLE(token[1]);
            string from = MIDDLE(token[2]);
            string condition = MIDDLE(token[3]);
            Value step = evalExpr(MIDDLE(token[4]));
            result << gettab(tabs) << "for(var " << variable << " = " << from << "; ";
			result << condition << "; " << variable << " = " << variable << " + " << MIDDLE(token[4]) << ") {" << endl;
			result << uncompile(token[5], tabs + 1);
			result << gettab(tabs) << "}" << endl;
        } else {
        	string stat = block[index];
        	vector<string> vs = tokenize(stat);
		    int i = 0;
		    int type = EXPR;
		    string flag = vs[0];
		
		    bool inq = false;
		
		    for(i = 0; i < stat.length(); i++) {
		        if(i > 0 && stat[i - 1] != '\\' && stat[i] == '\"') inq = !inq;
		        if(stat[i] == '=' && !inq) {
		            type = ASSIGN;
		            break;
		        }
		    }
		
		    if(type == ASSIGN) {
		        string left = trim(stat.substr(0, i));
		        string right = trim(stat.substr(i + 1, stat.length()));
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

string javascript() {
	map<string, vector<string> >::iterator paramIt = paramTable.begin();
    for(map<string, vector<string> >::iterator it = funcTable.begin(); it != funcTable.end(); it++) {
        string fun = it -> first;
        if(fun == "<main>") {
        	paramIt++;
        	continue;
		}
        if(!uncomped[fun] && fun[0] != '~') {
        	jscout << "function " << fun << " ( ";
			for(int i = 0; i < paramIt -> second.size(); i++) {
				string s = paramIt -> second[i];
				for(int x = 0; x < s.length(); x++) {
					if(s[x] == ':') {
						jscout << s.substr(0, x - 1);
						break;
					}
				}
				jscout << paramIt -> second[i] << ",";
			}
			string cur = jscout.str();
			cur[cur.length() - 1] = ')';
			jscout.str(""); jscout << cur;
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

Value makeLambda(string param, string ret) {
	ret = MIDDLE(ret);
    string expr = MIDDLE(param);
    vector<string> args = removeAll(buildWith(split(expr, ','), ","), ",");
    vector<string> finalArgs;
    vector<string> empty;
    for(int i = 0; i < args.size(); i++) {
        if(args[i] != "") finalArgs.add(args[i]);
    }
    
    string rname = makeName();
    vector<string> block;
    block.add("return " + ret);
	
	Value retval = Value::makeFunction(rname);

    registFunc(rname, finalArgs, empty, block);
    return retval;
}

void parsePseudocode() {
    string realcode = PseudocodeParser().main(codeStream.str());
    codeStream.str("");
    codeStream << realcode << endl;
}

#ifdef WIN32
Value loadNativeFF0(string nativeName, string func) {
	stringstream ret("");
	HINSTANCE libaddr = LoadLibrary((LPCSTR)nativeName.c_str());
	ret << libaddr << '@';
	if(!libaddr) RUN_ERR("Can not found dll lib: " + nativeName, 21);
	NativeFF0 ff0 = (NativeFF0)GetProcAddress(libaddr, (LPCSTR)func.c_str());
	ret << ff0;
	if(!ff0) RUN_ERR("Can not found the native function " + func, 22);
	nativeNames.push_back(func);
	nativeFunc.push_back(ff0);
	
	Value f = Value::makeFunction(func);
	setVarVal(func, f);
	return f;
}
#endif

#ifndef FF0DLL
int main(int argc, char ** argv) {
	int retval = 0;
    init();
    initUncompile();
    if(mAutoUpdate) checkUpdate();
    vector<Value> args;
    if(argc == 1) {
        cout << VERSION << endl;
        readCode();
        if(mParse) parsePseudocode();
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
            if(argc >= 3) {
            	if(argv[2][0] == '-') {
            		string todo = parseArg(argv[2]);
                    for(int i = 0; i < todo.length(); i++) {
                        if(todo[i] == 'c') mCompile = true;
                        if(todo[i] == 'r') mExecute = true;
                        if(todo[i] == 'j') mJavaScript = true;
                        if(todo[i] == 'p') mParse = true;
                    }
                    for(int i = 3; i < argc; i++) args.push_back(Value::makeString(argv[i]));
                    setVarVal("args", makeArray(args));
				}
				else {
					for(int i = 2; i < argc; i++) args.push_back(Value::makeString(argv[i]));
                    setVarVal("args", makeArray(args));
				}
			}
        }
    }
    try {
        if(mCompile) {
            try {
                readCodeFrom(argv[1]);
                if(mParse) parsePseudocode();
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
            } catch(ExitException ex) {
            	retval = ex.val.numVal;
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
    return retval;
}
#endif
