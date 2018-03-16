#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<sstream>
#include<stdlib.h>
#define MAX 200

using namespace std;

//输入输出文件
string in;
ifstream infile;
ofstream outfile("E:/result.txt", ios::out);
ofstream mips("E:/mips.txt",ios::out);
ofstream mips2("E:/mips2.txt",ios::out);
ofstream out_four("E:/four.txt", ios::out);
ofstream out_statement("E:/statement.txt",ios::out);
ofstream out_four2("E:/four2.txt",ios::out);

//词法分析
string keyword[13]={"const","int","char","void","main","if","do",
	"while","switch","case","scanf","printf","return"};
string keyword_symbol[13]={"CONSTSY","INTSY","CHARSY","VOIDSY","MAINSY",
	"IFSY","DOSY","WHILESY","SWITCHSY","CASESY","SCANFSY","PRINTFSY","RETURNSY"};


char token[200]={};	//存放单词的字符串
int num;			//存放当前读入的整数值
char ch;			//存放当前读进的字符
int cnt = 1;		//单词计数器
string sym;			//取词记忆符（getsym中使用）
string str;			//具体取词（getsym中使用）

//回溯存词
struct Temp{
	string Sym;		//记忆符
	string strr;	//具体单词
};
Temp temp[100];		//回溯存储
int temp_index = 0;

struct symTable{
	string name;	
	int kind;		//const-0, var-1, func-2, para-3, array-4, 临时寄存器-5
	int type;		//int-0,char-1;  func: int-0, char-1, void-2;
 	int value;		//常量的值;
	int paranum;	//函数参数个数或数组大小
	int level;		//层数
	int address;	//相对位置
};
symTable symtab[MAX];
int stindex = 0;

string name;
int type;
int paranum;
int level;
int address = 0;
int funcvalid = 0;	//函数参数和函数中变量存进符号表-0， 不存-1

void insert_table(string name, int kind, int type, int value, int paranum, int level){
		symtab[stindex].name = name;
		symtab[stindex].kind = kind;
		symtab[stindex].type = type;
		symtab[stindex].value = value;
		symtab[stindex].paranum = paranum;
		symtab[stindex].level = level;
		symtab[stindex].address = address;
		stindex++;
}
bool find_table(string name,int level){
	if(level == 0){
		for(int i = 0; i < stindex; i++){
			if(symtab[i].name == name && symtab[i].level == level){
				return true;
			}
		}
	}else if(level == 1){
		for(int j = stindex-1; j >= 0; j--){
			if(symtab[j].level == 0)
				break;
			else if(symtab[j].name == name && symtab[j].level == level)
				return true;
		}
	}
	return false;
}

//四元式
struct intermediate_code{
	string op;
	string dst;
	string src1;
	string src2;
};
vector<intermediate_code> four;	//四元式存储容器

string fourvalue;	//全局变量，传递四元式的数据
int tempc = 0;	//临时变量计数器
int tempci = 0; //临时变量存储下标计数器
int strcount = 0;	//打印的str下标计数器
int line = 1;		//行数

void insert_four(string op, string dst, string src1, string src2){
	intermediate_code itm;
	itm.op = op;
	itm.dst = dst;
	itm.src1 = src1;
	itm.src2 = src2;
	four.push_back(itm);
}

string tostring(int num){
	stringstream ss;
	ss << num;
	return ss.str();
}

int toint(string s){
	int result;
	stringstream ss;
	ss << s;
	ss >> result;
	return result;
}

string temp_array[MAX];
//生成新的临时变量寄存器
string get_temp(){
	string s;
	s = "t" + tostring(tempc);
	temp_array[tempci] = s;		//临时变量存储,函数运行结束之后出栈
	tempc++;
	tempci++;
	return s;
}

void clear_temp(string funcname){
	for(int i = tempci - 1; ; i--){
		if(temp_array[i] == funcname){
			temp_array[i] = "";
			tempci --;
			break;
		}else{
			temp_array[i] = "";
			tempc --;
			tempci --;
		}
	}
}

int switchc = 0;	//switch计数器
int labc = 0;		//label计数器
int nofactor = 0;	
string new_label(){
	string s;
	s = "label_" + tostring(labc);
	labc++;
	return s;
}

void set_label(string label){
	insert_four(label,"","","");
}

string casevalue1;	//传递switch的值
string casevalue2;  //传递case的值

string function;	//函数名字
int returnflag = 0;		//标记函数中有没有返回语句 有 1， 无 0

string fooname[100];	//存储函数的名字
int foocount = 0;		//计算函数的个数

void error();
void NUM();
void const_def();
void const_statement();
void var_def();
void var_statement();
void expr();
void term();
void factor();
void returnable_statement();
void void_statement();
void parameter_list();
void compound_statement();
void mian_statement();
void assign_statement();
void condition_statement();
void condition();
void do_while_statement();
void switch_statement();
void case_list(string caselabel);
void case_statement(string caselabel);
void call_returnable_statement();
void call_void_statement();
void value_table(string funcname);
void statement_list();
void statement();
void read_statement();
void write_statement();
void return_statement();


bool isSpace(char ch){
	if(ch == ' ')
		return true;
	return false;
}

bool isNewLine(char ch){
	if(ch == '\n'){
		line++;
		return true;
	}
	return false;
}

bool isTab(char ch){
	if(ch == '\t')
		return true;
	return false;
}

bool isDigit(char ch){
	if(ch >= '0' && ch <= '9')
		return true;
	return false;
}

bool isLetter(char ch){
	if(ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >='A' && ch <= 'Z'))
		return true;
	return false;
}

void clearToken(char token[]){
	for(int i = 0; token[i] != NULL; i++)
		token[i] = NULL;
}

void catToken(char token[],char ch){
	int i;
	for(i = 0; token[i] != NULL; i++){
		;
	}
	token[i] = ch;
}

void toLow(char token[]){
	for(int i = 0; token[i] != NULL; i++){
		if(token[i] >= 'A' && token[i] <= 'Z')
			token[i] = token[i] + 32;
	}
}

int reserver(char token[]){
	int i;
	string str;
	toLow(token);
	for(int j = 0; token[j] != NULL; j++){
		str += token[j];
	}
	for(i = 0; i < 13; i++){
		if(str == keyword[i])
			return i;
	}
	return -1;	//不是保留字
}

int transNUM(char token[]){
	int num = 0;
	for(int i = 0; token[i] != NULL; i++){
		num = num*10 + (token[i] - 48);
	}
	return num;
}

