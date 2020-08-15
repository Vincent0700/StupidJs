#define DEBUG_LEXER false

#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
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

// 全局作用域
Scope global;

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

#define PushCode Push<int8_t>
#define PopCode Pop<int8_t>
#define PushVal Push<JsValue *>
#define PopVal Pop<JsValue *>

map<OpCode, function<void()>> dispatchTable = {
    {OP_ASSIGN,
     []() -> void {
       JsValue *val = PopVal();
       JsValue *_name = PopVal();
       string name((char *)(_name->val.ptr));
       global.set(name, val);
     }},
    {OP_PRINT, []() -> void {
       JsValue *_name = PopVal();
       string name((char *)(_name->val.ptr));
       JsValue *_val = global.get(name);
       double val = _val->val.float64;
       cout.precision(numeric_limits<double>::max_digits10);
       cout << val << endl;
     }}};

void run() {
  while (!dataStack.empty()) {
    OpCode code = (OpCode)PopCode();
    auto func = dispatchTable[code];
    func();
  }
}

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
    } else if (!name.compare("$print")) {
      return TOKEN_PRINT;
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
  // 解析括号
  switch (ch) {
    case '(': return TOKEN_LBRACKET;
    case ')': return TOKEN_RBRACKET;
    default: break;
  }
  // 处理结束符
  if (feof(fp)) return TOKEN_EOF;
  return TOKEN_UNKNOWN;
}

// 解析表达式
void parseExpression(FILE *fp) {
  stack<double> numStack;
  stack<OpCode> opStack;
  map<int, OpCode> toOpCode = {
      {TOKEN_PLUS, OP_ADD},
      {TOKEN_MINUS, OP_SUB},
      {TOKEN_STAR, OP_MUL},
      {TOKEN_SLASH, OP_DIV},
  };
  map<OpCode, int> P = {
      {OP_ADD, 1},
      {OP_SUB, 1},
      {OP_MUL, 2},
      {OP_DIV, 2},
  };

  Token token = getToken(fp);
  auto calc = [&](OpCode op) {
    opStack.pop();
    double v2 = numStack.top();
    numStack.pop();
    double v1 = numStack.top();
    numStack.pop();
    switch (op) {
      case OP_ADD: return v1 + v2;
      case OP_SUB: return v1 - v2;
      case OP_MUL: return v1 * v2;
      case OP_DIV: return v1 / v2;
      default: return 0.0;
    }
  };

  while (token != TOEKN_SEMI) {
    if (token == TOKEN_NUM) {
      // 如果是操作数，直接压入执行栈
      numStack.push(tmpNumVal);
    } else if (token == TOKEN_ID) {
      // 如果是标识符，从作用域读取值然后压入执行栈
      JsValue *value = global.get(tmpID);
      double num = value->val.float64;
      numStack.push(num);
    } else if (token == TOKEN_PLUS || token == TOKEN_MINUS ||
               token == TOKEN_STAR || token == TOKEN_SLASH) {
      // 如果是操作符，栈为空或者优先级>栈顶的优先级，直接压如op栈，否则依次追加到执行栈
      OpCode op = toOpCode[token];
      if (opStack.empty() || P[op] > P[opStack.top()]) {
        opStack.push(op);
      } else {
        while (!opStack.empty()) {
          OpCode op = opStack.top();
          double result = calc(op);
          numStack.push(result);
        }
        fseek(fp, -1L, SEEK_CUR);
      }
    }
    token = getToken(fp);
  }

  // 循环结束，如果op栈还有操作符，也依次压入执行栈
  while (!opStack.empty()) {
    OpCode op = opStack.top();
    double result = calc(op);
    numStack.push(result);
  }

  JsValue *ret = new JsValue(JS_TAG_NUM, numStack.top());
  PushVal(ret);
}

// 解析赋值语句
void parseVarStatement(FILE *fp) {
  // 解析标识符
  Token token = getToken(fp);
  if (token != TOKEN_ID) {
    printf("[ERROR] lines %d: no [ID] next to 'var'.\n", current_row);
    exit(1);
  }
  string _name = tmpID;
  char *ptr_name = new char[_name.length()];
  strcpy(ptr_name, _name.c_str());
  JsValue *name = new JsValue(JS_TAG_STRING, (void *)ptr_name);
  PushVal(name);
  // 解析等号
  token = getToken(fp);
  if (token != TOKEN_EQUAL) {
    printf("[ERROR] lines %d: no '=' next to [ID].\n", current_row);
    exit(1);
  }
  // 解析右边表达式
  parseExpression(fp);
  PushCode(OP_ASSIGN);
}

// 解析PRINT语句
void parsePrintStatement(FILE *fp) {
  // 左括号
  Token token = getToken(fp);
  if (token != TOKEN_LBRACKET) {
    printf("[ERROR] lines %d: no '(' next to '$print'.\n", current_row);
    exit(1);
  }
  // 标识符
  token = getToken(fp);
  if (token == TOKEN_ID) {
    char *ptr = new char[tmpID.length()];
    strcpy(ptr, tmpID.c_str());
    JsValue *name = new JsValue(JS_TAG_STRING, (void *)ptr);
    PushVal(name);
  } else {
    printf("[ERROR] lines %d: no [ID] in PRINT statement.\n", current_row);
    exit(1);
  }
  // 右括号
  token = getToken(fp);
  if (token != TOKEN_RBRACKET) {
    printf("[ERROR] lines %d: no ')' at the end of '$print'.\n", current_row);
    exit(1);
  }
  PushCode(OP_PRINT);
}

void eval(FILE *fp) {
  while (!feof(fp)) {
    Token token = getToken(fp);

    if (DEBUG_LEXER) {
      cout << "[DEBUG] LEXER: " << _V(token);
      if (token == TOKEN_ID) cout << "\t" << tmpID;
      if (token == TOKEN_NUM) cout << "\t" << tmpNumVal;
      cout << endl;
    }

    // 语法分析
    switch (token) {
      case TOKEN_VAR: parseVarStatement(fp); break;
      case TOKEN_PRINT: parsePrintStatement(fp); break;
      default: break;
    }

    // 执行堆栈
    run();
  }
}

int main(int argc, char **argv) {
  char *filename = argv[1];
  FILE *fp = fopen(filename, "r");
  eval(fp);
  fclose(fp);
  return 0;
}
