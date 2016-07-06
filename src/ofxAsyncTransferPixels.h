//
//  ofxAsyncTransferPixels.h
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

#include <cstddef>

#include "ofConstants.h"
#include "ofGLUtils.h"
#include "ofxAsyncTransferUtils.h"

namespace ofxasynctransfer {

template <class T>
class Pixels final {
 public:
  Pixels();
  Pixels(T * data, int width, int height, GLenum format);

  // Copy semantics
  Pixels(const Pixels&) = default;
  Pixels& operator=(const Pixels&) = default;

  // Data
  T * getData() const { return data; }
  operator T *() const { return data; }

  // Attributes
  int getWidth() const { return width; }
  int getHeight() const { return height; }
  GLenum getFormat() const { return format; }
  GLenum getType() const { return GLType<T>::value; }

  // Iterator
  T * begin() const { return data; }
  T * end() const { return data + size; }

 private:
  T * data;
  int width;
  int height;
  GLenum format;
  std::size_t size;
};

template <class T>
inline Pixels<T>::Pixels() : data(), width(), height(), format(), size() {}

template <class T>
inline Pixels<T>::Pixels(T * data, int width, int height, GLenum format)
    : data(data),
      width(width),
      height(height),
      format(format),
      size(width * height * ofGetNumChannelsFromGLFormat(format)) {}

}  // namespace ofxasynctransfer

template <class T>
using ofxAsyncTransferPixels = ofxasynctransfer::Pixels<T>;