void getsym(){
	clearToken(token);
	while(isSpace(ch) || isNewLine(ch) || isTab(ch)){
		ch = infile.get();
	}//跳过空格，空行，Tab
	if(isLetter(ch)){
		str = "";
		while(isLetter(ch) || isDigit(ch)){
			catToken(token,ch);
			ch = infile.get();
		}
		int resultValue = reserver(token);
		if(resultValue == -1){
			toLow(token);
			for(int j = 0; token[j] != NULL; j++){
				str += token[j];
			}
			sym = "IDSY";
			outfile << cnt << " IDSY "<< token << endl;
		}else{
			sym = keyword_symbol[resultValue];
			for(int j = 0; token[j] != NULL; j++){
				str += token[j];
			}
			outfile << cnt << " " << keyword_symbol[resultValue] << " " << token << endl;
		}
	}else if(isDigit(ch)){
		if(ch == '0'){
			ch = infile.get();
			if(ch >='0' && ch <='9'){
				cout << line << " " << "出现前0" << endl;
			}
		}
		while(isDigit(ch)){
			catToken(token,ch);
			ch = infile.get();
		}
		num = transNUM(token);
		sym = "INTEGERSY";
		str = tostring(num);
		outfile << cnt << " INTEGERSY " << num << endl;
	}else if(ch == '+'){
		sym = "PLUSSY";
		str = "+";
		outfile << cnt << " PLUSSY " << "+" << endl;
		ch = infile.get();
	}else if(ch == '-'){
		sym = "MINUSSY";
		str = "-";
		outfile << cnt << " MINUSSY " << "-" << endl;
		ch = infile.get();
	}else if(ch == '/'){
		sym = "DIVSY";
		str = "/";
		outfile << cnt << " DIVSY " << "/" << endl;
		ch = infile.get();
	}else if(ch == '*'){
		sym = "STARSY";
		str = "*";
		outfile << cnt << " STARSY " << "*" << endl;
		ch = infile.get();
	}else if(ch == ','){
		sym = "COMMASY";
		str = ",";
		outfile << cnt << " COMMASY " << "," << endl;
		ch = infile.get();
	}else if(ch == ';'){
		sym = "SEMISY";
		str = ";";
		outfile << cnt << " SEMISY " << ";" << endl;
		ch = infile.get();
	}else if(ch == ':'){
		sym = "COLONSY";
		str = ":";
		outfile << cnt << " COLONSY " << ":" << endl;
		ch = infile.get();
	}else if(ch == '('){
		sym = "LPARSY";
		str = "(";
		outfile << cnt << " LPARSY " << "(" << endl;
		ch = infile.get();
	}else if(ch == ')'){
		sym = "RPARSY";
		str = ")";
		outfile << cnt << " RPARSY " << ")" << endl;
		ch = infile.get();
	}else if(ch == '['){
		sym = "LEBARSY";
		str = "[";
		outfile << cnt << " LEBARSY " << "[" << endl;
		ch = infile.get();
	}else if(ch == ']'){
		sym = "REBARSY";
		str = "]";
		outfile << cnt << " REBARSY " << "]" << endl;
		ch = infile.get();
	}else if(ch == '{'){
		sym = "LBARSY";
		str = "{";
		outfile << cnt << " LBARSY " << "{" << endl;
		ch = infile.get();
	}else if(ch == '}'){
		sym = "RBARSY";
		str = "}";
		outfile << cnt << " RBARSY " << "}" << endl;
		ch = infile.get();
	}else if(ch == '<'){
		char ch2 = infile.get();
		if(ch2 == '='){
			sym = "LOESY";
			str = "<=";
			outfile << cnt << " LOESY " << "<=" << endl;
			ch = infile.get();
		}else{
			sym = "LESSSY";
			str = "<";
			outfile << cnt << " LESSSY " << "<" << endl;
			ch = ch2;
		}
	}else if(ch == '>'){
		char ch2 = infile.get();
		if(ch2 == '='){
			sym = "GOESY";
			str = ">=";
			outfile << cnt << " GOESY " << ">=" << endl;
			ch = infile.get();
		}else{
			sym = "GREATERSY";
			str = ">";
			outfile << cnt << " GREATERSY " << ">" << endl;
			ch = ch2;
		}
	}else if(ch == '!'){
		char ch2 = infile.get();
		if(ch2 == '='){
			sym = "UNEQUSY";
			str = "!=";
			outfile << cnt << " UNEQUSY " << "!=" << endl;
			ch = infile.get();
		}else{
			outfile << "error!" << endl;
		}
	}else if(ch == '='){
		char ch2 = infile.get();
		if(ch2 == '='){
			sym = "EQUSY";
			str = "==";
			outfile << cnt << " EQUSY " << "==" << endl;
			ch = infile.get();
		}else{
			sym = "ASSIGNSY";
			str = "=";
			outfile << cnt << " ASSIGNSY " << "=" << endl;
			ch = ch2;
		}
	}else if(ch == '\''){
		ch = infile.get();
		if(ch == '+' || ch == '-' || ch == '*' || ch == '/' || (ch >= 'A' && ch <= 'Z') || ch == '_'
			|| (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')){
			catToken(token,ch);
			ch = infile.get();
			if(ch == '\''){
				str = "";
				sym = "CHARACTERSY";
				for(int j = 0; token[j] != NULL; j++){
					str += token[j];
				}
				outfile << cnt << " CHARACTERSY " << "\'" << token << "\'" << endl;
			}else{
				outfile << "error! 词法分析:字符定义错误!" << endl;
				cout << line << " " << "error! 词法分析:字符定义错误!" << endl;
				while(ch != '\'')
					ch = infile.get();
			}
		}else{
			outfile << "error! 词法分析:字符定义错误!" << endl;
			cout << line << " " << "error! 词法分析:字符定义错误!" << endl;
			while(ch != '\'')
				ch = infile.get();
		}
		ch = infile.get();
	}else if(ch == '\"'){
		ch = infile.get();
		while(ch == 32 || ch == 33 || (ch >= 35 && ch <= 126)){
			catToken(token,ch);
			ch = infile.get();
		}
		if(ch == '\"'){
			str = "";
			sym = "STRINGSY";
			for(int j = 0; token[j] != NULL; j++){
				str += token[j];
			}
			outfile << cnt << " STRINGSY " << "\"" << token << "\"" << endl;
			ch = infile.get();
		}else{
			outfile << "error! 词法分析:字符串定义错误!" << endl;
			cout << line << " " << "error! 词法分析:字符串定义错误!" << endl;
		}
	}else{
		if(ch != EOF){
			cout << line << " " << "error! 非法字符" << endl;
			ch = infile.get();
			do{
				getsym();
			}while(sym != "VOIDSY" && sym != "INTSY" || sym != "CHARSY");
		}
	}
}


void error(int errflag){
	if(errflag == 1){
		cout << line << " " << "error! 不符合常量定义格式" << endl;
		while(str != "," && str != ";"){
			cnt++;
			getsym();
		}
	}else if(errflag == 2){
		cout << line << " " << "error! 不符合变量定义格式" << endl;
		while(str != "," && str != ";"){
			cnt++;
			getsym();
		}
	}else if(errflag == 3){				
		cout << line << " " << "error! 函数结构错误" << endl;
		while(str != "}"){
			cnt++;
			getsym();
		}
		cnt++;
		getsym();
		if(str != "int" || str != "char" || str != "void"){
			while(str != "int" && str != "char" && str != "void"){
				cnt++;
				getsym();
			}
		}
	}else if(errflag == 4){
		cout << line << " " << "error! 不符合参数表格式" << endl;
		while(str != "," && sym != ")"){
			cnt++;
			getsym();
		}
	}else if(errflag == 5){
		cout << line << " " << "error! 不能给常量赋值" << endl;
		while(str != ";"){
			cnt++;
			getsym();
		}
	}else if(errflag == 6){
		cout << line << " " << "error! 标识符未定义" << endl;
		while(str != ";" ){
			cnt++;
			getsym();
		}
	}else if(errflag == 7){
		cout << line << " " << "error! 不能给数组整体直接赋值" << endl;
		while(str != ";" ){
			cnt++;
			getsym();
		}
	}else if(errflag == 8){
		cout << line << " " << "error! 标识符重复定义" << endl;
		while(str != "," && str != ";"){
			cnt++;
			getsym();
		}
	}else if(errflag == 9){
		cout << line << " " << "error! 数组下标为负" << endl;
		while(str != ";"){
			cnt++;
			getsym();
		}
	}else if(errflag == 10){
		cout << line << " " << "error! 调用的函数不存在" << endl;
		while(str != ";"){
			cnt++;
			getsym();
		}
	}
}

void NUM(){
	string op;
	if(sym == "INTGERSY"){
		sym = "NUMSY";
	}
/*	while(sym == "PLUSSY" || sym == "MINUSSY"){
		cnt++;
		getsym();
		if(sym == "INTEGERSY"){
			if(num == 0){
				cout << line << " " << "0前面不允许有正负号" << endl;
			}else{
				sym = "NUMSY";
			}
		}
	}*/
	if(sym == "PLUSSY" || sym == "MINUSSY"){
		op = sym;
		cnt++;
		getsym();
		if(sym == "INTEGERSY"){
			if(num == 0){
				cout << line << " " << "0前面不允许有正负号" << endl;
			}else{
				if(op == "MINUSSY")	num = 0 - num;
				sym = "NUMSY";
			}
		}else if(sym == "PLUSSY" || sym == "MINUSSY"){
			op == sym;
			cnt++;
			getsym();
			if(sym == "INTEGERSY"){
				if(num == 0)	cout << line << " " << "0前面不允许有正负号" << endl;
				else{
					if(op == "MINUSSY")	num = 0 - num;
					sym = "NUMSY";
				}
			}
		}
	}
}

void const_def(){
	if(sym == "INTSY"){
		cnt++;
		getsym();
		if(sym == "IDSY"){
			name = str;
			cnt++;
			getsym();
			if(sym == "ASSIGNSY"){
				cnt++;
				getsym();
				if(str != "+" && str != "-" && (str[0] < '0'|| str[0] > '9')){
					cout << line << " " << name << " 常量定义右侧类型错误" << endl;
				}
				NUM();
				cnt++;
				getsym();
				if(funcvalid == 0 && find_table(name,level) == false){
					insert_table(name,0,0,num,-1,level);
					address = address + 4;
				}else if(funcvalid == 0)
					error(8);
				
			}else{
				error(1);
			}
		}else{
			error(1);
		}
		
		while(sym == "COMMASY"){
			cnt++;
			getsym();
			if(sym == "IDSY"){
				name = str;
				cnt++;
				getsym();
				if(sym == "ASSIGNSY"){
					cnt++;
					getsym();
					if(str != "+" && str != "-" && (str[0] < '0'|| str[0] > '9')){
						cout << line << " " << name << " 常量定义右侧类型错误" << endl;
					}
					NUM();
					cnt++;
					getsym();
					if(funcvalid == 0 && find_table(name,level) == false){
						insert_table(name,0,0,num,-1,level);
						address = address + 4;
					}else if(funcvalid == 0)
						error(8);
				}else{
					error(1);
				}
			}else{
				error(1);
			}
		}
		
	}else if(sym == "CHARSY"){
		cnt++;
		getsym();
		if(sym == "IDSY"){
			name = str;
			cnt++;
			getsym();
			if(sym == "ASSIGNSY"){
				cnt++;
				getsym();
				if(sym != "CHARACTERSY"){
					cout << line << " " << name << " 常量定义右侧类型错误" << endl;
					while(str != "," && str != ";"){
						cnt++;
						getsym();
					}
				}else{
					if(funcvalid == 0 && find_table(name,level) == false){
						insert_table(name,0,1,token[0],-1,level);
						address = address + 4;
					}else if(funcvalid == 0)
						error(8);
					cnt++;
					getsym();
				}
			}else{
				error(1);
			}
		}else{
			error(1);
		}

		while(sym == "COMMASY"){
			cnt++;
			getsym();
			if(sym == "IDSY"){
				name = str;
				cnt++;
				getsym();
				if(sym == "ASSIGNSY"){
					cnt++;
					getsym();
					if(sym != "CHARACTERSY"){
						cout << line << " " << name << " 常量定义右侧类型错误" << endl;
						while(str != "," && str != ";"){
							cnt++;
							getsym();
						}
					}else{
						if(funcvalid == 0 && find_table(name,level) == false){
							insert_table(name,0,1,token[0],-1,level);
							address = address + 4;
						}else if(funcvalid == 0)
							error(8);
						cnt++;
						getsym();
						
					}
				}else{
					error(1);
				}
			}else{
				error(1);
			}
		}
		
	}else{
		error(1);
	}
//	cout << line << " " << "This is constant defination!" << endl;
}

 void const_statement(){
	while(sym == "CONSTSY"){
		cnt++;
		getsym();
		const_def();
		if(sym != "SEMISY"){
			cout << line << " " << "error! without semi!" << endl;
			while(str != "const" && str != "int" && str != "char" && str != "void"){
				cnt++;
				getsym();
			}	//常量定义不是以分号结尾，读到下一个有效关键字出现
		}else{
			out_statement << "This is a constant statement!" << endl;
			cnt++;
			getsym();
		}
	}		
}

void var_def(){
	if(temp_index != 0){
		sym = temp[0].Sym;
		str = temp[0].strr;
		if(sym == "INTSY" || sym == "CHARSY"){
			if(sym == "INTSY")	type = 0;
			else	type = 1;	//符号表中int和char种类区分

			sym = temp[1].Sym;
			str = temp[1].strr;
			if(sym != "IDSY"){
				error(2);
			}else{
				name = str;
				paranum = -1;
				sym = temp[2].Sym;
				str = temp[2].strr;
				if(sym == "LEBARSY"){
					cnt++;
					getsym();
					if(sym == "PLUSSY"){
						cnt++;
						getsym();
					}
					if(sym == "INTEGERSY" && num != 0){
						paranum = num;
						cnt++;
						getsym();
						if(sym != "REBARSY"){
							error(2);
						}else{
							cnt++;
							getsym();
							if(sym == "COMMASY" || sym == "SEMISY"){
								if(funcvalid == 0 && find_table(name, level) == false){
									insert_table(name,4,type,-1,paranum,level);
									address = address + 4 * paranum;
								}else if(funcvalid == 0)
									error(8);
							}else{
								error(2);
							}
						}
					}else{
						error(2);
					}
				}else if(sym == "COMMASY" || sym == "SEMISY"){
					if(funcvalid == 0 && find_table(name,level) == false){
						insert_table(name,1,type,-1,paranum,level);
						address = address + 4;
					}else if(funcvalid == 0)
						error(8);
				}else{
					error(2);
				}
			}
		}
	}else{
		if(sym == "INTSY" || sym == "CHARSY"){
			if(sym == "INTSY"){
				type = 0;
			}else{
				type = 1;
			}	//区分符号中int和char的种类
			cnt++;
			getsym();
			if(sym != "IDSY"){
				error(2);
			}else{
				name = str;
				paranum = -1;
				cnt++;
				getsym();
				if(sym == "LEBARSY"){
					cnt++;
					getsym();
					if(sym == "PLUSSY"){
						cnt++;
						getsym();
					}
					if(sym == "INTEGERSY" && num != 0){
						paranum = num;
						cnt++;
						getsym();
						if(sym != "REBARSY"){
							error(2);
						}else{
							cnt++;
							getsym();
							if(sym == "COMMASY" || sym == "SEMISY"){
								if(funcvalid == 0 && find_table(name,level) == false){
									insert_table(name,4,type,-1,paranum,level);
									address = address + 4 * paranum;
								}else if(funcvalid == 0)
									error(8);
							}else{
								error(2);
							}
						}
					}else{
						error(2);
					}
				}else if(sym == "COMMASY" || sym == "SEMISY"){
					if(funcvalid == 0 && find_table(name,level) == false){
						insert_table(name,1,type,-1,paranum,level);
						address = address + 4;
					}else if(funcvalid == 0)
						error(8);
				}else{
					error(2);
				}
			}
		}
	}

	while(sym == "COMMASY"){
		cnt++;
		getsym();
		if(sym != "IDSY"){
			error(2);
		}else{
			name = str;
			paranum = -1;
			cnt++;
			getsym();
			if(sym == "LEBARSY"){
				cnt++;
				getsym();
				if(sym == "PLUSSY"){
					cnt++;
					getsym();
				}
				if(sym == "INTEGERSY" && num != 0){
					paranum = num;
					cnt++;
					getsym();
					if(sym != "REBARSY"){
						error(2);
					}else{
						cnt++;
						getsym();
						if(funcvalid == 0 && find_table(name,level) == false){
							insert_table(name,4,type,-1,paranum,level);
							address = address + 4 * paranum;
						}else if(funcvalid == 0)
							error(8);
					}
				}else{
					error(2);
				}
			}else if(sym == "COMMASY" || sym == "SEMISY"){ 
				if(funcvalid == 0 && find_table(name,level) == false){
					insert_table(name,1,type,-1,paranum,level);
					address = address + 4;
				}else if(funcvalid == 0)
					error(8);
			}else{
				error(2);
			}
		}
	}

	for(int j = 0; j < temp_index; j++){
		temp[j].strr = "";
		temp[j].Sym = "";
	}
	temp_index = 0;
}

void var_statement(){
	var_def();
	if(sym != "SEMISY"){
		cout << line << " " << "error! without semi!" << endl;
		while(str != "int" && str != "char" && str != "void"){
			cnt++;
			getsym();
		}
	}else{
		out_statement << "This is a variable statement!" << endl;
		for(int j = 0; j < temp_index; j++){
			temp[j].strr = "";
			temp[j].Sym = "";
		}
		temp_index = 0;
		cnt++;
		getsym();	
	}

	while(sym == "INTSY" || sym == "CHARSY"){
		temp_index = 0;
		temp[temp_index].Sym = sym;
		temp[temp_index].strr = str;
		temp_index++;
		cnt++;
		getsym();
		temp[temp_index].Sym = sym;
		temp[temp_index].strr = str;
		temp_index++;
			
		cnt++;
		getsym();
		temp[temp_index].Sym = sym;
		temp[temp_index].strr = str;
		temp_index++;
		if(sym == "LPARSY"){
			return;
		}else{
			var_def();
			if(sym != "SEMISY"){
				cout << line << " " << "error! without semi!" << endl;
				while(str != "int" && str != "char"){
					cnt++;
					getsym();
				}
			}else{
				cnt++;
				getsym();
				out_statement << "This is a variable statement!" << endl;
			}
		}
	}
}

void expr(){
	string str1,str2,str3;
	if(sym == "PLUSSY" || sym == "MINUSSY"){	//有+ | -的情况
		if(sym == "PLUSSY"){
			cnt++;
			getsym();
			term();
			str3 = fourvalue;
		}else if(sym == "MINUSSY"){
			cnt++;
			getsym();
			term();
			str2 = fourvalue;
			str3 = get_temp();
			insert_four("-",str3,"0",str2);		// 例：-9 => - t1 0 9
			insert_table(str3,5,0,-1,-1,1);
			address = address + 4;
		}
	}else{
		term();
		str3 = fourvalue;
	}

	while(sym == "PLUSSY" || sym == "MINUSSY"){	
		str1 = str3;	//src1==前一个的dst
		if(sym == "PLUSSY"){
			cnt++;
			getsym();
			term();
			str2 = fourvalue;	//生成src2
			str3 = get_temp();	//生成新的中间变量
			insert_four("+",str3,str1,str2);
			insert_table(str3,5,0,-1,-1,1);
			address = address + 4;
		}else if(sym == "MINUSSY"){
			cnt++;
			getsym();
			term();
			str2 = fourvalue;	//生成src2
			str3 = get_temp();	//生成新的中间变量
			insert_four("-",str3,str1,str2);
			insert_table(str3,5,0,-1,-1,1);
			address = address + 4;
		}	
	}
	fourvalue = str3;
}

void term(){
	string str1,str2,str3;

	factor();
	str3 = fourvalue;
	while(sym == "STARSY" || sym == "DIVSY"){
		str1 = str3;	//dst给下一个四元式的src1
		if(sym == "STARSY"){
			cnt++;
			getsym();
			factor();
			str2 = fourvalue;
			str3 = get_temp();
			insert_four("*",str3,str1,str2);
			insert_table(str3,5,0,-1,-1,1);
			address = address + 4;
		}else if(sym == "DIVSY"){
			cnt++;
			getsym();
			factor();
			str2 = fourvalue;
			str3 = get_temp();
			insert_four("/",str3,str1,str2);
			insert_table(str3,5,0,-1,-1,1);
			address = address + 4;
		}
	}
	fourvalue = str3;	//每一项计算后的值都在fourvalue里面
	for(int j = 0; j < temp_index; j++){
		temp[j].strr = "";
		temp[j].Sym = "";
	}
	temp_index = 0;
}

void factor(){
	nofactor = 0;
	string factorname, str3;
	for(int j = 0; j < temp_index; j++){
		temp[j].strr = "";
		temp[j].Sym = "";
	}
	temp_index = 0;
	if(sym == "IDSY"){
		factorname = str;
		temp[temp_index].Sym = sym;
		temp[temp_index].strr = str;
		temp_index++;
		cnt++;
		getsym();
		if(sym == "LEBARSY"){		//有中括号，数组
			cnt++;
			getsym();
			expr();					//此时fourvalue中存贮的应为数组的下标
			if(fourvalue[0] == '-'){
				error(9);
				return;
			}else{
				if(sym != "REBARSY"){
					cout << line << " " << "error! " << "没有]" << endl;
					return;
				}else{
					if(find_table(factorname,1) == true || find_table(factorname,0) == true){
						if(fourvalue[0] >= '0' && fourvalue[0] <= '9'){
							for(int i = 0; i < stindex; i++){
								if(symtab[i].name == factorname && symtab[i].kind == 4 && toint(fourvalue) >= symtab[i].paranum){	
									cout << line << " " << "error! 数组越界" << endl;
									break;
								}
							}
						}
						str3 = get_temp();	//生成临时变量,数组返回值存贮在里面
						insert_four("array",str3,factorname,fourvalue);
						for(int j = stindex - 1; j >= 0 ; j--){
							if(symtab[j].name == factorname && symtab[j].kind == 4){
								if(symtab[j].type == 0){
									insert_table(str3,5,0,-1,-1,1);
								}else if(symtab[j].type == 1){
									insert_table(str3,5,1,-1,-1,1);
								}
								break;
							}
						}
					//	insert_table(str3,5,-1,-1,-1,1);
						address = address + 4;
						fourvalue = str3;
						cnt++;
						getsym();
					}else{
						nofactor = 1;
						error(6);
					}

				}
			}
			
		}else if(sym == "LPARSY"){	//有左括号，有返回值函数调用
			temp[temp_index].Sym = sym;
			temp[temp_index].strr = str;
			temp_index++;
			call_returnable_statement();

			str3 = get_temp();	//生成临时变量,调用返回值存贮在里面
			insert_four("call",str3,factorname,"");
			for(int j = 0; j < stindex; j++){
				if(symtab[j].name == factorname && symtab[j].kind == 2){
					if(symtab[j].type == 0){
						insert_table(str3,5,0,-1,-1,1);
					}else if(symtab[j].type == 1){
						insert_table(str3,5,1,-1,-1,1);
					}
					break;
				}
			}
			address = address + 4;
			fourvalue = str3;
		}else{
			if(find_table(factorname,1) == true || find_table(factorname,0) == true)
				fourvalue =  factorname;
			else{
				nofactor = 1;
				error(6);
			}
		}

	}else if(sym == "INTEGERSY" || sym == "PLUSSY" || sym == "MINUSSY"){
		if((sym == "PLUSSY" || sym == "MINUSSY")){
			while(isSpace(ch) || isNewLine(ch) || isTab(ch)){
				ch = infile.get();
			}//跳过空格，空行，Tab
			if(ch == '+' || ch == '-'){
				cnt++;
				getsym();
				while(isSpace(ch) || isNewLine(ch) || isTab(ch)){
					ch = infile.get();
				}//跳过空格，空行，Tab
			}
			if(ch >= '0' && ch <= '9'){
				NUM();
				fourvalue = tostring(num);
				cnt++;
				getsym();
			}else if(ch == '('){
				cnt++;
				getsym();
				expr();
				if(sym != "RPARSY"){
					cout << line << " " << "error! " << "没有)" << endl;
					return;
				}else{
					cnt++;
					getsym();
				}
			}else{
				cnt++;
				getsym();
				fourvalue = str;
				cnt++;
				getsym();
			}
		}else{
			NUM();
			fourvalue = tostring(num);
			cnt++;
			getsym();
		}
	}else if(sym == "CHARACTERSY"){
		fourvalue = "\'" + str + "\'";
		cnt++;
		getsym();
	}else if(sym == "LPARSY"){
		cnt++;
		getsym();
		expr();
		if(sym != "RPARSY"){
			cout << line << " " << "error! " << "没有)" << endl;
			return;
		}else{
			cnt++;
			getsym();
		}
	}else
		fourvalue = "";

}

void returnable_statement(){
	string funcname;
	if(temp_index != 0){
		sym = temp[0].Sym;
		str = temp[0].strr;
		if(sym == "INTSY" || sym == "CHARSY"){
			if(sym == "INTSY")	type = 0;
			else type = 1;

			sym = temp[1].Sym;
			str = temp[1].strr;
			if(sym != "IDSY"){
				error(3);
				return;
			}else{
				funcname = str;
				function = str;		
				sym = temp[2].Sym;
				str = temp[2].strr;
				if(sym == "LPARSY"){
					for(int j = 0; j < temp_index; j++){
						temp[j].strr = "";
						temp[j].Sym = "";
					}
					temp_index = 0;
					if(find_table(funcname,0) == false){
						address = 0;
						insert_table(funcname,2,type,-1,0,0);
						insert_four(type==0 ? "int":"char", funcname,"(",")");
						temp_array[tempci++] = funcname;		//funcname入临时变量栈，作为函数结束之后出栈标志
					}else{
						cout << line << " " << funcname << " 函数名冲突" << endl;
					}
					cnt++;
					getsym();
					level = 1;
					parameter_list();
					if(sym == "RPARSY"){
						cnt++;
						getsym();
						if(sym == "LBARSY"){
							for(int i = stindex-1; i >=0; i--){
								if(symtab[i].name == funcname && symtab[i].kind == 2 && symtab[i].level == 0){
									symtab[i].paranum = paranum;
									break;
								}
							}		//修改符号表中函数的参数个数属性
							cnt++;
							getsym();
							level = 1;
							returnflag = 0;
							compound_statement();
							if(returnflag == 0){
								cout << line << " error! 有返回值函数无返回值" << endl;
							}
							if(sym == "RBARSY"){
								cnt++;
								getsym();
							}else{
								error(3);
								return;
							}
						}else{
							error(3);
							return;
						}
					}else{
						error(3);
						return;
					}
				}else{
					error(3);
					return;
				}
			}
		}else{
			error(3);
			return;
		}
	}else{
		if(sym == "INTSY" || sym == "CHARSY"){
			if(sym == "INTSY")	type = 0;
			else	type = 1;
			cnt++;
			getsym();
			if(sym == "IDSY"){
				funcname = str;
				function = str;
				cnt++;
				getsym();
				if(sym == "LPARSY"){
					if(find_table(funcname,0) == false){
						address = 0;
						insert_table(funcname,2,type,-1,0,0);
						insert_four(type==0 ? "int":"char", funcname,"(",")");
						temp_array[tempci++] = funcname;		//funcname入临时变量栈，作为函数结束之后出栈标志
					}else
						cout << line << " " << funcname << " 函数名冲突" << endl;
					cnt++;
					getsym();
					level = 1;
					parameter_list();
					if(sym == "RPARSY"){
						cnt++;
						getsym();
						if(sym == "LBARSY"){
							for(int i = stindex-1; i >=0; i--){
								if(symtab[i].name == funcname && symtab[i].kind == 2 && symtab[i].level == 0){
									symtab[i].paranum = paranum;
									break;
								}
							}		//修改符号表中函数的参数个数属性
							cnt++;
							getsym();
							level = 1;
							returnflag = 0;
							compound_statement();
							if(returnflag == 0){
								cout << line << " error! 有返回值函数无返回值" << endl;
							}
							if(sym == "RBARSY"){
								cnt++;
								getsym();
							}else{
								error(3);
								return;
							}
						}else{
							error(3);
							return;
						}
					}else{
						error(3);
						return;
					}
				}else{
					error(3);
					return;
				}
			}else{
				error(3);
				return;
			}
		}else{
			error(3);
			return;
		}
	}
	
	clear_temp(funcname);	//函数结束之后弹出临时变量栈的内容
	insert_four("end " + funcname,"","","");
	out_statement << "This is a returnable function statement!" << endl;
}

void void_statement(){
	string funcname;
	if(temp_index != 0){
		sym = temp[0].Sym;
		str = temp[0].strr;
		if(sym == "VOIDSY"){
			sym = temp[1].Sym;
			str = temp[1].strr;
			if(sym == "IDSY"){
				for(int j = 0; j < temp_index; j++){
					temp[j].strr = "";
					temp[j].Sym = "";
				}
				temp_index = 0;
				funcname = str;
				function = str;
				cnt++;
				getsym();
				if(sym == "LPARSY"){
					if(find_table(funcname,0) == false){
						address = 0;
						insert_table(funcname,2,2,-1,0,0);
						insert_four("void", funcname,"(",")");
						temp_array[tempci++] = funcname;		//funcname入临时变量栈，作为函数结束之后出栈标志
					}else
						cout << line << " " << funcname << " 函数名冲突" << endl;
					cnt++;
					getsym();
					level = 1;
					parameter_list();
					if(sym == "RPARSY"){
						cnt++;
						getsym();
						if(sym == "LBARSY"){
							for(int i = stindex-1; i >=0; i--){
								if(symtab[i].name == funcname && symtab[i].kind == 2 && symtab[i].level == 0){
									symtab[i].paranum = paranum;
									break;
								}
							}		//修改符号表中函数的参数个数属性
							cnt++;
							getsym();
							level = 1;
							compound_statement();
							if(sym == "RBARSY"){
								cnt++;
								getsym();
							}else{
								error(3);
								return;
							}
						}else{
							error(3);
							return;
						}
					}else{
						error(3);
						return;
					}
				}else{
					error(3);
					return;
				}
			}else{
				error(3);
				return;
			}
		}else{
			error(3);
			return;
		}
	}else{
		if(sym == "VOIDSY"){
			cnt++;
			getsym();
			if(sym == "IDSY"){
				funcname = str;
				function = str;
				cnt++;
				getsym();
				if(sym == "LPARSY"){
					if(find_table(funcname,0) == false){
						address = 0;
						insert_table(funcname,2,2,-1,0,0);
						insert_four("void", funcname,"(",")");
						temp_array[tempci++] = funcname;		//funcname入临时变量栈，作为函数结束之后出栈标志
					}else
						cout << line << " " << funcname << " 函数名冲突" << endl;
					cnt++;
					getsym();
					level = 1;
					parameter_list();
					if(sym == "RPARSY"){
						cnt++;
						getsym();
						if(sym == "LBARSY"){
							for(int i = stindex-1; i >=0; i--){
								if(symtab[i].name == funcname && symtab[i].kind == 2 && symtab[i].level == 0){
									symtab[i].paranum = paranum;
									break;
								}
							}		//修改符号表中函数的参数个数属性
							cnt++;
							getsym();
							level = 1;
							compound_statement();
							if(sym == "RBARSY"){
								cnt++;
								getsym();
							}
						}
					}
				}
			}
		}else{
			error(3);
			return;
		}
	}
	insert_four("end " + funcname,"","","");
	clear_temp(funcname);
	out_statement << "This is a void function statement!" << endl;
}

void parameter_list(){
	paranum = 0;
	if(sym == "INTSY" || sym == "CHARSY"){
		if(sym == "INTSY")	type = 0;
		else	type = 1;

		cnt++;
		getsym();
		if(sym == "IDSY"){
			name = str;
			cnt++;
			getsym();
			if(funcvalid == 0 && find_table(name,level) == false){
				insert_table(name,3,type,-1,-1,level);
				address = address + 4;
				paranum++;
				insert_four("para",type==0 ? "int":"char",name,"");
			}else if(funcvalid == 0)
				cout << line << " " << name << " 参数名冲突" << endl;

			while(sym == "COMMASY"){
				cnt++;
				getsym();
				if(sym == "INTSY" || sym == "CHARSY"){
					if(sym == "INTSY")	type = 0;
					else	type = 1;

					cnt++;
					getsym();
					if(sym == "IDSY"){
						name = str;
						cnt++;
						getsym();
						if(funcvalid == 0 && find_table(name,level) == false){
							insert_table(name,3,type,-1,-1,level);
							address = address + 4;
							paranum++;
							insert_four("para",type==0 ? "int":"char",name,"");
						}else if(funcvalid == 0)
							cout << line << " " << name << " 参数名冲突" << endl;
						
					}else{
						error(4);
					}
				}else{
					error(4);
				}
			}
		}
	}else if(sym != "RPARSY"){
		error(4);
	}
}

void compound_statement(){
	if(sym == "CONSTSY"){
		const_statement();
	}
	if(sym == "INTSY" || sym == "CHARSY"){
		var_statement();
	}
	statement_list();
}

void main_statement(){
	if(sym == "MAINSY"){
		function = "main";
		for(int j = 0; j < temp_index; j++){
			temp[j].strr = "";
			temp[j].Sym = "";
		}
		temp_index = 0;
		cnt++;
		getsym();
		if(sym != "LPARSY"){
			cout << line << " " << "error! 没有(" << endl;
			return;
		}else{
			cnt++;
			getsym();
			if(sym != "RPARSY"){
				cout << line << " " << "error! 没有)" << endl;
				return;
			}else{
				cnt++;
				getsym();
				if(sym != "LBARSY"){
					cout << line << " " << "error! 没有{" << endl;
					return;
				}else{
					if(find_table("main",0) == false){
						address = 0;
						insert_table("main",2,2,-1,0,0);
						insert_four("void", "main","(",")");
						temp_array[tempci++] = "main";		//funcname入临时变量栈，作为函数结束之后出栈标志
					}else
						cout << line << " " << "main" << " 函数名冲突" << endl;
					cnt++;
					getsym();
					level = 1;
					compound_statement();
					if(sym != "RBARSY"){
						cout << line << " " << "error! 没有}" << endl;
						return;
					}else{
						cnt++;
						getsym();
					}
				}
			}
		}
	}
	clear_temp("main");
	insert_four("end main","","","");
	out_statement << "This is main statement!" << endl;
}


void assign_statement(){
	string assignname;
	string arrayindex;
	if(sym == "IDSY" && (find_table(str,0) == true || find_table(str,1) == true)){

		int findconst = 0;
		int findarray = 0;
		int findfunction = 0;
		int findvar = 0;
		for(int i = 0; i < stindex; i++){
			if(symtab[i].name == function && symtab[i].level == 0){
				for(int j = i + 1; symtab[j].level != 0; j++){
					if(symtab[j].name == str && symtab[j].kind == 0){
						error(5);
						findconst = 1;
						break;
					}else if(symtab[j].name == str && symtab[j].kind == 4){
						findarray = 1;
						break;
					}else if(symtab[j].name == str && symtab[j].kind == 1){
						findvar = 1;
						break;
					}
				}
				break;
			}
		}
		if(findconst == 0 && findarray == 0 && findvar == 0){
			for(int i = 0; i < stindex; i++){
				if(symtab[i].name == str && symtab[i].kind == 0 && symtab[i].level == 0){
					error(5);
					findconst = 1;
					break;
				}else if(symtab[i].name == str && symtab[i].kind == 4 && symtab[i].level == 0){
					findarray = 1;
					break;
				}else if(symtab[i].name == str && symtab[i].kind == 2 && symtab[i].level == 0){
					int findfunction = 1;
					cout << line << " " << "error! 不能给函数赋值" << endl;
					return;
				}
			}
		}			//在局部符号表和全局符号表中查看标识符是否为常量,或者数组
		
		if(findconst == 0 && findfunction == 0){
			assignname = str;
			cnt++;
			getsym();
			if(sym == "ASSIGNSY"){
				if(findarray == 1){		//直接给数组整体赋值
					error(7);
				}else{
					cnt++;
					getsym();
					expr();
					if(nofactor == 0)
						insert_four("=",assignname,fourvalue,"");
				}
			}else if(sym == "LEBARSY"){
				if(findarray == 0){
					cout << line << " " << "error! 该标识符不是数组" << endl;
					while(str != ";"){
						cnt++;
						getsym();
					}
				}else{
					cnt++;
					getsym();
					expr();
					if(fourvalue[0] == '-'){
						error(9);
						return;
					}else{
						arrayindex = fourvalue;
						if(sym != "REBARSY"){
							cout << line << " " << "error! " << "没有]" << endl;
							while(str != ";"){
								cnt++;
								getsym();
							}
							return;
						}else{
							cnt++;
							getsym();
							if(sym != "ASSIGNSY"){
								cout << line << " " << "error! 不符合赋值语句结构" << endl;
								while(str != ";"){
									cnt++;
									getsym();
								}
								return;
							}else{
								cnt++;
								getsym();
								expr();
								if(nofactor == 0){
									if(arrayindex[0] >= '0' && arrayindex[0] <= '9'){
										for(int i = 0; i < stindex; i++){
											if(symtab[i].name == assignname && symtab[i].kind == 4 && toint(arrayindex) >= symtab[i].paranum)
												cout << line << " " << "error! 数组越界" << endl;
										}
									}
									insert_four("[]=",assignname,arrayindex,fourvalue);
								}
							}
						}
					}
					
				}

			}else{
				cout << line << " " << "error! 不符合赋值语句结构" << endl;
				while(str != ";"){
					cnt++;
					getsym();
				}
				return;
			}
		}

	}else{		//在符号表中没有找到标识符，读到分号结束
		error(6);
		return;
	}

	out_statement << "This is an assign statement!" << endl;
}

void condition_statement(){
	string label;	
	if(sym == "IFSY"){
		cnt++;
		getsym();
		label = new_label();	//生成一个新的标签
		if(sym == "LPARSY"){
			cnt++;
			getsym();
			condition();
			insert_four("BZ",label,"","");		//不满足条件跳转	
			if(sym != "RPARSY"){
				cout << line << " " << "没有)" << endl;
				return;
			}else{
				cnt++;
				getsym();
				statement();
			}
		}else{
			cout << line << " " << "没有(" << endl;
			return;
		}
	}
	set_label(label);			//插入标签
	
	out_statement << "This is a condition statement!" << endl;
}

void condition(){
	string src1, src2,symm;
	expr();
	src1 = fourvalue;
	if(sym == "LESSSY" || sym == "LOESY" || sym == "GREATERSY" 
		|| sym == "GOESY" || sym == "EQUSY" || sym == "UNEQUSY"){
			symm = sym;
			cnt++;
			getsym();
			expr();
			src2 = fourvalue;
			if(symm == "LESSSY")		insert_four("<",src1,src2,"");
			else if(symm == "LOESY")	insert_four("<=",src1,src2,"");
			else if(symm == "GREATERSY")	insert_four(">",src1,src2,"");
			else if(symm == "GOESY")	insert_four(">=",src1,src2,"");
			else if(symm == "EQUSY")	insert_four("==",src1,src2,"");
			else if(symm == "UNEQUSY")	insert_four("!=",src1,src2,"");
	}else if(str == ")"){
	//	string s = get_temp();
	//	insert_four("=", s, src1,"");
	//	insert_table(s,5,-1,-1,-1,1);
		insert_four("!=",src1,"0","");
	}else{
		cout << line << " " << "error! 条件不符合要求" << endl; 
	}
}

void do_while_statement(){
	string label;
	if(sym == "DOSY"){
		label = new_label();		//生成一个新标签		
		cnt++;
		getsym();
		set_label(label);			//do之后插入标签
		statement();
		if(sym != "WHILESY"){
			cout << line << " " << "error! 不符合do while 语句格式" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
			return;
		}else{
			cnt++;
			getsym();
			if(sym != "LPARSY"){
				cout << line << " " << "error! 没有(" << endl;
				while(str != ";"){
					cnt++;
					getsym();
				}
				return;
			}else{
				cnt++;
				getsym();
				condition();
				insert_four("BNZ",label,"","");		//满足条件跳转到循环里继续
				if(sym != "RPARSY"){
					cout << line << " " << "error! 没有)" << endl;
					while(str != ";"){
						cnt++;
						getsym();
					}
					return;
				}else{
					cnt++;
					getsym();
				}
			}
		}
	}
	out_statement << "This is a do-while statement!" << endl;
}

void switch_statement(){
	string caselabel = "switch_end" + tostring(switchc++);		//switch结束标签
	if(sym == "SWITCHSY"){
		cnt++;
		getsym();
		if(sym != "LPARSY"){
			cout << line << " " << "error! 没有(" << endl;
			return;
		}else{
			cnt++;
			getsym();
			expr();
			casevalue1 = fourvalue;		//switch 的值
			if(sym != "RPARSY"){
				return;
			}else{
				cnt++;
				getsym();
				if(sym != "LBARSY"){
					cout << line << " " << "error! 没有{" << endl;
					return;
				}else{
					cnt++;
					getsym();
					set_label(new_label());
					case_list(caselabel);
					if(sym != "RBARSY"){
						cout << line << " " << "error! 没有}" << endl;
						return;
					}else{
						cnt++;
						getsym();
					}
				}
			}
		}
	}
	set_label(caselabel);		//插入结束标签

	out_statement << "This is a switch statement!" << endl;
}

void case_list(string caselabel){
	while(sym == "CASESY"){
//		set_label(new_label());
		case_statement(caselabel);
	}
}

void case_statement(string caselabel){
	string label = "label_" + tostring(labc);
	labc++;
	if(sym == "CASESY"){
		cnt++;
		getsym();
		if(sym == "PLUSSY" || sym == "MINUSSY" || sym == "INTEGERSY"){
			NUM();
			casevalue2 = tostring(num);		//记录case后面的数值用于四元式生成的时候比较
			insert_four("==",casevalue1,casevalue2,"");
			insert_four("BZ",label,"","");
			cnt++;
			getsym();
		}else if(sym == "CHARACTERSY"){
			casevalue2 = str;				//记录case后面的字符用于四元式生成的时候比较
			insert_four("==",casevalue1,"\'" + casevalue2 + "\'","");
			insert_four("BZ",label,"","");
			cnt++;
			getsym();
		}else{
			cout << line << " " << "error! case后面不是整数或者字符" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
		}

		if(sym != "COLONSY"){
			cout << line << " " << "error! 缺少：" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
		}else{
			cnt++;
			getsym();
			statement();
			insert_four("GOTO",caselabel,"","");
			set_label(label);
			
		}
	}
}

void call_returnable_statement(){
	string funcname;
	int parac = 0;	//parac 为函数形参的个数
	if(temp_index != 0){
		sym = temp[0].Sym;
		str = temp[0].strr;
		if(sym == "IDSY"){
			funcname = str;
			sym = temp[1].Sym;
			str = temp[1].strr;
			if(sym != "LPARSY"){
				cout << line << " " << "error! 没有(" << endl;
				while(str != ";"){
					cnt++;
					getsym();
				}
				return;
			}else{
				for(int j = 0; j <= temp_index; j++){
					temp[j].strr = "";
					temp[j].Sym = "";
				}
				temp_index = 0;
				cnt++;
				getsym();
				value_table(funcname);

				for(int j = 0; j < stindex; j++){
					if(symtab[j].name == funcname && symtab[j].kind == 2){
						parac = symtab[j].paranum;
					}
				}


				if(sym != "RPARSY"){
					cout << line << " " << "error! 没有)" << endl;
					while(str != ";"){
						cnt++;
						getsym();
					}
					return;
				}else{
					cnt++;
					getsym();
				}
			}
		}else{
			cout << line << " " << "error! 格式不正确" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
			return;
		}
	}else{
		if(sym == "IDSY"){
			funcname = str;
			cnt++;
			getsym();
			if(sym != "LPARSY"){
				cout << line << " " << "error! 没有(" << endl;
				while(str != ";"){
					cnt++;
					getsym();
				}
				return;
			}else{
				cnt++;
				getsym();
				value_table(funcname);

				for(int j = 0; j < stindex; j++){
					if(symtab[j].name == funcname && symtab[j].kind == 2){
						parac = symtab[j].paranum;
					}
				}


				if(sym != "RPARSY"){
					cout << line << " " << "error! 没有(" << endl;
					while(str != ";"){
						cnt++;
						getsym();
					}
					return;
				}else{
					cnt++;
					getsym();
				}
			}
		}else{
			cout << line << " " << "error! 格式不正确" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
			return;
		}
	}
	
	insert_four("call",funcname,"",tostring(parac));
	out_statement << "This is returnable statement call!" << endl;
}

void call_void_statement(){
	string funcname;
	int parac = 0;	//parac 为函数形参的个数
	if(sym == "IDSY"){
		funcname = str;
		cnt++;
		getsym();
		if(sym != "LPARSY"){
			cout << line << " " << "error! 没有(" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
			return;
		}else{
			cnt++;
			getsym();
			value_table(funcname);
			
			for(int j = 0; j < stindex; j++){
				if(symtab[j].name == funcname && symtab[j].kind == 2){
					parac = symtab[j].paranum;
				}
			}

			if(sym != "RPARSY"){
				cout << line << " " << "error! 没有)" << endl;
				while(str != ";"){
					cnt++;
					getsym();
				}
				return;
			}else{
				cnt++;
				getsym();
			}
		}
	}
	insert_four("call",funcname,"",tostring(parac));
	out_statement << "This is void statement call!" << endl;
}

void value_table(string funcname){
	int parlc = 0;	//实参个数
	int parac = 0;	//形参个数
	int paratype = -1;	// null -1， int 0, char 1

	expr();

	if(fourvalue != ""){
		insert_four("push",fourvalue,"","");

		if(fourvalue[0] == '\''){
			paratype = 1;
		}else if(fourvalue[0] >= '0' && fourvalue[0] <= '9'){
			paratype = 0;
		}else{
			int find = 0;
			for(int i = 0; i < stindex; i++){
				if(symtab[i].name == function && symtab[i].kind == 2){
					for(int j = i + 1; symtab[j].level != 0; j++){
						if(symtab[j].name == fourvalue){
							if(symtab[j].type == 0){
								paratype = 0;
							}else if(symtab[j].type == 1){
								paratype = 1;
							}
							find = 1;
							break;
						}
					}
					break;
				}
			}
			if(find == 0){
				for(int i = 0; i < stindex; i++){
					if(symtab[i].name == fourvalue && symtab[i].level == 0){
						if(symtab[i].type == 0){
							paratype = 0;
						}else if(symtab[i].type == 1){
							paratype = 1;
						}
						break;
					}
				}
			}
		}
	}

	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname && symtab[j].kind == 2){
			parac = symtab[j].paranum;		//确定形参个数
			if(parac != 0){
				parlc++;
				if(symtab[j+parlc].type != paratype){
					cout << line << " " << "error! 函数调用形参实参类型不符" << endl;
				}
			}
			break;
		}
	}
	
	while(sym == "COMMASY"){
		cnt++;
		getsym();
		expr();

		if(fourvalue != ""){
			insert_four("push",fourvalue,"","");

			if(fourvalue[0] == '\''){
				paratype = 1;
			}else if(fourvalue[0] >= '0' && fourvalue[0] <= '9'){
				paratype = 0;
			}else{
				int find = 0;
				for(int i = 0; i < stindex; i++){
					if(symtab[i].name == function && symtab[i].kind == 2){
						for(int j = i + 1; symtab[j].level != 0; j++){
							if(symtab[j].name == fourvalue){
								if(symtab[j].type == 0){
									paratype = 0;
								}else if(symtab[j].type == 1){
									paratype = 1;
								}
								find = 1;
								break;
							}
						}
						break;
					}
				}
				if(find == 0){
					for(int i = 0; i < stindex; i++){
						if(symtab[i].name == fourvalue && symtab[i].level == 0){
							if(symtab[i].type == 0){
								paratype = 0;
							}else if(symtab[i].type == 1){
								paratype = 1;
							}
							break;
						}
					}
				}
			}
		}

		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname && symtab[j].kind == 2){
				parac = symtab[j].paranum;		//确定形参个数
				if(parac != 0){
					parlc++;
					if(symtab[j+parlc].type != paratype){
						cout << line << " " << "error! 函数调用形参实参类型不符" << endl;
					}
				}
				break;
			}
		}
	}

	if(parlc != parac){
		cout << line << " " << "error! 函数调用参数个数不正确" << endl;
	}
}

