#include<iostream>
#include<fstream>
#include<string>
using namespace std;

string keyword[13]={"const","int","char","void","main","if","do",
	"while","switch","case","scanf","printf","return"};

string keyword_symbol[13]={"CONSTSY","INTSY","CHARSY","VOIDSY","MAINSY",
	"IFSY","DOSY","WHILESY","SWITCHSY","CASESY","SCANFSY","PRINTFSY","RETURNSY"};

char token[200]={};//��ŵ��ʵ��ַ���
int num;//��ŵ�ǰ���������ֵ
char ch;//��ŵ�ǰ�������ַ�
string in;
ifstream infile;
ofstream outfile("E:/result.txt", ios::out);
int cnt = 1;//���ʼ�����
int errflag = 0; 

string sym;//ȡ�ʼ����
string str; //����ȡ��