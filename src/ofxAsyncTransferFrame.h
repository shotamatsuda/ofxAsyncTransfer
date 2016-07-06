//
//  ofxAsyncTransferFrame.h
//
//  The MIT License
//
//  Copyright (C) 2016 Shota Matsuda
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//

#pragma once

#include "ofBufferObject.h"

namespace ofxasynctransfer {

template <class Data>
class Frame final {
 public:
  Frame() = default;

  // Disallow copy semantics
  Frame(const Frame&) = delete;
  Frame& operator=(const Frame&) = delete;

  // Move semantics
  Frame(Frame&&) = default;
  Frame& operator=(Frame&&) = default;

  // Buffer object
  ofBufferObject& getBuffer() { return buffer; }
  const ofBufferObject& getBuffer() const { return buffer; }
  ofBufferObject * operator->() { return &buffer; }
  const ofBufferObject * operator->() const { return &buffer; }
  operator ofBufferObject&() { return buffer; }
  operator const ofBufferObject&() const { return buffer; }

  // Data
  template <class... Args>
  void setData(Args&&... args) { data = Data(args...); }
  const Data& getData() const { return data; }

 private:
  ofBufferObject buffer;
  Data data;
};

}  // namespace ofxasynctransfer