void statement_list(){
	while(sym == "IFSY" || sym == "DOSY" || sym == "SWITCHSY"
		|| sym == "LBARSY" || sym == "IDSY" || sym == "SCANFSY"
		|| sym == "PRINTFSY" || sym == "SEMISY" || sym == "RETURNSY"){
			statement();
	}
}

void statement(){
	if(sym == "IFSY"){
 		condition_statement();
	}else if(sym == "DOSY"){
		do_while_statement();
	}else if(sym == "SWITCHSY"){
		switch_statement();
	}else if(sym == "LBARSY"){
		cnt++;
		getsym();
		statement_list();
		if(sym != "RBARSY"){
			cout << line << " " << "error! 没有}" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
		}else{
			cnt++;
			getsym();
		}
	}else if(sym == "IDSY"){
		while(isSpace(ch) || isNewLine(ch) || isTab(ch)){
			ch = infile.get();
		}
		if(ch == '=' || ch == '['){
			assign_statement();
		}else if(ch == '('){
			int find = 0;
			for(int j = 0; j < stindex; j++){
				if(str == symtab[j].name && symtab[j].level == 0){
					if(symtab[j].type == 0 || symtab[j].type == 1){
						find = 1;
						call_returnable_statement();
					}else if(symtab[j].type == 2){
						find = 1;
						call_void_statement();
					}
					break;
				}
			}
			if(find == 0){
				error(10);
			}
		}else{
			cout << line << " " << "error! 未知语句" << endl;
			do{
				cnt++;
				getsym();
			}while(sym != "IDSY" && sym != "SEMISY" && sym != "IFSY" && sym != "DOSY" 
				&& sym != "SWITCHSY" && sym != "SCANFSY" && sym != "PRINTFSY" && sym != "RETURNSY");
			if(sym == "SEMISY"){
				cnt++;
				getsym();
			}
			if(sym == "RBARSY"){
				cnt++;
				getsym();
			}

			return;
		}

		if(sym != "SEMISY"){
			cout << line - 1 << " " << "error! 没有分号" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
		}else{
			cnt++;
			getsym();
		}
	}else if(sym == "SCANFSY"){
		read_statement();
		if(sym != "SEMISY"){
			cout << line - 1 << " " << "error! 没有分号" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
		}else{
			cnt++;
			getsym();
		}
	}else if(sym == "PRINTFSY"){
		write_statement();
		if(sym != "SEMISY"){
			cout << line - 1 << " " << "error! 没有分号" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
		}else{
			cnt++;
			getsym();
		}
	}else if(sym == "RETURNSY"){
		returnflag = 1;
		return_statement();
		if(sym != "SEMISY"){
			cout << line - 1 << " " << "error! 没有分号" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
		}else{
			cnt++;
			getsym();
		}
	}else if(sym == "SEMISY"){
		cnt++;
		getsym();
	}
}

