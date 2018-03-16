/*	标识符不区分大小写
	输出单词类别码(记忆符表示)，单词值
	保留字：const,int,char,void,main,if,do,while,switch,case,scanf,printf,return 
	分界符或操作符：+ - * / < <= > >= != == = ( ) [ ] { } , ; :
*/

#include "getsym.h"

bool isSpace(char ch){
	if(ch == ' ')
		return true;
	return false;
}

bool isNewLine(char ch){
	if(ch == '\n')
		return true;
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
				cout << "出现前0" << endl;
			}
		}
		while(isDigit(ch)){
			catToken(token,ch);
			ch = infile.get();
		}
		num = transNUM(token);
		sym = "INTEGERSY";
		str = num;
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
		if(ch == '+' || ch == '-' || ch == '*' || ch == '/' || (ch >= 'A' && ch <= 'Z') 
			|| (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')){
			catToken(token,ch);
			ch = infile.get();
			if(ch == '\''){
				sym = "CHARACTERSY";
				for(int j = 0; token[j] != NULL; j++){
					str += token[j];
				}
				outfile << cnt << " CHARACTERSY " << "\'" << token << "\'" << endl;
			}else{
				outfile << "error!" << endl;
				while(ch != '\'')
					ch = infile.get();
			}
		}else{
			outfile << "error!" << endl;
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
			sym = "STRINGSY";
			for(int j = 0; token[j] != NULL; j++){
				str += token[j];
			}
			outfile << cnt << " STRINGSY " << "\"" << token << "\"" << endl;
			ch = infile.get();
		}else{
			outfile << "error!" << endl;
		}
	}
}