// Copyright 2020 Vincent Wang <wang.yuanqiu007@gmail.com>
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_AST_H_
#define SRC_AST_H_

#include <string>

namespace sjs {

enum NodeType {
  Identifier
};

class SourceLocation {
 public:
  std::string source;
};

class Node {
 public:
  NodeType type;
};

}  // namespace sjs

#endif  // SRC_AST_H_