void read_statement(){
	if(sym == "SCANFSY"){
		cnt++;
		getsym();
		if(sym != "LPARSY"){
			cout << line << " " << "error! 没有(" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
			return;
		}else{
			cnt++;
			getsym();
			if(sym == "IDSY" && (find_table(str,0) == true || find_table(str,1) == true)){

				int findconst = 0;
				int findarray = 0;
				int findfunction = 0;
				int findvar = 0;
				for(int i = 0; i < stindex; i++){
					if(symtab[i].name == function && symtab[i].kind == 2 && symtab[i].level == 0){
						for(int j = i + 1; symtab[j].level != 0; j++){
							if(symtab[j].name == str && symtab[j].kind == 0){
								findconst = 1;
								cout << line << " " << "error! scanf中的内容不能为常量" << endl;
								while(str != "," && str != ")"){
									cnt++;
									getsym();
								}
								break;
							}else if(symtab[j].name == str && symtab[j].kind == 4){
								findarray = 1;
								cout << line << " " << "error! scanf中的内容不能为数组整体" << endl;
								while(str != "," && str != ")"){
									cnt++;
									getsym();
								}
								break;
							}else if(symtab[j].name == str && symtab[j].kind == 1){
								findvar = 1;
								break;
							}
						}
						break;
					}
				}
				if(findconst == 0 && findarray == 0 && findvar == 0){
					for(int i = 0; i < stindex; i++){
						if(symtab[i].name == str && symtab[i].kind == 0 && symtab[i].level == 0){
							findconst = 1;
							cout << line << " " << "error! scanf中的内容不能为常量" << endl;
							while(str != "," && str != ")"){
								cnt++;
								getsym();
							}
							break;
						}else if(symtab[i].name == str && symtab[i].kind == 4 && symtab[i].level == 0){
							findarray = 1;
							cout << line << " " << "error! scanf中的内容不能为数组整体" << endl;
							while(str != "," && str != ")"){
								cnt++;
								getsym();
							}
							break;
						}else if(symtab[i].name == str && symtab[i].kind == 2 && symtab[i].level == 0){
							findfunction = 1;
							cout << line << " " << "error! scanf中的内容不能为函数" << endl;
							while(str != ")" && str != ","){
								cnt++;
								getsym();
							}
						}
					}
				}	//判断scanf里面的内容是不是常量或者数组或者函数

				if(findconst == 0 && findarray == 0 && findfunction == 0){
					insert_four("scanf",str,"","");
					cnt++;
					getsym();
				}
				

				while(sym == "COMMASY"){
					cnt++;
					getsym();
					if(sym != "IDSY"){
						cout << line << " " << "error! 不符合scanf语句格式" << endl;
						while(str != ";"){
							cnt++;
							getsym();
						}
						return;
					}else if(sym == "IDSY" && (find_table(str,0) == true || find_table(str,1) == true)){
						int findconst = 0;
						int findarray = 0;
						int findfunction = 0;
						int findvar = 0;
						for(int i = 0; i < stindex; i++){
							if(symtab[i].name == function && symtab[i].kind == 2 && symtab[i].level == 0){
								for(int j = i + 1; symtab[j].level != 0; j++){
									if(symtab[j].name == str && symtab[j].kind == 0){
										findconst = 1;
										cout << line << " " << "error! scanf中的内容不能为常量" << endl;
										while(str != "," && str != ")"){
											cnt++;
											getsym();
										}
										break;
									}else if(symtab[j].name == str && symtab[j].kind == 4){
										findarray = 1;
										cout << line << " " << "error! scanf中的内容不能为数组整体" << endl;
										while(str != "," && str != ")"){
											cnt++;
											getsym();
										}
										break;
									}else if(symtab[j].name == str && symtab[j].kind == 1){
										findvar = 1;
										break;
									}
								}
								break;
							}
						}
						if(findconst == 0 && findarray == 0 && findvar == 0){
							for(int i = 0; i < stindex; i++){
								if(symtab[i].name == str && symtab[i].kind == 0 && symtab[i].level == 0){
									findconst = 1;
									cout << line << " " << "error! scanf中的内容不能为常量" << endl;
									while(str != "," && str != ")"){
										cnt++;
										getsym();
									}
									break;
								}else if(symtab[i].name == str && symtab[i].kind == 4 && symtab[i].level == 0){
									findarray = 1;
									cout << line << " " << "error! scanf中的内容不能为数组整体" << endl;
									while(str != "," && str != ")"){
										cnt++;
										getsym();
									}
									break;
								}else if(symtab[i].name == str && symtab[i].kind == 2 && symtab[i].level == 0){
									findfunction = 1;
									cout << line << " " << "error! scanf中的内容不能为函数" << endl;
									while(str != ")" && str != ","){
										cnt++;
										getsym();
									}
								}
							}
						}	//判断scanf里面的内容是不是常量或者数组或者函数

						if(findconst == 0 && findarray == 0 && findfunction == 0){
							insert_four("scanf",str,"","");
							cnt++;
							getsym();
						}
					}else{
						error(6);
					}
				}
				if(sym != "RPARSY"){
					cout << line << " " << "error! 没有)" << endl;
					while(str != ";"){
						cnt++;
						getsym();
					}
					return;
				}else{
					cnt++;
					getsym();
				}
			}else{
				error(6);
			}
		}
	}
	out_statement << "This is a read statemnt!" << endl;
}

void write_statement(){
	string s;
	if(sym == "PRINTFSY"){
		cnt++;
		getsym();
		if(sym != "LPARSY"){
			cout << line << " " << "error! 没有(" << endl;
			while(str != ";"){
				cnt++;
				getsym();
			}
			return;
		}else{
			cnt++;
			getsym();
			if(sym == "STRINGSY"){
				s = str;
				cnt++;
				getsym();
				if(sym == "COMMASY"){
					cnt++;
					getsym();
					expr();
					if(sym != "RPARSY"){
						cout << line << " " << "error! 没有)" << endl;
						while(str != ";"){
							cnt++;
							getsym();
						}
						return;
					}else{
						insert_four("#string","str"+tostring(strcount),s,"");
						insert_four("printf","str"+tostring(strcount),fourvalue,"");
						strcount++;
						cnt++;
						getsym();
					}
				}else if(sym == "RPARSY"){
					insert_four("#string","str"+tostring(strcount),s,"");
					insert_four("printf","str"+tostring(strcount),"","");
					strcount++;
					cnt++;
					getsym();
				}else{
					cout << line << " " << "error! 不符合printf格式规定" << endl;
					while(str != ";"){
						cnt++;
						getsym();
					}
					return;
				}
			}else if(sym == "IDSY" || sym == "CHARACTERSY" || sym == "INTEGERSY" || sym == "PLUSSY" || sym == "MINUSSY"){
				expr();
				if(sym != "RPARSY"){
					cout << line << " " << "error! 没有)" << endl;
					while(str != ";"){
						cnt++;
						getsym();
					}
				}else{
					insert_four("printf","",fourvalue,"");
					cnt++;
					getsym();
				}
			}else{
				cout << line << " error! 输出内容不符合要求" << endl;
				while(str != ";"){
					cnt++;
					getsym();
				}
			}
		}
	}
	out_statement << "This is a write statement!" << endl;
}

void return_statement(){
	int returnable = 0;		// returnable 0; void 1;
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == function && symtab[j].kind == 2){
			if(symtab[j].type == 2)
				returnable = 1;
			break;
		}
	}
	if(sym == "RETURNSY"){
		cnt++;
		getsym();
		if(sym == "LPARSY"){
			cnt++;
			getsym();
			if(returnable == 1 && str != ")"){
				cout << line << " " << "error! 无返回值函数有返回值" << endl;
			}
			expr();
			if(sym != "RPARSY"){
				cout << line << " " << "error! 缺少)" << endl;
				while(str != ";"){
					cnt++;
					getsym();
				}
				
			}else{
				cnt++;
				getsym();
				insert_four("return",fourvalue,"","");
			}
		}else{
			if(returnable == 0){
				cout << line << " error! 有返回值函数无返回值" << endl;
			}
			insert_four("return","","","");
		}
	}
//	insert_four("end function","","","");
	out_statement << "This is a return statement!" << endl;
}

