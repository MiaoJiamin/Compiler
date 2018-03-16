#include<iostream>
#include<fstream>
#include<string>
using namespace std;

extern char ch;
extern int errflag;
extern string sym;
extern int cnt;
extern string str;
extern int num;
extern string in;
extern ifstream infile;
extern ofstream outfile("E:/result.txt", ios::out);

extern void getsym();
extern bool isSpace(char ch);
extern bool isTab(char ch);
extern bool isNewLine(char ch);