// Copyright 2020 Vincent Wang <wang.yuanqiu007@gmail.com>
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

namespace sjs {

  #define uint64 unsigned long long int

  uint64 getUTF8Length(const char* pStr);

}  // namespace sjs

#endif  // SRC_UTILS_H_