void program(){
 	if(sym == "CONSTSY"){
		level = 0;
		const_statement();
	}
	if(sym == "INTSY" || sym == "CHARSY"){
		for(int j = 0; j < temp_index; j++){
			temp[j].strr = "";
			temp[j].Sym = "";
		}
		temp_index = 0;
		temp[temp_index].Sym = sym;
		temp[temp_index].strr = str;
		temp_index++;
		cnt++;
		getsym();
		temp[temp_index].Sym = sym;
		temp[temp_index].strr = str;
		temp_index++;
		cnt++;
		getsym();
		temp[temp_index].Sym = sym;
		temp[temp_index].strr = str;
		temp_index++;

		if(sym == "SEMISY" || sym == "COMMASY" || sym == "LEBARSY"){
			level = 0;
			var_statement();
		}else{
			level = 0;
			returnable_statement();
			if(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY")
				cout << line << " error! 非法字符" << endl;
			while(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY"){
				cnt++;
				getsym();
			}
		}
		if(temp_index != 0){
			level = 0;
			returnable_statement();
			if(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY")
				cout << line << " error! 非法字符" << endl;
			while(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY"){
				cnt++;
				getsym();
			}
		}
		while(sym == "INTSY" || sym == "CHARSY" || sym == "VOIDSY"){
			if(sym == "INTSY" || sym == "CHARSY"){
				level = 0;
				returnable_statement();
				if(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY")
					cout << line << " error! 非法字符" << endl;
				while(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY"){
					cnt++;
					getsym();
				}
			}else if(sym == "VOIDSY"){
				for(int j = 0; j <= temp_index; j++){
					temp[j].strr = "";
					temp[j].Sym = "";
				}
				temp_index = 0;
				temp[temp_index].Sym = sym;
				temp[temp_index].strr = str;
				temp_index++;

				cnt++;
				getsym();
				temp[temp_index].Sym = sym;
				temp[temp_index].strr = str;
				temp_index++;

				if(sym == "MAINSY"){
					level = 0;
					main_statement();
				}else{
					level = 0;
					void_statement();
					if(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY")
						cout << line << " error! 非法字符" << endl;
					while(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY"){
						cnt++;
						getsym();
					}
					for(int j = 0; j <= temp_index; j++){
						temp[j].strr = "";
						temp[j].Sym = "";
					}
					temp_index = 0;
				}
			}
		}
	}else if(sym == "VOIDSY"){
		for(int j = 0; j <= temp_index; j++){
			temp[j].strr = "";
			temp[j].Sym = "";
		}
		temp_index = 0;
		temp[temp_index].Sym = sym;
		temp[temp_index].strr = str;
		temp_index++;

		cnt++;
		getsym();
		temp[temp_index].Sym = sym;
		temp[temp_index].strr = str;
		temp_index++;

		if(sym == "MAINSY"){
			for(int j = 0; j < temp_index; j++){
				temp[j].strr = "";
					temp[j].Sym = "";
			}
			temp_index = 0;
			level = 0;
			main_statement();
		}else{
			level = 0;
			void_statement();
			if(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY")
				cout << line << " error! 非法字符" << endl;
			while(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY"){
				cnt++;
				getsym();
			}
			while(sym == "INTSY" || sym == "CHARSY" || sym == "VOIDSY"){
				if(sym == "INTSY" || sym == "CHARSY"){
					level = 0;
					returnable_statement();
					if(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY")
						cout << line << " error! 非法字符" << endl;
					while(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY"){
						cnt++;
						getsym();
					}
				}else if(sym == "VOIDSY"){
					for(int j = 0; j <= temp_index; j++){
						temp[j].strr = "";
						temp[j].Sym = "";
					}
					temp_index = 0;
					temp[temp_index].Sym = sym;
					temp[temp_index].strr = str;
					temp_index++;

					cnt++;
					getsym();
					temp[temp_index].Sym = sym;
					temp[temp_index].strr = str;
					temp_index++;

					if(sym == "MAINSY"){
						level = 0;
						main_statement();
					}else{
						level = 0;
						void_statement();
						if(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY")
							cout << line << " error! 非法字符" << endl;
						while(sym != "INTSY" && sym != "CHARSY" && sym != "VOIDSY"){
							cnt++;
							getsym();
						}
					}
				}
			}
		}
	}
	out_statement << "This is a programme!" << endl;
}

void golbal();
void push_mips();
void call_mips();
void add_mips();
void sub_mips();
void mult_mips();
void div_mips();
void assign_mips();
void return_mips();
void scanf_mips();
void printf_mips();
void branch_mips(int i);
void assign_array_mips();
void use_array_mips();

intermediate_code itm;
string funcname_mips;
int offset;		//偏移量
int paracount = 0;	//函数形参的个数
int return_add = 0;	//函数返回值偏移地址

void to_mips(){
	golbal();

	//.data处理结束，开始处理 .text部分
	mips << ".text" << endl;
	mips << "j main" << endl;
	mips << "nop" << endl;

	for(int i = 0; i < four.size(); i++){		//遍历四元式
		itm = four[i];
//		mips << "##########" << itm.op << " " << itm.dst << " " << itm.src1 << " " << itm.src2 << endl;
		if(itm.src1 == "(" && itm.src2 == ")"){			//这是一个函数
			offset = 0;
			funcname_mips = itm.dst;
			mips << itm.dst << ":" << endl;		//函数标签
			if(itm.dst != "main"){
				mips << "sub $sp, $sp, 4" << endl;
				mips << "sw $ra, 4($sp)" << endl;	//保存$ra
			}										
			mips << "move $fp, $sp" << endl;	
			for(int j = 0; j < stindex; j++){
				if(symtab[j].name == itm.dst && symtab[j].kind == 2){
					paracount = symtab[j].paranum;
					return_add = (3 + symtab[j].paranum) * 4 + 4;
					j++;
					for(int k = j; k < stindex; k++){		//将函数内部的常量、变量及形参转换为mips
						if(symtab[k].level == 1){
							if(symtab[k].kind == 0 && symtab[k].type == 0){	//const int
								mips << "sub $sp, $sp, 4" << endl;
								mips << "li $t0, " << symtab[k].value << endl;
								offset = 0 - symtab[k].address;
								mips << "sw $t0, " << offset << "($fp)" << endl;
							}else if(symtab[k].kind == 0 && symtab[k].type == 1){	// const char			!!!!!!!!!! 与 int 相同
								mips << "sub $sp, $sp, 4" << endl;
								mips << "li $t0, " << symtab[k].value << endl;
								offset = 0 - symtab[k].address;
								mips << "sw $t0, " << offset << "($fp)" << endl;
							}else if(symtab[k].kind == 1){				//var
								mips << "sub $sp, $sp, 4" << endl;
								offset = offset - 4;
							}else if(symtab[k].kind == 4){				// var array
								mips << "sub $sp, $sp, " << symtab[k].paranum * 4 << endl;
								offset = offset - paranum * 4 - 4;
							}else if(symtab[k].kind == 3){			//参数
								mips << "sub $sp, $sp, 4" << endl;
								mips << "lw $t0, " << (paracount + 3)  * 4 << "($fp)" << endl;
								offset = 0 - symtab[k].address;
								mips << "sw $t0, " << offset << "($fp)" << endl;
								offset = offset - 4;
								paracount--;
							}else if(symtab[k].kind == 5){			//临时变量
								mips << "sub $sp, $sp, 4" << endl;
								offset = offset - 4;
							}
						}else if(symtab[k].level == 0)
							break;
					}
					break;
				}
			}
		}else if(itm.op == "push"){ 
			push_mips();
			i++;
			itm = four[i];
			while(itm.op == "push"){
				push_mips();
				i++;
				itm = four[i];
			}
			i--;
 		}else if(itm.op == "call"){
			call_mips();
		}else if(itm.op == "para"){
			;						//在上面符号表中已处理
 		}else if(itm.op == "+"){
			add_mips();
		}else if(itm.op == "-"){
			sub_mips();
		}else if(itm.op == "return"){
			return_mips();
		}else if(itm.op == "="){
			assign_mips();
		}else if(itm.op == "*"){
			mult_mips();
		}else if(itm.op == "/"){
			div_mips();
		}else if(itm.op.substr(0,6) == "label_"){
			mips << itm.op << ":" << endl;
		}else if(itm.op == "scanf"){
			scanf_mips();
		}else if(itm.op == "printf"){
			printf_mips();
		}else if(itm.op == "<" || itm.op == "<=" || itm.op == ">" 
			|| itm.op == ">=" || itm.op == "!=" || itm.op == "=="){
				branch_mips(i);
		}else if(itm.op == "GOTO"){			
			mips << "j " << itm.dst << endl;
		}else if(itm.op.substr(0,10) == "switch_end"){
			mips << itm.op << ":" << endl;
		}else if(itm.op == "[]="){
			assign_array_mips();
		}else if(itm.op == "array"){
			use_array_mips();
		}else if(itm.op.substr(0,3) == "end"){
			if(itm.op == "end main"){
				mips << "li $v0, 10" << endl;
				mips << "syscall" << endl;
			}else{
				mips << "lw $sp, 12($fp)" << endl;
				mips << "lw $ra, 4($fp)" << endl;
				mips << "lw $fp,8($fp)" << endl;
				mips << "jr $ra" << endl;
				mips << "nop" << endl;
			}
		}
	}
}

void golbal(){
	mips << ".data" << endl;

	for(int i = 0; i < stindex; i++){
		if(symtab[i].level == 0){		//全局
			if(symtab[i].kind == 0){	//常量 const
				if(symtab[i].type == 0){	// int
					mips << symtab[i].name << ": " << ".word " << symtab[i].value << endl;
				}else if(symtab[i].type == 1){	//char
					mips << symtab[i].name << ": " << ".word " << "\'" << char(symtab[i].value) << "\'" << endl;
				}
			}else if(symtab[i].kind == 1){	//变量var
				if(symtab[i].type == 0){	// int
					mips << symtab[i].name << ": .space 4" << endl;
				}else if(symtab[i].type == 1){	//char
					mips << symtab[i].name << ": .space 4 " << endl;
				}
			}else if(symtab[i].kind == 4){	//数组array
				if(symtab[i].type == 0){	//int
					mips << symtab[i].name << ": " << ".space " << symtab[i].paranum * 4 << endl;
				}else if(symtab[i].type == 1){	//char
					mips << symtab[i].name << ": " << ".space " << symtab[i].paranum * 4 << endl;
				}
			}
		}		
	}

	//str 字符串打印
	intermediate_code itm;	
	for(int i = 0; i < four.size(); i++){
		itm = four[i];
		if(itm.op == "#string"){
			mips << itm.dst << ": .asciiz " << "\""+itm.src1+"\"" << endl;
		}
	}
}

void push_mips(){

	int find = 0;	//实参为函数内部的-1，全局-0
	mips << "sub $sp, $sp, 4" << endl;

	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst && symtab[k].level == 1){
					find = 1;
					mips << "lw $t0, " << 0 -symtab[k].address << "($fp)" << endl;
					mips << "sw $t0, 4($sp)" << endl;
					break;
				}
			}
			break;
		}
	}
		
	if(find == 0){
		if(itm.dst[0] >= '0' && itm.dst[0] <= '9' || itm.dst[0] == '+' || itm.dst[0] == '-' || itm.dst[0] == '\''){
			mips << "li $t0, " << itm.dst << endl;
			mips << "sw $t0, 4($sp)" << endl;
		}else{
			mips << "lw $t0, " << itm.dst << endl;
			mips << "sw $t0, 4($sp)" << endl;
		}
	}
}

void call_mips(){
	if(itm.src1 == ""){

		mips << "li $t0, " << itm.src2 << endl;
		mips << "mul $t0, $t0, 4" << endl;
		mips << "add $t1, $t0, $sp" << endl;

		mips << "sub $sp, $sp, 4" << endl;
		mips << "sw $t1, 4($sp)" << endl;
		mips << "sub $sp, $sp, 4" << endl;
		mips << "sw $fp, 4($sp)" << endl;
		mips << "jal " << itm.dst << endl;
		mips << "nop" << endl;
		
	}else{
		mips << "move $t0, $v0" << endl;
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.dst){
						offset = 0 - symtab[k].address;
						break;
					}
				}
				break;
			}
		}
		mips << "sw $t0, " << offset << "($fp)" << endl;	
				
	}
}

void add_mips(){
	int find1 = 0;
	int find2 = 0;
	int temp_address = 0;
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src1
				if(symtab[k].name == itm.src1){
					mips << "lw $t0, " << -symtab[k].address << "($fp)" << endl;
					find1 = 1;
					break;
				}
			}
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src2
				if(symtab[k].name == itm.src2){
					mips << "lw $t1, " << -symtab[k].address << "($fp)" << endl;
					find2 = 1;
					break;
				}
			}
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){							//找到临时变量的相对位置
					temp_address = 0 - symtab[k].address;
					break;
				}
			}
			break;
		}
	}
	if(find1 == 0){				//在局部中没找到，直接调用全局常量或者是立即数
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+')
			mips << "li $t0, " << itm.src1 << endl;
		else if(itm.src1[0] == '\''){
			mips << "li $t0, " << itm.src1 << endl;
		}else{
			mips << "la $t3, " << itm.src1 << endl;
			mips << "lw $t0, 0($t3)" << endl;
		}
	}
	if(find2 == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '-' || itm.src2[0] == '+')
			mips << "li $t1, " << itm.src2 << endl;
		else if(itm.src2[0] == '\''){
			mips << "li $t1, " << itm.src2 << endl;
		}else{
			mips << "la $t3, " << itm.src2 << endl;
			mips << "lw $t1, 0($t3)" << endl;
		}
	}

	mips << "add $t2, $t0, $t1" << endl;
	mips << "sw $t2, " << temp_address << "($fp)" << endl;	//将+运算之后的值存在临时变量中，临时变量压栈
}

void sub_mips(){
	int find1 = 0;
	int find2 = 0;
	int temp_address = 0;
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src1
				if(symtab[k].name == itm.src1){
					mips << "lw $t0, " << -symtab[k].address << "($fp)" << endl;
					find1 = 1;
					break;
				}
			}
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src2
				if(symtab[k].name == itm.src2){
					mips << "lw $t1, " << -symtab[k].address << "($fp)" << endl;
					find2 = 1;
					break;
				}
			}
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){							//找到临时变量的相对位置
					temp_address = 0 - symtab[k].address;
					break;
				}
			}
			break;
		}
	}
	if(find1 == 0){				//在局部中没找到，直接调用全局常量或者是立即数
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+')
			mips << "li $t0, " << itm.src1 << endl;
		else if(itm.src1[0] == '\''){
			mips << "li $t0, " << itm.src1 << endl;
		}else{
			mips << "la $t3, " << itm.src1 << endl;
			mips << "lw $t0, 0($t3)" << endl;
		}
	}
	if(find2 == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '-' || itm.src2[0] == '+')
			mips << "li $t1, " << itm.src2 << endl;
		else if(itm.src2[0] == '\''){
			mips << "li $t1, " << itm.src2 << endl;
		}else{
			mips << "la $t3, " << itm.src2 << endl;
			mips << "lw $t1, 0($t3)" << endl;
		}
	}

	mips << "sub $t2, $t0, $t1" << endl;
	mips << "sw $t2, " << temp_address << "($fp)" << endl;	//将-运算之后的值存在临时变量中，临时变量压栈
}

void mult_mips(){
	int find1 = 0;
	int find2 = 0;
	int temp_address = 0;
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src1
				if(symtab[k].name == itm.src1){
					mips << "lw $t0, " << -symtab[k].address << "($fp)" << endl;
					find1 = 1;
					break;
				}
			}
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src2
				if(symtab[k].name == itm.src2){
					mips << "lw $t1, " << -symtab[k].address << "($fp)" << endl;
					find2 = 1;
					break;
				}
			}
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){							//找到临时变量的相对位置
					temp_address = 0 - symtab[k].address;
					break;
				}
			}
			break;
		}
	}
	if(find1 == 0){				//在局部中没找到，直接调用全局常量或者是立即数
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+' || itm.src1[0] == '\'')
			mips << "li $t0, " << itm.src1 << endl;
		else{
			mips << "la $t3, " << itm.src1 << endl;
			mips << "lw $t0, 0($t3)" << endl;
		}
	}
	if(find2 == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '-' || itm.src2[0] == '+' || itm.src2[0] == '\'')
			mips << "li $t1, " << itm.src2 << endl;
		else{
			mips << "la $t3, " << itm.src2 << endl;
			mips << "lw $t1, 0($t3)" << endl;
		}
	}

	mips << "mul $t2, $t0, $t1" << endl;
	mips << "sw $t2, " << temp_address << "($fp)" << endl;	//将-运算之后的值存在临时变量中，临时变量压栈
}

void div_mips(){
	int find1 = 0;
	int find2 = 0;
	int temp_address = 0;
//	mips << "sub $sp, $sp, 4" << endl;
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src1
				if(symtab[k].name == itm.src1){
					mips << "lw $t0, " << -symtab[k].address << "($fp)" << endl;
					find1 = 1;
					break;
				}
			}
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src2
				if(symtab[k].name == itm.src2){
					mips << "lw $t1, " << -symtab[k].address << "($fp)" << endl;
					find2 = 1;
					break;
				}
			}
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){							//找到临时变量的相对位置
					temp_address = 0 - symtab[k].address;
					break;
				}
			}
			break;
		}
	}
	if(find1 == 0){				//在局部中没找到，直接调用全局常量或者是立即数
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+' || itm.src1[0] == '\'')
			mips << "li $t0, " << itm.src1 << endl;
		else{
			mips << "la $t3, " << itm.src1 << endl;
			mips << "lw $t0, 0($t3)" << endl;
		}
	}
	if(find2 == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '-' || itm.src2[0] == '+' || itm.src2[0] == '\'')
			mips << "li $t1, " << itm.src2 << endl;
		else{
			mips << "la $t3, " << itm.src2 << endl;
			mips << "lw $t1, 0($t3)" << endl;
		}
	}

	mips << "div $t2, $t0, $t1" << endl;
	mips << "sw $t2, " << temp_address << "($fp)" << endl;	//将-运算之后的值存在临时变量中，临时变量压栈
}

void assign_mips(){
	int find1 = 0;		//标记在函数内部找到了赋值的对象-1
	int find2 = 0;		//标记赋的值是临时变量，不是全局常量或变量-1
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.src1){
					mips << "lw $t0, " << 0 - symtab[k].address << "($fp)" << endl;			//取出赋的值
					find2 = 1;
					break;
				}
			}
			break;
		}
	}

	if(find2 == 0){
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+')
			mips << "li $t0, " << itm.src1 << endl;
		else if(itm.src1[0] == '\''){
			mips << "li $t0, " << itm.src1 << endl;
		}else{
			mips << "la $t3, " << itm.src1 << endl;
			mips << "lw $t0, 0($t3)" << endl;
		}
	}


	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){
					mips << "sw $t0, " << 0 -symtab[k].address << "($fp)" << endl;			//找到赋值的对象
					find1 = 1;
					break;
				}
			}
			break;
		}
	}

	if(find1 == 0){
		mips << "sw $t0, " << itm.dst << endl;
	}
}

void return_mips(){
	int find = 0;
	if(itm.dst == ""){
		;
	}else{
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.dst){
						find = 1;
						mips << "lw $v0, " << 0 - symtab[k].address << "($fp)" << endl;			//取出赋的值	
						break;
					}
				}
				break;
			}
		}

		if(find == 0){
			if(itm.dst[0] >= '0' && itm.dst[0] <= '9' || itm.dst[0] == '-' || itm.dst[0] == '+'){
				mips << "li $v0, " << itm.dst << endl;
			}else if(itm.dst[0] == '\''){
				mips << "li $v0, " << itm.dst << endl;
			}else{
				mips << "la $t0, " << itm.dst << endl;
				mips << "lw $v0, 0($t0)" << endl;
			}
		}
	}
	if(funcname_mips == "main"){
		mips << "li $v0, 10" << endl;
		mips << "syscall" << endl;
	}else{
		mips << "lw $sp, 12($fp)" << endl;
		mips << "lw $ra, 4($fp)" << endl;
		mips << "lw $fp,8($fp)" << endl;
		mips << "jr $ra" << endl;
		mips << "nop" << endl;
	}
}

void scanf_mips(){
	int find = 0;		//在函数局部中找到-1， 全局-0
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){
					if(symtab[k].type == 0){
						mips << "li $v0, 5" << endl;
						mips << "syscall" << endl;
						mips << "sw $v0, " << 0 - symtab[k].address << "($fp)" << endl;
						find = 1;
					}else if(symtab[k].type == 1){
						mips << "li $v0, 12" << endl;
						mips << "syscall" << endl;
						mips << "sw $v0, " << 0 - symtab[k].address << "($fp)" << endl;
						find = 1;
					}
					break;
				}
			}
			break;
		}
	}

	if(find == 0){
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == itm.dst && symtab[j].type == 0){
				mips << "li $v0, 5" << endl;
				mips << "syscall" << endl;
				mips << "la $t0, " << itm.dst << endl;
				mips << "sw $v0, 0($t0)" << endl;
				break;
			}else if(symtab[j].name == itm.dst && symtab[j].type == 1){
				mips << "li $v0, 12" << endl;
				mips << "syscall" << endl;
				mips << "la $t0, " << itm.dst << endl;
				mips << "sw $v0, 0($t0)" << endl;
				break;
			}
		}
	}
}

void printf_mips(){
	int find = 0;
	if(itm.src1 == "" && itm.dst.substr(0,3) == "str"){		//只输出字符串
		mips << "li $v0, 4" << endl;
		mips << "la $a0, " << itm.dst << endl;
		mips << "syscall" << endl;
	}else if(itm.src1 != "" && itm.dst.substr(0,3) == "str"){	//输出字符串+表达式
		mips << "li $v0, 4" << endl;
		mips << "la $a0, " << itm.dst << endl;
		mips << "syscall" << endl;

//		mips << "li $v0, 1" << endl;
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.src1){
						find = 1;
						if(symtab[k].type == 0){
							mips << "li $v0, 1" << endl;
							mips << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}else if(symtab[k].type == 1){
							mips << "li $v0, 11" << endl;
							mips << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}else if(symtab[k].type == -1){			///////////
							mips << "li $v0, 1" << endl;
							mips << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}
 						
						break;
					}
				}
				break;
			}
		}
		if(find == 0){
			if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+'){
				mips << "li $v0, 1" << endl;
				mips << "li $a0," << itm.src1 << endl;
			}else if(itm.src1[0] == '\''){
				mips << "li $v0, 11" << endl;
				mips << "li $a0, " << itm.src1 << endl;
			}else{
				for(int j = 0; j < stindex; j++){
					if(symtab[j].name == itm.src1){
						if(symtab[j].type == 0){
							mips << "li $v0, 1" << endl;
						}else if(symtab[j].type == 1){
							mips << "li $v0, 11" << endl;
						}
						break;
					}
				}
				mips << "la $t0, "<< itm.src1 << endl;
				mips << "lw $a0, 0($t0)" << endl;
			}
		}
		mips << "syscall" << endl;
	}else{			//输出表达式
	//	mips << "li $v0, 1" << endl;
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.src1){
						find = 1;
						if(symtab[k].type == 0){
							mips << "li $v0, 1" << endl;
							mips << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}else if(symtab[k].type == 1){
							mips << "li $v0, 11" << endl;
							mips << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}else if(symtab[k].type == -1){			///////////
							mips << "li $v0, 1" << endl;
							mips << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}
						break;
					}
				}
				break;
			}
		}
		if(find == 0){
			if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+'){
				mips << "li $v0, 1" << endl;
				mips << "li $a0," << itm.src1 << endl;
			}else if(itm.src1[0] == '\''){
				mips << "li $v0, 11" << endl;
				mips << "li $a0, " << itm.src1 << endl;
			}else{
				for(int j = 0; j < stindex; j++){
					
					if(symtab[j].name == itm.src1){
						if(symtab[j].type == 0){
							mips << "li $v0, 1" << endl;
						}else if(symtab[j].type == 1){
							mips << "li $v0, 11" << endl;
						}
						break;
					}
				}
				mips << "la $t0, "<< itm.src1 << endl;
				mips << "lw $a0, 0($t0)" << endl;
			}
		}
		mips << "syscall" << endl;
	}
}

