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

#include <type_traits>

#include "ofConstants.h"
#include "ofGLUtils.h"
#include "ofPixels.h"
#include "ofxAsyncTransferUtils.h"

namespace ofxasynctransfer {

// A lightweight intermediate class representing a matrix of pixels. Converting
// this into an ofPixels does not copy its contents, and it should finish
// instantly. Because ofPixels does not accept const types in its template
// argument, the converted type will be const if the template argument is a
// const type in order to avoid the violation of constness.

template <class T>
class Pixels_ final {
 public:
  using NonConstPixel = typename std::remove_const<T>::type;
  using NonConstPixels = ofPixels_<NonConstPixel>;
  using Pixels = typename std::conditional<
    std::is_const<T>::value,
    typename std::add_const<NonConstPixels>::type,
    ofPixels_<T>
  >::type;

 public:
  Pixels_();
  Pixels_(T * data, int width, int height, GLenum format);

  // Copy semantics
  Pixels_(const Pixels_&) = default;
  Pixels_& operator=(const Pixels_&) = default;

  // Data
  T * getData() const { return data; }
  operator T *() const { return data; }

  // Attributes
  int getWidth() const { return width; }
  int getHeight() const { return height; }
  GLenum getFormat() const { return format; }
  GLenum getType() const { return GLType<T>::value; }

  // Iterator
  T * begin() const;
  T * end() const;

  // Pixels
  Pixels& getPixels() const;
  operator Pixels&() const { return getPixels(); }

 private:
  T * data;
  int width;
  int height;
  GLenum format;
  mutable NonConstPixels pixels;
};

template <class T>
inline Pixels_<T>::Pixels_() : data(), width(), height(), format() {}

template <class T>
inline Pixels_<T>::Pixels_(T * data, int width, int height, GLenum format)
    : data(data),
      width(width),
      height(height),
      format(format) {}

template <class T>
inline T * Pixels_<T>::begin() const {
  return data;
}

template <class T>
inline T * Pixels_<T>::end() const {
  return data + width * height * ofGetNumChannelsFromGLFormat(format);
}

template <class T>
typename Pixels_<T>::Pixels& Pixels_<T>::getPixels() const {
  if (!pixels.isAllocated()) {
    pixels.setFromExternalPixels(const_cast<NonConstPixel *>(data),
                                 width,
                                 height,
                                 ofGetNumChannelsFromGLFormat(format));
  }
  return pixels;
}

}  // namespace ofxasynctransfer

template <class T>
using ofxAsyncTransferPixels = ofxasynctransfer::Pixels_<T>;
