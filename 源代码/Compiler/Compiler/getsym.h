#include<iostream>
#include<fstream>
#include<string>
using namespace std;

string keyword[13]={"const","int","char","void","main","if","do",
	"while","switch","case","scanf","printf","return"};

string keyword_symbol[13]={"CONSTSY","INTSY","CHARSY","VOIDSY","MAINSY",
	"IFSY","DOSY","WHILESY","SWITCHSY","CASESY","SCANFSY","PRINTFSY","RETURNSY"};

char token[200]={};//存放单词的字符串
int num;//存放当前读入的整数值
char ch;//存放当前读进的字符
string in;
ifstream infile;
ofstream outfile("E:/result.txt", ios::out);
int cnt = 1;//单词计数器
int errflag = 0; 

string sym;//取词记忆符
string str; //具体取词