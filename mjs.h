#include <map>
#include <string>
using namespace std;

// 词法单元
enum Token {
  TOKEN_UNKNOWN = -100,
  TOKEN_EOF,       // end of file
  TOKEN_ID,        // identifier
  TOKEN_VAR,       // "var"
  TOKEN_PRINT,     // "print"
  TOKEN_NUM,       // number
  TOKEN_STRING,    // string
  TOKEN_EQUAL,     // "="
  TOKEN_PLUS,      // "+"
  TOKEN_MINUS,     // "-"
  TOKEN_STAR,      // "*"
  TOKEN_SLASH,     // "/"
  TOKEN_LBRACKET,  // "("
  TOKEN_RBRACKET,  // ")"
  TOEKN_SEMI,      // ";"
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

union JsValueUnion {
  int32_t int32;
  double float64;
  void* ptr;

  JsValueUnion() {}
  JsValueUnion(int32_t v) : int32(v) {}
  JsValueUnion(double v) : float64(v) {}
  JsValueUnion(void* v) : ptr(v) {}
};

enum JsValueTag {
  JS_TAG_NUM = 0,
  JS_TAG_STRING,
  JS_TAG_NULL,
};

// 虚拟机内部数据类型
class JsValue {
 public:
  JsValueUnion val;
  JsValueTag tag;

  JsValue(JsValueTag _tag) : tag(_tag) {}
  JsValue(JsValueTag _tag, JsValueUnion _val) : tag(_tag), val(_val) {}
  JsValue(JsValueTag _tag, int32_t _val) : tag(_tag), val(JsValueUnion(_val)) {}
  JsValue(JsValueTag _tag, double _val) : tag(_tag), val(JsValueUnion(_val)) {}
  JsValue(JsValueTag _tag, void* _val) : tag(_tag), val(JsValueUnion(_val)) {}
};

// 作用域
class Scope {
 private:
  Scope* parent;
  map<string, JsValue*> values;

 public:
  void set(string name, JsValue* value) {
    this->values.insert(pair<string, JsValue*>(name, value));
  }
  JsValue* get(string name) {
    auto iter = this->values.find(name);
    if (iter != this->values.end()) {
      return iter->second;
    } else {
      printf("[ERROR] Variable '%s' is undefined.\n", name.c_str());
      exit(1);
    }
  }
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
    case TOKEN_PRINT: return "TOKEN_PRINT";
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