void branch_mips(int i){
	int find1 = 0,find2 = 0;
	intermediate_code itm2;

	//cmp1
	if(itm.dst[0] >= '0' && itm.dst[0] <= '9' || itm.dst[0] == '\'' || itm.dst[0] == '-' || itm.dst[0] == '+'){
		mips << "li $t0, " << itm.dst << endl;
	}else{
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.dst){
						find1 = 1;
						mips << "lw $t0, " << 0 - symtab[k].address << "($fp)" << endl;
						break;
					}
				}
				break;
			}
		}
		if(find1 == 0){
			mips << "la $t2, " << itm.dst << endl;
			mips << "lw $t0, 0($t2)" << endl;
		}
	}

	//cmp2
	if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '\'' || itm.src1[0] == '-' || itm.src1[0] == '+'){
		mips << "li $t1, " << itm.src1 << endl;
	}else{
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.src1){
						find2 = 1;
						mips << "lw $t1, " << 0 - symtab[k].address << "($fp)" << endl;
						break;
					}
				}
				break;
			}
		}
		if(find2 == 0){
			mips << "la $t2, " << itm.src1 << endl;
			mips << "lw $t1, 0($t2)" << endl;
		}
	}

	i++;
	itm2 = four[i];
	if(itm2.op == "BNZ"){
		if(itm.op == "<")	mips << "blt $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "<=")	mips << "ble $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == ">")	mips << "bgt $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == ">=")	mips << "bge $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "==")	mips << "beq $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "!=")	mips << "bne $t0, $t1, " << itm2.dst << endl;
	}else{
		if(itm.op == "<")	mips << "bge $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "<=")	mips << "bgt $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == ">")	mips << "ble $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == ">=")	mips << "blt $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "==")	mips << "bne $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "!=")	mips << "beq $t0, $t1, " << itm2.dst << endl;
	}
}

void assign_array_mips(){
	int find = 0;
	int findindex = 0;
	int findsrc = 0;
	//数组名字
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){
					find = 1;
					mips << "move $t0, $fp" << endl;
					mips << "add $t0, $t0, " << 0 - symtab[k].address << endl;
					break;
				}
			}
			break;
		}
	}
	if(find == 0){
		mips << "la $t0, " << itm.dst << endl;
	}

	//数组下标
	for(int j = 0; j < stindex; j++){				//是函数内部的变量，$t2 = index * 4
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.src1){
					findindex = 1;
					mips << "lw $t1, " << 0 - symtab[k].address << "($fp)" << endl;
					mips << "mul $t2, $t1, 4" << endl;
					break;
				}
			}
			break;
		}
	}
	if(findindex == 0){
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '+'){		//下标为数字
			mips << "li $t1, " << itm.src1 << endl;
			mips << "mul $t2, $t1, 4" << endl;
		}
		else{						//下标为全局常量或者变量
			mips << "la $t1, " << itm.src1 << endl;
			mips << "lw $t1, 0($t1)" << endl;
			mips << "mul $t2, $t1, 4" << endl;
		}
	}

	if(find == 0){						//位置.data往上长，sp往下
		mips << "add $t4, $t0, $t2" << endl;
	}else{
		mips << "sub $t4, $t0, $t2" << endl;
	}
	
	//找到赋值内容，$t5存储
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.src2){
					findsrc = 1;
					mips << "lw $t5, " << 0 - symtab[k].address << "($fp)" << endl;
					break;
				}
			}
			break;
		}
	}
	if(findsrc == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '-' || itm.src2[0] == '+'){		//数字
			mips << "li $t5, " << itm.src2 << endl;
		}else if(itm.src2[0] == '\''){	//字符
			mips << "li $t5, " << itm.src2 << endl;
		}else{		//全局常量或变量
			mips << "la $t3, " << itm.src2 << endl;
			mips << "lw $t5, 0($t3)" << endl;
		}
	}

	mips << "sw $t5, 0($t4)" << endl;

}

void use_array_mips(){
	int find = 0;
	int findindex = 0;
	//数组名字
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.src1){
					find = 1;
					mips << "move $t0, $fp" << endl;
					mips << "add $t0, $t0, " << 0 - symtab[k].address << endl;
					break;
				}
			}
			break;
		}
	}
	if(find == 0){
		mips << "la $t0, " << itm.src1 << endl;
	}

	//找到下标
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.src2){
					findindex = 1;
					mips << "lw $t1, " << 0 - symtab[k].address << "($fp)" << endl;
					mips << "mul $t2, $t1, 4" << endl;
					break;
				}
			}
			break;
		}
	}
	if(findindex == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '+'){
			mips << "li $t1, " << itm.src2 << endl;
			mips << "mul $t2, $t1, 4" << endl;
		}else{
			mips << "la $t1, " << itm.src2 << endl;
			mips << "lw $t1, 0($t1)" << endl;
			mips << "mul $t2, $t1, 4" << endl;
		}
	}

	if(find == 0){						//位置.data往上长，sp往下
		mips << "add $t4, $t0, $t2" << endl;
	}else{
		mips << "sub $t4, $t0, $t2" << endl;
	}

	//找到临时寄存器
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){
					mips << "lw $t5, 0($t4)" << endl;
					mips << "sw $t5, " << 0 - symtab[k].address << "($fp)" << endl;
					break;
				}
			}
			break;
		}
	}

}

vector<intermediate_code>::iterator itm1,itm2,itm3;  
void dag(){
	for(itm1 = four.begin(); itm1 != four.end(); ++itm1){
		if(itm1->op != "+" && itm1->op != "-" && itm1->op != "*" && itm1->op != "/")
			continue;
		for(itm2 = itm1 + 1; itm2->op.substr(0,3) != "end"; ++itm2){	
			if(itm2->op == "=" && (itm2->dst == itm1->src1 || itm2->dst == itm1->src2))
				break;
			if(itm2->op == "+" || itm2->op == "-" || itm2->op == "*" || itm2->op == "/"){
				if(itm2->op == itm1->op && itm2->src1 == itm1->src1 && itm2->src2 == itm1->src2){
					for(itm3 = itm2; itm3->op.substr(0,3) != "end"; ++itm3){
						if(itm3->src1 == itm2->dst)
							itm3->src1 = itm1->dst;
						if(itm3->src2 == itm2->dst)
							itm3->src2 = itm1->dst;
					}
					itm2 = four.erase(itm2);
				}
			}
		}
	}
}


string var[1000];
string var2[1000];
int k = 0;
int graph[1000][1000];		//没有初始化

void active_var(){
	for(int n = 0; n < four.size(); n++){		//计算所有函数的个数,并且将所有的函数名都存进fooname数组里面
		if(four[n].src1 == "(" && four[n].src2 == ")"){
			fooname[foocount] = four[n].dst;
			foocount++;
		}
	}

	for(int n = 0; n < 1000; n++){		//初始化存活跃变量的数组
		var[n] = "";
		var2[n] = "";
	}
	int i = 0;
	for(i = four.size() - 1; i >= 0; i--){
		if(four[i].op.substr(0,3) == "end"){
			foocount--;
		}else if(four[i].op == "="){
			int flag = 0;
			for(int j = 0; j < k; j++){			//删除def
				if(var[j] == tostring(foocount) + "." + four[i].dst){
					var[j] = "";
					break;
				}
			}

			if((four[i].src1[0] >= '0' && four[i].src1[0] <= '9') || four[i].src1[0] == '\''){
				continue;
			}
			for(int j = 0; j < k; j++){
				if(var2[j] == tostring(foocount) + "." + four[i].src1){
					flag = 1;
					var[j] = var2[j];
					break;
				}
			}
			if(flag == 0){
				var2[k] = tostring(foocount) + "." + four[i].src1;
				var[k++] = tostring(foocount) + "." + four[i].src1;			//存use，并且冲突图连线
				for(int j = 0; j < k - 1; j++){
					if(var[j] != ""){
						graph[j][k-1] = 1;
						graph[k-1][j] = 1;
					}
				}
			}
		}else if(four[i].op == "[]="){
			int flag = 0;
			for(int j = 0; j < k; j++){			//删除def
				if(var[j] == tostring(foocount) + "." + four[i].dst + "[" + four[i].src1 + "]"){
					var[j] = "";
					break;
				}
			}

			if((four[i].src2[0] >= '0' && four[i].src2[0] <= '9') || four[i].src2[0] == '\''){
				continue;
			}

			for(int j = 0; j < k; j++){
				if(var2[j] == tostring(foocount) + "." + four[i].src2){
					flag = 1;
					var[j] = var2[j];
					break;
				}
			}
			if(flag == 0){
				var2[k] = tostring(foocount) + "." + four[i].src2;
				var[k++] = tostring(foocount) + "." + four[i].src2;			//存use，并且冲突图连线
				for(int j = 0; j < k - 1; j++){
					if(var[j] != ""){
						graph[j][k-1] = 1;
						graph[k-1][j] = 1;
					}
				}
			}
		}else if(four[i].op == "+" || four[i].op == "-" || four[i].op == "*" || four[i].op == "/"){
			int flag = 0;
			for(int j = 0; j < k; j++){			//删除def
				if(var[j] == tostring(foocount) + "." + four[i].dst){
					var[j] = "";
					break;
				}
			}

			if(!((four[i].src1[0] >= '0' && four[i].src1[0] <= '9') || four[i].src1[0] == '\'')){
				for(int j = 0; j < k; j++){
					if(var2[j] == tostring(foocount) + "." + four[i].src1){
						flag = 1;
						var[j] = var2[j];
						break;
					}
				}
				if(flag == 0){
					var2[k] = tostring(foocount) + "." + four[i].src1;
					var[k++] = tostring(foocount) + "." + four[i].src1;			//存use，并且冲突图连线
					for(int j = 0; j < k - 1; j++){
						if(var[j] != ""){
							graph[j][k-1] = 1;
							graph[k-1][j] = 1;
						}
					}
				}
			}
			flag = 0;
			if(!((four[i].src2[0] >= '0' && four[i].src2[0] <= '9') || four[i].src2[0] == '\'')){
				for(int j = 0; j < k; j++){
					if(var2[j] == tostring(foocount) + "." + four[i].src2){
						flag = 1;
						var[j] = var2[j];
						break;
					}
				}
				if(flag == 0){
					var2[k] = tostring(foocount) + "." + four[i].src2;
					var[k++] = tostring(foocount) + "." + four[i].src2;			//存use，并且冲突图连线
					for(int j = 0; j < k - 1; j++){
						if(var[j] != ""){
							graph[j][k-1] = 1;
							graph[k-1][j] = 1;
						}
					}
				}
			}

		}else if(four[i].op == ">" || four[i].op == ">=" || four[i].op == "<" || four[i].op == "<=" || four[i].op == "==" || four[i].op == "!="){
			int flag = 0;
			if(!((four[i].dst[0] >= '0' && four[i].dst[0] <= '9') || four[i].dst[0] == '\'')){
				for(int j = 0; j < k; j++){
					if(var2[j] == tostring(foocount) + "." + four[i].dst){
						flag = 1;
						var[j] = var2[j];
						break;
					}
				}
				if(flag == 0){
					var2[k] = tostring(foocount) + "." + four[i].dst;
					var[k++] = tostring(foocount) + "." + four[i].dst;			//存use，并且冲突图连线
					for(int j = 0; j < k - 1; j++){
						if(var[j] != ""){
							graph[j][k-1] = 1;
							graph[k-1][j] = 1;
						}
					}
				}
			}
			flag = 0;
			if(!((four[i].src1[0] >= '0' && four[i].src1[0] <= '9') || four[i].src1[0] == '\'')){
				for(int j = 0; j < k; j++){
					if(var2[j] == tostring(foocount) + "." + four[i].src1){
						flag = 1;
						var[j] = var2[j];
						break;
					}
				}
				if(flag == 0){
					var2[k] = tostring(foocount) + "." + four[i].src1;
					var[k++] = tostring(foocount) + "." + four[i].src1;			//存use，并且冲突图连线
					for(int j = 0; j < k - 1; j++){
						if(var[j] != ""){
							graph[j][k-1] = 1;
							graph[k-1][j] = 1;
						}
					}
				}
			}
		}else if(four[i].op == "array"){
			int flag = 0;
			for(int j = 0; j < k; j++){			//删除def
				if(var[j] == tostring(foocount) + "." + four[i].dst){
					var[j] = "";
					break;
				}
			}
			for(int j = 0; j < k; j++){
				if(var2[j] == tostring(foocount) + "." + four[i].src1 + "[" + four[i].src2 + "]"){
					flag = 1;
					var[j] = var2[j];
					break;
				}
			}
			if(flag == 0){
				var2[k] = tostring(foocount) + "." + four[i].src1 + "[" + four[i].src2 + "]";	
				var[k++] = tostring(foocount) + "." + four[i].src1 + "[" + four[i].src2 + "]";			//存use，并且冲突图连线
				for(int j = 0; j < k - 1; j++){
					if(var[j] != ""){
						graph[j][k-1] = 1;
						graph[k-1][j] = 1;
					}
				}
			}
		}else if(four[i].op.substr(0,5) == "label"){
			int small,big;
			string tempvar[1000];
			int k1 = 0;
			for(int n = 0; n < 1000; n++){		//初始化临时活跃变量数组
				tempvar[n] = "";
			}

			for(int n = 0; n < four.size(); n++){		//找到BZ或者BNZ
				if((four[n].op == "BZ" || four[n].op == "BNZ") && four[n].dst == four[i].op){
					if(n > i){
						big = n;
						small = i;
					}else{
						big = i;
						small = n;
					}
					break;
				}
			}

			for(int i1 = big; i1 >= small; i1--){			//内部存
				if(four[i1].op == "="){
					for(int j = 0; j < k1; j++){			//删除def
						if(tempvar[j] == tostring(foocount) + "." + four[i1].dst){
							tempvar[j] = "";
							break;
						}
					}

					if((four[i1].src1[0] >= '0' && four[i1].src1[0] <= '9') || four[i1].src1[0] == '\''){
						continue;
					}
					tempvar[k1++] = tostring(foocount) + "." + four[i1].src1;			//存use
					
				}else if(four[i1].op == "[]="){
					for(int j = 0; j < k1; j++){			//删除def
						if(tempvar[j] == tostring(foocount) + "." + four[i1].dst + "[" + four[i1].src1 + "]"){
							tempvar[j] = "";
							break;
						}
					}

					if((four[i1].src2[0] >= '0' && four[i1].src2[0] <= '9') || four[i1].src2[0] == '\''){
						continue;
					}
					tempvar[k1++] = tostring(foocount) + "." + four[i1].src2;			//存use
					
				}else if(four[i1].op == "+" || four[i1].op == "-" || four[i1].op == "*" || four[i1].op == "/"){
					for(int j = 0; j < k1; j++){			//删除def
						if(tempvar[j] == tostring(foocount) + "." + four[i1].dst){
							tempvar[j] = "";
							break;
						}
					}

					if(!((four[i1].src1[0] >= '0' && four[i1].src1[0] <= '9') || four[i1].src1[0] == '\'')){
						tempvar[k1++] = tostring(foocount) + "." + four[i1].src1;			//存use
					}
					if(!((four[i1].src2[0] >= '0' && four[i1].src2[0] <= '9') || four[i1].src2[0] == '\'')){
						tempvar[k1++] = tostring(foocount) + "." + four[i1].src2;			//存use
					}

				}else if(four[i1].op == ">" || four[i1].op == ">=" || four[i1].op == "<" || four[i1].op == "<=" || four[i1].op == "==" || four[i1].op == "!="){
					if(!((four[i1].dst[0] >= '0' && four[i1].dst[0] <= '9') || four[i1].dst[0] == '\'')){
						tempvar[k1++] = tostring(foocount) + "." + four[i1].dst;			//存use
					}
					if(!((four[i1].src1[0] >= '0' && four[i1].src1[0] <= '9') || four[i1].src1[0] == '\'')){
						tempvar[k1++] = tostring(foocount) + "." + four[i1].src1;			//存use
					}
				}else if(four[i1].op == "array"){
					for(int j = 0; j < k1; j++){			//删除def
						if(tempvar[j] == tostring(foocount) + "." + four[i1].dst){
							tempvar[j] = "";
							break;
						}
					}

					tempvar[k1++] = tostring(foocount) + "." + four[i1].src1 + "[" + four[i1].src2 + "]";			//存use
				}
			}

			int varflag = 0;
			for(int n = 0; n < k1; n++){
				for(int m = 0; m < k; m++){
					if(tempvar[n] == var[m]){
						varflag = 1;
						break;
					}	
				}
				if(varflag == 0){
					var2[k] = tempvar[n];
					var[k++] = tempvar[n];
					for(int p = 0; p < k - 1; p++){
						if(var[p] != ""){
							graph[p][k-1] = 1;
							graph[k-1][p] = 1;
						}
					}
				}
			}
		}else{
			continue;
		}
	}
}

