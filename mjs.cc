#define DEBUG_LEXER true

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <string>
using namespace std;

#include "mjs.h"

// 词法分析产生的临时变量
string tmpID;
double tmpNumVal;

// 当前解析到的行数
int current_row = 1;

// 虚拟机堆栈
stack<uint8_t> dataStack;

// 弹栈
template <typename T>
T Pop() {
  int n = sizeof(T);
  uint8_t buf[n];
  while (n--) {
    buf[n] = dataStack.top();
    dataStack.pop();
  }
  return fromBytes<T>(buf);
}

// 压栈
template <typename T>
void Push(T x) {
  const int SIZE = sizeof(T);
  int n = SIZE;
  uint8_t *buf = toBytes<T>(x);
  while (n--) {
    dataStack.push(buf[SIZE - n - 1]);
  }
  free(buf);
}

#define PushNum Push<double>
#define PopNum Pop<double>

map<OpCode, function<void()>> dispatchTable = {
    {OP_ADD, []() -> void { PushNum(PopNum() + PopNum()); }},
    {OP_SUB, []() -> void { PushNum(-PopNum() + PopNum()); }},
    {OP_MUL, []() -> void { PushNum(PopNum() * PopNum()); }},
    {OP_DIV, []() -> void { PushNum(1.0 / PopNum() * PopNum()); }},
};

Token getToken(FILE *fp) {
  char ch = fgetc(fp);
  // 忽略空格
  while (isspace(ch)) {
    if (ch == '\n') current_row++;
    ch = fgetc(fp);
  }
  // 解析符号 [=+-*/]
  if (strchr("=+-*/;", ch)) {
    switch (ch) {
      case '=': return TOKEN_EQUAL;
      case '+': return TOKEN_PLUS;
      case '-': return TOKEN_MINUS;
      case '*': return TOKEN_STAR;
      case '/': return TOKEN_SLASH;
      case ';': return TOEKN_SEMI;
      default: break;
    }
  }
  // 解析标识符和关键字 [a-zA-Z_$][a-zA-Z0-9_$]
  if (isalpha(ch) || ch == '_' || ch == '$') {
    stringstream buf;
    buf << ch;
    char next = fgetc(fp);
    while (isalnum(next) || next == '_' || next == '$') {
      ch = next;
      buf << next;
      next = fgetc(fp);
    }
    fseek(fp, -1L, SEEK_CUR);
    string name = buf.str();
    if (!name.compare("var")) {
      return TOKEN_VAR;
    } else {
      tmpID = buf.str();
      return TOKEN_ID;
    }
  }
  // 解析数字
  if (isdigit(ch) || ch == '.') {
    stringstream buf;
    buf << ch;
    int dot = (ch != '.');
    char next = fgetc(fp);
    while (isdigit(next) || (next == '.' && dot--)) {
      ch = next;
      buf << next;
      next = fgetc(fp);
    }
    fseek(fp, -1L, SEEK_CUR);
    tmpNumVal = stod(buf.str(), NULL);
    return TOKEN_NUM;
  }
  // 处理结束符
  if (feof(fp)) return TOKEN_EOF;
  return TOKEN_UNKNOWN;
}

void parseVarStatement(FILE *fp) {
  // 解析标识符
  Token token = getToken(fp);
  if (token != TOKEN_ID) {
    printf("[ERROR] lines %d: no [ID] next to 'var'.\n", current_row);
    exit(1);
  }
  string id = tmpID;
  // 解析等号
  token = getToken(fp);
  if (token != TOKEN_EQUAL) {
    printf("[ERROR] lines %d: no '=' next to [ID].\n", current_row);
    exit(1);
  }
  // 解析右边
  token = getToken(fp);
}

void parse(FILE *fp) {
  while (!feof(fp)) {
    Token token = getToken(fp);

    if (DEBUG_LEXER) {
      cout << "[DEBUG] LEXER: " << _V(token);
      if (token == TOKEN_ID) cout << "\t" << tmpID;
      if (token == TOKEN_NUM) cout << "\t" << tmpNumVal;
      cout << endl;
    }

    switch (token) {
      case TOKEN_VAR: parseVarStatement(fp); break;
      default: break;
    }
  }
}

int main(int argc, char **argv) {
  char *filename = argv[1];
  FILE *fp = fopen(filename, "r");
  parse(fp);
  fclose(fp);
  return 0;
}
