#include <map>
#include <string>
using namespace std;

// 词法单元
enum Token {
  TOKEN_UNKNOWN = -100,
  TOKEN_EOF,     // end of file
  TOKEN_ID,      // identifier
  TOKEN_VAR,     // "var"
  TOKEN_NUM,     // number
  TOKEN_STRING,  // string
  TOKEN_EQUAL,   // "="
  TOKEN_PLUS,    // "+"
  TOKEN_MINUS,   // "-"
  TOKEN_STAR,    // "*"
  TOKEN_SLASH,   // "/"
  TOEKN_SEMI,    // ";"
};

// 虚拟机操作码
enum OpCode : uint8_t {
  OP_ADD = 0,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_PRINT,
  OP_JMP,
  OP_ASSIGN,
  OP_IF,
  OP_RET,
  OP_CALL,
  OP_EXIT
};

union JSValueUnion {
  int32_t int32;
  double float64;
  void* ptr;
};

enum JsValueTag {
  JS_TAG_NUM = 0,
  JS_TAG_STRING,
};

// 虚拟机内部数据类型
class JsValue {
 public:
  JSValueUnion value;
  JsValueTag tag;
};

// 作用域
class Scope {
 public:
  Scope* parent;
  map<string, JsValue> values;
};

template <typename T>
static uint8_t* toBytes(T u) {
  int n = sizeof(T);
  uint8_t* b = new uint8_t[n];
  memcpy(b, &u, n);
  return b;
}

template <typename T>
static T fromBytes(uint8_t* bytes) {
  T res = 0;
  int n = sizeof(T);
  memcpy(&res, bytes, n);
  return res;
}

string _V(Token token) {
  switch (token) {
    case TOKEN_UNKNOWN: return "TOKEN_UNKNOWN";
    case TOKEN_EOF: return "TOKEN_EOF";
    case TOKEN_ID: return "TOKEN_ID";
    case TOKEN_VAR: return "TOKEN_VAR";
    case TOKEN_NUM: return "TOKEN_NUM";
    case TOKEN_STRING: return "TOKEN_STRING";
    case TOKEN_EQUAL: return "TOKEN_EQUAL";
    case TOKEN_PLUS: return "TOKEN_PLUS";
    case TOKEN_MINUS: return "TOKEN_MINUS";
    case TOKEN_STAR: return "TOKEN_STAR";
    case TOKEN_SLASH: return "TOKEN_SLASH";
    case TOEKN_SEMI: return "TOEKN_SEMI";
    default: return NULL;
  }
}