//可以使用的寄存器14个  t6 t7 t8 t9 s0-s7 k0 k1
int color_count[1000];
string order[1000];			//移走的节点存储
int reg[1000];			//存储order中对应节点分配的寄存器
int q = 0;
void color_graph(){
	active_var();
	int regnum = 14;		//寄存器个数
	for(int i = 0; i < 1000; i++){
		color_count[i] = 0;
	}
	for(int i = 0; i < k; i++){
		int ccount = 0;
		for(int j = 0; j < k; j++){
			if(graph[j][i] == 1)
				ccount++;
		}
		color_count[i] = ccount;
	}					//冲突图里面每个节点有几条边与之相连


/*	for(int i = 0; i < k; i++){
		cout << var2[i] << " " << color_count[i] << endl;
	}
	cout << "!!!!!!!!!" << endl;*/
	int moveflag = 0;
	for(int i = 0; i < k; i++){
		if(var2[i] != ""){
			moveflag = 0;
			for(int j = 0; j < k; j++){
				if(color_count[j] < regnum && var2[j] != ""){
					order[q++] = var2[j];
					var2[j] = "";
					color_count[j] = 0;
					moveflag = 1;
					for(int n = 0; n < k; n++){
						if(graph[n][j] == 1 && var2[n] != ""){
							color_count[n]--;
						}
					}
					break;
				}
			}

			if(moveflag == 0){
				for(int j = 0; j < k; j++){
					if(var2[j] != ""){
						color_count[j] = 0;
						var2[j] = "";
						for(int n = 0; n < k; n++){
							if(graph[n][j] == 1 && var2[n] != ""){
								color_count[n]--;
							}
						}
						break;
					}
				}
			}
		}	
	}
/*	for(int i = 0; i < q; i++){
		cout << order[i] << endl;
	}
	cout << "########" << endl;*/

	int now;		//现在分配到第几号寄存器
	reg[q-1] = 14;
	now = 14;
	for(int i = q - 2; i >= 0; i--){
		int regflag = 0;	//标记是否已经分配了寄存器
		for(int j = i + 1; j < q; j++){		//判断和已经分配好寄存器的节点有没有连接
			if(graph[i][j] == 0){
				regflag = 1;
				reg[i] = reg[j];
				break;
			}
		}

		if(regflag == 0){		//和已经分配好的节点都有连接，分配给它没有剩下未被使用的寄存器
			if(now < 27){
				reg[i] = ++now;
			}
		}
	}

/*	for(int i = 0; i < q; i++){
		cout << order[i] << " " << reg[i] << endl;
	}*/
	

}



void golbal2();
void push_mips2();
void call_mips2();
void add_mips2();
void sub_mips2();
void mult_mips2();
void div_mips2();
void assign_mips2();
void return_mips2();
void scanf_mips2();
void printf_mips2();
void branch_mips2(int i);
void assign_array_mips2();
void use_array_mips2();

void to_mips2(){
	golbal2();

	//.data处理结束，开始处理 .text部分
	mips2 << ".text" << endl;
	mips2 << "j main" << endl;
	mips2 << "nop" << endl;

	for(int i = 0; i < four.size(); i++){		//遍历四元式
		itm = four[i];
//		mips2 << "##########" << itm.op << " " << itm.dst << " " << itm.src1 << " " << itm.src2 << endl;
		if(itm.src1 == "(" && itm.src2 == ")"){			//这是一个函数
			offset = 0;
			funcname_mips = itm.dst;
			mips2 << itm.dst << ":" << endl;		//函数标签
			if(itm.dst != "main"){
				mips2 << "sub $sp, $sp, 4" << endl;
				mips2 << "sw $ra, 4($sp)" << endl;	//保存$ra
			}										
			mips2 << "move $fp, $sp" << endl;	
			for(int j = 0; j < stindex; j++){
				if(symtab[j].name == itm.dst && symtab[j].kind == 2){
					paracount = symtab[j].paranum;
					return_add = (3 + symtab[j].paranum) * 4 + 4;
					j++;
					for(int k = j; k < stindex; k++){		//将函数内部的常量、变量及形参转换为mips
						if(symtab[k].level == 1){
							if(symtab[k].kind == 0 && symtab[k].type == 0){	//const int
								mips2 << "sub $sp, $sp, 4" << endl;
								mips2 << "li $t0, " << symtab[k].value << endl;
								offset = 0 - symtab[k].address;
								mips2 << "sw $t0, " << offset << "($fp)" << endl;
							}else if(symtab[k].kind == 0 && symtab[k].type == 1){	// const char			!!!!!!!!!! 与 int 相同
								mips2 << "sub $sp, $sp, 4" << endl;
								mips2 << "li $t0, " << symtab[k].value << endl;
								offset = 0 - symtab[k].address;
								mips2 << "sw $t0, " << offset << "($fp)" << endl;
							}else if(symtab[k].kind == 1){				//var
								mips2 << "sub $sp, $sp, 4" << endl;
								offset = offset - 4;
							}else if(symtab[k].kind == 4){				// var array
								mips2 << "sub $sp, $sp, " << symtab[k].paranum * 4 << endl;
								offset = offset - paranum * 4 - 4;
							}else if(symtab[k].kind == 3){			//参数
								mips2 << "sub $sp, $sp, 4" << endl;
								mips2 << "lw $t0, " << (paracount + 3)  * 4 << "($fp)" << endl;
								offset = 0 - symtab[k].address;
								mips2 << "sw $t0, " << offset << "($fp)" << endl;
								offset = offset - 4;
								paracount--;
							}else if(symtab[k].kind == 5){			//临时变量
								mips2 << "sub $sp, $sp, 4" << endl;
								offset = offset - 4;
							}
						}else if(symtab[k].level == 0)
							break;
					}
					break;
				}
			}
		}else if(itm.op == "push"){ 
			push_mips2();
			i++;
			itm = four[i];
			while(itm.op == "push"){
				push_mips2();
				i++;
				itm = four[i];
			}
			i--;
 		}else if(itm.op == "call"){
			call_mips2();
		}else if(itm.op == "para"){
			;						//在上面符号表中已处理
 		}else if(itm.op == "+"){
			add_mips2();
		}else if(itm.op == "-"){
			sub_mips2();
		}else if(itm.op == "return"){
			return_mips2();
		}else if(itm.op == "="){
			assign_mips2();
		}else if(itm.op == "*"){
			mult_mips2();
		}else if(itm.op == "/"){
			div_mips2();
		}else if(itm.op.substr(0,6) == "label_"){
			mips2 << itm.op << ":" << endl;
		}else if(itm.op == "scanf"){
			scanf_mips2();
		}else if(itm.op == "printf"){
			printf_mips2();
		}else if(itm.op == "<" || itm.op == "<=" || itm.op == ">" 
			|| itm.op == ">=" || itm.op == "!=" || itm.op == "=="){
				branch_mips2(i);
		}else if(itm.op == "GOTO"){			
			mips2 << "j " << itm.dst << endl;
		}else if(itm.op.substr(0,10) == "switch_end"){
			mips2 << itm.op << ":" << endl;
		}else if(itm.op == "[]="){
			assign_array_mips2();
		}else if(itm.op == "array"){
			use_array_mips2();
		}else if(itm.op.substr(0,3) == "end"){
			if(itm.op == "end main"){
				mips2 << "li $v0, 10" << endl;
				mips2 << "syscall" << endl;
			}else{
				mips2 << "lw $sp, 12($fp)" << endl;
				mips2 << "lw $ra, 4($fp)" << endl;
				mips2 << "lw $fp,8($fp)" << endl;
				mips2 << "jr $ra" << endl;
				mips2 << "nop" << endl;
			}
		}
	}
}

void golbal2(){
	mips2 << ".data" << endl;

	for(int i = 0; i < stindex; i++){
		if(symtab[i].level == 0){		//全局
			if(symtab[i].kind == 0){	//常量 const
				if(symtab[i].type == 0){	// int
					mips2 << symtab[i].name << ": " << ".word " << symtab[i].value << endl;
				}else if(symtab[i].type == 1){	//char
					mips2 << symtab[i].name << ": " << ".word " << "\'" << char(symtab[i].value) << "\'" << endl;
				}
			}else if(symtab[i].kind == 1){	//变量var
				if(symtab[i].type == 0){	// int
					mips2 << symtab[i].name << ": .space 4" << endl;
				}else if(symtab[i].type == 1){	//char
					mips2 << symtab[i].name << ": .space 4 " << endl;
				}
			}else if(symtab[i].kind == 4){	//数组array
				if(symtab[i].type == 0){	//int
					mips2 << symtab[i].name << ": " << ".space " << symtab[i].paranum * 4 << endl;
				}else if(symtab[i].type == 1){	//char
					mips2 << symtab[i].name << ": " << ".space " << symtab[i].paranum * 4 << endl;
				}
			}
		}		
	}

	//str 字符串打印
	intermediate_code itm;	
	for(int i = 0; i < four.size(); i++){
		itm = four[i];
		if(itm.op == "#string"){
			mips2 << itm.dst << ": .asciiz " << "\""+itm.src1+"\"" << endl;
		}
	}
}

void push_mips2(){

	int find = 0;	//实参为函数内部的-1，全局-0
	mips2 << "sub $sp, $sp, 4" << endl;

	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst && symtab[k].level == 1){
					find = 1;
					mips2 << "lw $t0, " << 0 -symtab[k].address << "($fp)" << endl;
					mips2 << "sw $t0, 4($sp)" << endl;
					break;
				}
			}
			break;
		}
	}
		
	if(find == 0){
		if(itm.dst[0] >= '0' && itm.dst[0] <= '9' || itm.dst[0] == '+' || itm.dst[0] == '-' || itm.dst[0] == '\''){
			mips2 << "li $t0, " << itm.dst << endl;
			mips2 << "sw $t0, 4($sp)" << endl;
		}else{
			mips2 << "lw $t0, " << itm.dst << endl;
			mips2 << "sw $t0, 4($sp)" << endl;
		}
	}
}

void call_mips2(){
	if(itm.src1 == ""){

		mips2 << "li $t0, " << itm.src2 << endl;
		mips2 << "mul $t0, $t0, 4" << endl;
		mips2 << "add $t1, $t0, $sp" << endl;

		mips2 << "sub $sp, $sp, 4" << endl;
		mips2 << "sw $t1, 4($sp)" << endl;
		mips2 << "sub $sp, $sp, 4" << endl;
		mips2 << "sw $fp, 4($sp)" << endl;
		mips2 << "jal " << itm.dst << endl;
		mips2 << "nop" << endl;
		
	}else{
		mips2 << "move $t0, $v0" << endl;
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.dst){
						offset = 0 - symtab[k].address;
						break;
					}
				}
				break;
			}
		}
		mips2 << "sw $t0, " << offset << "($fp)" << endl;	
				
	}
}

void add_mips2(){
	int find1 = 0;
	int find2 = 0;
	int temp_address = 0;
	int fooc = 0;		
	int flag = 0;
	int dstreg = 0;
	for(int i = 0; fooname[i] != ""; i++){
		if(fooname[i] == funcname_mips){
			fooc = i;
			break;
		}
	}
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src1
				if(symtab[k].name == itm.src1){
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.src1 && reg[n] != 0){
							mips2 << "move $t0, $" << reg[n] << endl;
							flag = 1;
							break;
						}
					}
					if(flag == 0)
						mips2 << "lw $t0, " << -symtab[k].address << "($fp)" << endl;
					find1 = 1;
					break;

				}
			}
			flag = 0;
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src2
				if(symtab[k].name == itm.src2){
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.src2 && reg[n] != 0){
							mips2 << "move $t1, $" << reg[n] << endl;
							flag = 1;
							break;
						}
					}
					if(flag == 0)
						mips2 << "lw $t1, " << -symtab[k].address << "($fp)" << endl;
					find2 = 1;
					break;
				}
			}
			flag = 0;
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){							//找到临时变量的相对位置
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.dst && reg[n] != 0){
							dstreg = reg[n];
							flag = 1;
							break;
						}
					}
					temp_address = 0 - symtab[k].address;
					break;
				}
			}
			break;
		}
	}
	if(find1 == 0){				//在局部中没找到，直接调用全局常量或者是立即数
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+')
			mips2 << "li $t0, " << itm.src1 << endl;
		else if(itm.src1[0] == '\''){
			mips2 << "li $t0, " << itm.src1 << endl;
		}else{
			mips2 << "la $t3, " << itm.src1 << endl;
			mips2 << "lw $t0, 0($t3)" << endl;
		}
	}
	if(find2 == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '-' || itm.src2[0] == '+')
			mips2 << "li $t1, " << itm.src2 << endl;
		else if(itm.src2[0] == '\''){
			mips2 << "li $t1, " << itm.src2 << endl;
		}else{
			mips2 << "la $t3, " << itm.src2 << endl;
			mips2 << "lw $t1, 0($t3)" << endl;
		}
	}

	mips2 << "add $t2, $t0, $t1" << endl;
	if(dstreg != 0)
		mips2 << "move $" << dstreg << ", $t2" << endl;
	else
		mips2 << "sw $t2, " << temp_address << "($fp)" << endl;	//将+运算之后的值存在临时变量中，临时变量压栈
}

void sub_mips2(){
	int find1 = 0;
	int find2 = 0;
	int temp_address = 0;
	int fooc = 0;		
	int flag = 0;
	int dstreg = 0;
	for(int i = 0; fooname[i] != ""; i++){
		if(fooname[i] == funcname_mips){
			fooc = i;
			break;
		}
	}
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src1
				if(symtab[k].name == itm.src1){
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.src1 && reg[n] != 0){
							mips2 << "move $t0, $" << reg[n] << endl;
							flag = 1;
							break;
						}
					}
					if(flag == 0)
						mips2 << "lw $t0, " << -symtab[k].address << "($fp)" << endl;
					find1 = 1;
					break;
				}
			}
			flag = 0;
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src2
				if(symtab[k].name == itm.src2){
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.src2 && reg[n] != 0){
							mips2 << "move $t1, $" << reg[n] << endl;
							flag = 1;
							break;
						}
					}
					if(flag == 0)
						mips2 << "lw $t1, " << -symtab[k].address << "($fp)" << endl;
					find2 = 1;
					break;
				}
			}
			flag = 0;
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){							//找到临时变量的相对位置
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.dst && reg[n] != 0){
							dstreg = reg[n];
							flag = 1;
							break;
						}
					}
					temp_address = 0 - symtab[k].address;
					break;
				}
			}
			break;
		}
	}
	if(find1 == 0){				//在局部中没找到，直接调用全局常量或者是立即数
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+')
			mips2 << "li $t0, " << itm.src1 << endl;
		else if(itm.src1[0] == '\''){
			mips2 << "li $t0, " << itm.src1 << endl;
		}else{
			mips2 << "la $t3, " << itm.src1 << endl;
			mips2 << "lw $t0, 0($t3)" << endl;
		}
	}
	if(find2 == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '-' || itm.src2[0] == '+')
			mips2 << "li $t1, " << itm.src2 << endl;
		else if(itm.src2[0] == '\''){
			mips2 << "li $t1, " << itm.src2 << endl;
		}else{
			mips2 << "la $t3, " << itm.src2 << endl;
			mips2 << "lw $t1, 0($t3)" << endl;
		}
	}

	mips2 << "sub $t2, $t0, $t1" << endl;
	if(dstreg != 0)
		mips2 << "move $" << dstreg << ", $t2" << endl;
	else
		mips2 << "sw $t2, " << temp_address << "($fp)" << endl;	//将-运算之后的值存在临时变量中，临时变量压栈
}

void mult_mips2(){
	int find1 = 0;
	int find2 = 0;
	int temp_address = 0;
	int fooc = 0;		
	int flag = 0;
	int dstreg = 0;
	for(int i = 0; fooname[i] != ""; i++){
		if(fooname[i] == funcname_mips){
			fooc = i;
			break;
		}
	}
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src1
				if(symtab[k].name == itm.src1){
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.src1 && reg[n] != 0){
							mips2 << "move $t0, $" << reg[n] << endl;
							flag = 1;
							break;
						}
					}
					if(flag == 0)
						mips2 << "lw $t0, " << -symtab[k].address << "($fp)" << endl;
					find1 = 1;
					break;
				}
			}
			flag = 0;
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src2
				if(symtab[k].name == itm.src2){
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.src2 && reg[n] != 0){
							mips2 << "move $t1, $" << reg[n] << endl;
							flag = 1;
							break;
						}
					}
					if(flag == 0)
						mips2 << "lw $t1, " << -symtab[k].address << "($fp)" << endl;
					find2 = 1;
					break;
				}
			}
			flag = 0;
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){							//找到临时变量的相对位置
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.dst && reg[n] != 0){
							dstreg = reg[n];
							flag = 1;
							break;
						}
					}
					temp_address = 0 - symtab[k].address;
					break;
				}
			}
			break;
		}
	}
	if(find1 == 0){				//在局部中没找到，直接调用全局常量或者是立即数
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+' || itm.src1[0] == '\'')
			mips2 << "li $t0, " << itm.src1 << endl;
		else{
			mips2 << "la $t3, " << itm.src1 << endl;
			mips2 << "lw $t0, 0($t3)" << endl;
		}
	}
	if(find2 == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '-' || itm.src2[0] == '+' || itm.src2[0] == '\'')
			mips2 << "li $t1, " << itm.src2 << endl;
		else{
			mips2 << "la $t3, " << itm.src2 << endl;
			mips2 << "lw $t1, 0($t3)" << endl;
		}
	}

	mips2 << "mul $t2, $t0, $t1" << endl;
	if(dstreg != 0)
		mips2 << "move $" << dstreg << ", $t2" << endl;
	else
		mips2 << "sw $t2, " << temp_address << "($fp)" << endl;	//将-运算之后的值存在临时变量中，临时变量压栈
}

void div_mips2(){
	int find1 = 0;
	int find2 = 0;
	int temp_address = 0;
	int fooc = 0;		
	int flag = 0;
	int dstreg = 0;
	for(int i = 0; fooname[i] != ""; i++){
		if(fooname[i] == funcname_mips){
			fooc = i;
			break;
		}
	}
//	mips2 << "sub $sp, $sp, 4" << endl;
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src1
				if(symtab[k].name == itm.src1){
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.src1 && reg[n] != 0){
							mips2 << "move $t0, $" << reg[n] << endl;
							flag = 1;
							break;
						}
					}
					if(flag == 0)
						mips2 << "lw $t0, " << -symtab[k].address << "($fp)" << endl;
					find1 = 1;
					break;
				}
			}
			flag = 0;
			for(int k = j + 1; symtab[k].level != 0; k++){				//find src2
				if(symtab[k].name == itm.src2){
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.src2 && reg[n] != 0){
							mips2 << "move $t1, $" << reg[n] << endl;
							flag = 1;
							break;
						}
					}
					if(flag == 0)
						mips2 << "lw $t1, " << -symtab[k].address << "($fp)" << endl;
					find2 = 1;
					break;
				}
			}
			flag = 0;
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){							//找到临时变量的相对位置
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.dst && reg[n] != 0){
							dstreg = reg[n];
							flag = 1;
							break;
						}
					}
					temp_address = 0 - symtab[k].address;
					break;
				}
			}
			break;
		}
	}
	if(find1 == 0){				//在局部中没找到，直接调用全局常量或者是立即数
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+' || itm.src1[0] == '\'')
			mips2 << "li $t0, " << itm.src1 << endl;
		else{
			mips2 << "la $t3, " << itm.src1 << endl;
			mips2 << "lw $t0, 0($t3)" << endl;
		}
	}
	if(find2 == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '-' || itm.src2[0] == '+' || itm.src2[0] == '\'')
			mips2 << "li $t1, " << itm.src2 << endl;
		else{
			mips2 << "la $t3, " << itm.src2 << endl;
			mips2 << "lw $t1, 0($t3)" << endl;
		}
	}

	mips2 << "div $t2, $t0, $t1" << endl;
	if(dstreg != 0)
		mips2 << "move $" << dstreg << ", $t2" << endl;
	else
		mips2 << "sw $t2, " << temp_address << "($fp)" << endl;	//将-运算之后的值存在临时变量中，临时变量压栈
}

void assign_mips2(){
	int find1 = 0;		//标记在函数内部找到了赋值的对象-1
	int find2 = 0;		//标记赋的值是临时变量，不是全局常量或变量-1
	int fooc = 0;		
	int flag = 0;
	for(int i = 0; fooname[i] != ""; i++){
		if(fooname[i] == funcname_mips){
			fooc = i;
			break;
		}
	}
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.src1){
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.src1 && reg[n] != 0){
							mips2 << "move $t0, $" << reg[n] << endl;
							flag = 1;
							break;
						}
					}
					if(flag == 0)
						mips2 << "lw $t0, " << 0 - symtab[k].address << "($fp)" << endl;			//取出赋的值
					find2 = 1;
					break;
				}
			}
			break;
		}
	}
	flag = 0;
	if(find2 == 0){
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+')
			mips2 << "li $t0, " << itm.src1 << endl;
		else if(itm.src1[0] == '\''){
			mips2 << "li $t0, " << itm.src1 << endl;
		}else{
			mips2 << "la $t3, " << itm.src1 << endl;
			mips2 << "lw $t0, 0($t3)" << endl;
		}
	}


	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){
					for(int n = 0; n < q; n++){
						if(order[n] == tostring(fooc) + "." + itm.dst && reg[n] != 0){
							mips2 << "move $t0, $" << reg[n] << endl;
							flag = 1;
							break;
						}
					}
					if(flag == 0)
						mips2 << "sw $t0, " << 0 -symtab[k].address << "($fp)" << endl;			//找到赋值的对象
					find1 = 1;
					break;
				}
			}
			break;
		}
	}

	if(find1 == 0){
		mips2 << "sw $t0, " << itm.dst << endl;
	}
}

void return_mips2(){
	int find = 0;
	if(itm.dst == ""){
		;
	}else{
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.dst){
						find = 1;
						mips2 << "lw $v0, " << 0 - symtab[k].address << "($fp)" << endl;			//取出赋的值	
						break;
					}
				}
				break;
			}
		}

		if(find == 0){
			if(itm.dst[0] >= '0' && itm.dst[0] <= '9' || itm.dst[0] == '-' || itm.dst[0] == '+'){
				mips2 << "li $v0, " << itm.dst << endl;
			}else if(itm.dst[0] == '\''){
				mips2 << "li $v0, " << itm.dst << endl;
			}else{
				mips2 << "la $t0, " << itm.dst << endl;
				mips2 << "lw $v0, 0($t0)" << endl;
			}
		}
	}
	if(funcname_mips == "main"){
		mips2 << "li $v0, 10" << endl;
		mips2 << "syscall" << endl;
	}else{
		mips2 << "lw $sp, 12($fp)" << endl;
		mips2 << "lw $ra, 4($fp)" << endl;
		mips2 << "lw $fp,8($fp)" << endl;
		mips2 << "jr $ra" << endl;
		mips2 << "nop" << endl;
	}
}

void scanf_mips2(){
	int find = 0;		//在函数局部中找到-1， 全局-0
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){
					if(symtab[k].type == 0){
						mips2 << "li $v0, 5" << endl;
						mips2 << "syscall" << endl;
						mips2 << "sw $v0, " << 0 - symtab[k].address << "($fp)" << endl;
						find = 1;
					}else if(symtab[k].type == 1){
						mips2 << "li $v0, 12" << endl;
						mips2 << "syscall" << endl;
						mips2 << "sw $v0, " << 0 - symtab[k].address << "($fp)" << endl;
						find = 1;
					}
					break;
				}
			}
			break;
		}
	}

	if(find == 0){
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == itm.dst && symtab[j].type == 0){
				mips2 << "li $v0, 5" << endl;
				mips2 << "syscall" << endl;
				mips2 << "la $t0, " << itm.dst << endl;
				mips2 << "sw $v0, 0($t0)" << endl;
				break;
			}else if(symtab[j].name == itm.dst && symtab[j].type == 1){
				mips2 << "li $v0, 12" << endl;
				mips2 << "syscall" << endl;
				mips2 << "la $t0, " << itm.dst << endl;
				mips2 << "sw $v0, 0($t0)" << endl;
				break;
			}
		}
	}
}

void printf_mips2(){
	int find = 0;
	int fooc = 0;		
	int flag = 0;
	for(int i = 0; fooname[i] != ""; i++){
		if(fooname[i] == funcname_mips){
			fooc = i;
			break;
		}
	}
	if(itm.src1 == "" && itm.dst.substr(0,3) == "str"){		//只输出字符串
		mips2 << "li $v0, 4" << endl;
		mips2 << "la $a0, " << itm.dst << endl;
		mips2 << "syscall" << endl;
	}else if(itm.src1 != "" && itm.dst.substr(0,3) == "str"){	//输出字符串+表达式
		mips2 << "li $v0, 4" << endl;
		mips2 << "la $a0, " << itm.dst << endl;
		mips2 << "syscall" << endl;

//		mips2 << "li $v0, 1" << endl;
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.src1){
						find = 1;
						if(symtab[k].type == 0){
							mips2 << "li $v0, 1" << endl;
							for(int n = 0; n < q; n++){
								if(order[n] == tostring(fooc) + "." + itm.src1 && reg[n] != 0){
									mips2 << "move $a0, $" << reg[n] << endl;
									flag = 1;
									break;
								}
							}
							if(flag == 0)
								mips2 << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}else if(symtab[k].type == 1){
							mips2 << "li $v0, 11" << endl;
							for(int n = 0; n < q; n++){
								if(order[n] == tostring(fooc) + "." + itm.src1 && reg[n] != 0){
									mips2 << "move $a0, $" << reg[n] << endl;
									flag = 1;
									break;
								}
							}
							if(flag == 0)
								mips2 << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}else if(symtab[k].type == -1){			///////////
							mips2 << "li $v0, 1" << endl;
							for(int n = 0; n < q; n++){
								if(order[n] == tostring(fooc) + "." + itm.src1 && reg[n] != 0){
									mips2 << "move $a0, $" << reg[n] << endl;
									flag = 1;
									break;
								}
							}
							if(flag == 0)
								mips2 << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}
 						
						break;
					}
				}
				break;
			}
		}
		if(find == 0){
			if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+'){
				mips2 << "li $v0, 1" << endl;
				mips2 << "li $a0," << itm.src1 << endl;
			}else if(itm.src1[0] == '\''){
				mips2 << "li $v0, 11" << endl;
				mips2 << "li $a0, " << itm.src1 << endl;
			}else{
				for(int j = 0; j < stindex; j++){
					if(symtab[j].name == itm.src1){
						if(symtab[j].type == 0){
							mips2 << "li $v0, 1" << endl;
						}else if(symtab[j].type == 1){
							mips2 << "li $v0, 11" << endl;
						}
						break;
					}
				}
				mips2 << "la $t0, "<< itm.src1 << endl;
				mips2 << "lw $a0, 0($t0)" << endl;
			}
		}
		mips2 << "syscall" << endl;
	}else{			//输出表达式
	//	mips2 << "li $v0, 1" << endl;
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.src1){
						find = 1;
						if(symtab[k].type == 0){
							mips2 << "li $v0, 1" << endl;
							for(int n = 0; n < q; n++){
								if(order[n] == tostring(fooc) + "." + itm.src1 && reg[n] != 0){
									mips2 << "move $t0, $" << reg[n] << endl;
									flag = 1;
									break;
								}
							}
							if(flag == 0)
								mips2 << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}else if(symtab[k].type == 1){
							mips2 << "li $v0, 11" << endl;
							for(int n = 0; n < q; n++){
								if(order[n] == tostring(fooc) + "." + itm.src1 && reg[n] != 0){
									mips2 << "move $t0, $" << reg[n] << endl;
									flag = 1;
									break;
								}
							}
							if(flag == 0)
								mips2 << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}else if(symtab[k].type == -1){			///////////
							mips2 << "li $v0, 1" << endl;
							for(int n = 0; n < q; n++){
								if(order[n] == tostring(fooc) + "." + itm.src1 && reg[n] != 0){
									mips2 << "move $t0, $" << reg[n] << endl;
									flag = 1;
									break;
								}
							}
							if(flag == 0)
								mips2 << "lw $a0, " << 0 - symtab[k].address << "($fp)" << endl;
						}
						break;
					}
				}
				break;
			}
		}
		if(find == 0){
			if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '-' || itm.src1[0] == '+'){
				mips2 << "li $v0, 1" << endl;
				mips2 << "li $a0," << itm.src1 << endl;
			}else if(itm.src1[0] == '\''){
				mips2 << "li $v0, 11" << endl;
				mips2 << "li $a0, " << itm.src1 << endl;
			}else{
				for(int j = 0; j < stindex; j++){
					
					if(symtab[j].name == itm.src1){
						if(symtab[j].type == 0){
							mips2 << "li $v0, 1" << endl;
						}else if(symtab[j].type == 1){
							mips2 << "li $v0, 11" << endl;
						}
						break;
					}
				}
				mips2 << "la $t0, "<< itm.src1 << endl;
				mips2 << "lw $a0, 0($t0)" << endl;
			}
		}
		mips2 << "syscall" << endl;
	}
}

void branch_mips2(int i){
	int find1 = 0,find2 = 0;
	intermediate_code itm2;

	//cmp1
	if(itm.dst[0] >= '0' && itm.dst[0] <= '9' || itm.dst[0] == '\'' || itm.dst[0] == '-' || itm.dst[0] == '+'){
		mips2 << "li $t0, " << itm.dst << endl;
	}else{
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.dst){
						find1 = 1;
						mips2 << "lw $t0, " << 0 - symtab[k].address << "($fp)" << endl;
						break;
					}
				}
				break;
			}
		}
		if(find1 == 0){
			mips2 << "la $t2, " << itm.dst << endl;
			mips2 << "lw $t0, 0($t2)" << endl;
		}
	}

	//cmp2
	if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '\'' || itm.src1[0] == '-' || itm.src1[0] == '+'){
		mips2 << "li $t1, " << itm.src1 << endl;
	}else{
		for(int j = 0; j < stindex; j++){
			if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
				for(int k = j + 1; symtab[k].level != 0; k++){
					if(symtab[k].name == itm.src1){
						find2 = 1;
						mips2 << "lw $t1, " << 0 - symtab[k].address << "($fp)" << endl;
						break;
					}
				}
				break;
			}
		}
		if(find2 == 0){
			mips2 << "la $t2, " << itm.src1 << endl;
			mips2 << "lw $t1, 0($t2)" << endl;
		}
	}

	i++;
	itm2 = four[i];
	if(itm2.op == "BNZ"){
		if(itm.op == "<")	mips2 << "blt $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "<=")	mips2 << "ble $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == ">")	mips2 << "bgt $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == ">=")	mips2 << "bge $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "==")	mips2 << "beq $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "!=")	mips2 << "bne $t0, $t1, " << itm2.dst << endl;
	}else{
		if(itm.op == "<")	mips2 << "bge $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "<=")	mips2 << "bgt $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == ">")	mips2 << "ble $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == ">=")	mips2 << "blt $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "==")	mips2 << "bne $t0, $t1, " << itm2.dst << endl;
		else if(itm.op == "!=")	mips2 << "beq $t0, $t1, " << itm2.dst << endl;
	}
}

void assign_array_mips2(){
	int find = 0;
	int findindex = 0;
	int findsrc = 0;
	//数组名字
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){
					find = 1;
					mips2 << "move $t0, $fp" << endl;
					mips2 << "add $t0, $t0, " << 0 - symtab[k].address << endl;
					break;
				}
			}
			break;
		}
	}
	if(find == 0){
		mips2 << "la $t0, " << itm.dst << endl;
	}

	//数组下标
	for(int j = 0; j < stindex; j++){				//是函数内部的变量，$t2 = index * 4
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.src1){
					findindex = 1;
					mips2 << "lw $t1, " << 0 - symtab[k].address << "($fp)" << endl;
					mips2 << "mul $t2, $t1, 4" << endl;
					break;
				}
			}
			break;
		}
	}
	if(findindex == 0){
		if(itm.src1[0] >= '0' && itm.src1[0] <= '9' || itm.src1[0] == '+'){		//下标为数字
			mips2 << "li $t1, " << itm.src1 << endl;
			mips2 << "mul $t2, $t1, 4" << endl;
		}
		else{						//下标为全局常量或者变量
			mips2 << "la $t1, " << itm.src1 << endl;
			mips2 << "lw $t1, 0($t1)" << endl;
			mips2 << "mul $t2, $t1, 4" << endl;
		}
	}

	if(find == 0){						//位置.data往上长，sp往下
		mips2 << "add $t4, $t0, $t2" << endl;
	}else{
		mips2 << "sub $t4, $t0, $t2" << endl;
	}
	
	//找到赋值内容，$t5存储
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.src2){
					findsrc = 1;
					mips2 << "lw $t5, " << 0 - symtab[k].address << "($fp)" << endl;
					break;
				}
			}
			break;
		}
	}
	if(findsrc == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '-' || itm.src2[0] == '+'){		//数字
			mips2 << "li $t5, " << itm.src2 << endl;
		}else if(itm.src2[0] == '\''){	//字符
			mips2 << "li $t5, " << itm.src2 << endl;
		}else{		//全局常量或变量
			mips2 << "la $t3, " << itm.src2 << endl;
			mips2 << "lw $t5, 0($t3)" << endl;
		}
	}

	mips2 << "sw $t5, 0($t4)" << endl;

}

void use_array_mips2(){
	int find = 0;
	int findindex = 0;
	//数组名字
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.src1){
					find = 1;
					mips2 << "move $t0, $fp" << endl;
					mips2 << "add $t0, $t0, " << 0 - symtab[k].address << endl;
					break;
				}
			}
			break;
		}
	}
	if(find == 0){
		mips2 << "la $t0, " << itm.src1 << endl;
	}

	//找到下标
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.src2){
					findindex = 1;
					mips2 << "lw $t1, " << 0 - symtab[k].address << "($fp)" << endl;
					mips2 << "mul $t2, $t1, 4" << endl;
					break;
				}
			}
			break;
		}
	}
	if(findindex == 0){
		if(itm.src2[0] >= '0' && itm.src2[0] <= '9' || itm.src2[0] == '+'){
			mips2 << "li $t1, " << itm.src2 << endl;
			mips2 << "mul $t2, $t1, 4" << endl;
		}else{
			mips2 << "la $t1, " << itm.src2 << endl;
			mips2 << "lw $t1, 0($t1)" << endl;
			mips2 << "mul $t2, $t1, 4" << endl;
		}
	}

	if(find == 0){						//位置.data往上长，sp往下
		mips2 << "add $t4, $t0, $t2" << endl;
	}else{
		mips2 << "sub $t4, $t0, $t2" << endl;
	}

	//找到临时寄存器
	for(int j = 0; j < stindex; j++){
		if(symtab[j].name == funcname_mips && symtab[j].kind == 2){
			for(int k = j + 1; symtab[k].level != 0; k++){
				if(symtab[k].name == itm.dst){
					mips2 << "lw $t5, 0($t4)" << endl;
					mips2 << "sw $t5, " << 0 - symtab[k].address << "($fp)" << endl;
					break;
				}
			}
			break;
		}
	}

}


int main(){
	cout << "请输入文件路径：";
	cin >> in;
//	in = "E:/test.txt";
	infile.open(in);
	ch = infile.get();
	getsym();
	program();

	intermediate_code itm;
	for(int i = 0; i < four.size(); i++){
		itm = four[i];
		out_four << itm.op << " " << itm.dst << " " << itm.src1 << " " << itm.src2 << endl;
	}

	to_mips();

//	dag();
//	color_graph();
//	to_mips2();

	for(int i = 0; i < four.size(); i++){
		itm = four[i];
		out_four2 << itm.op << " " << itm.dst << " " << itm.src1 << " " << itm.src2 << endl;
	}
	infile.close();
	outfile.close();
}

