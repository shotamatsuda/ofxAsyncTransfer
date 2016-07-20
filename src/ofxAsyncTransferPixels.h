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

#include <cassert>
#include <iterator>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

#include "ofConstants.h"
#include "ofGLUtils.h"
#include "ofPixels.h"
#include "ofxAsyncTransferUtils.h"

namespace ofxasynctransfer {

// A lightweight intermediate class representing a matrix of pixels. Converting
// this to an ofPixels does not copy its contents and finishes instantly.

template <class T>
class Pixels_ final {
 public:
  using NonConstPixels = ofPixels_<typename std::remove_const<T>::type>;

  // Because ofPixels does not accept const types in its template argument, the
  // converted type will be const if the template argument is a const type in
  // order to avoid the violation of constness.
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

 private:
  template <class U>
  friend class Pixels_;
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
inline typename Pixels_<T>::Pixels& Pixels_<T>::getPixels() const {
  if (!pixels.isAllocated()) {
    pixels.setFromExternalPixels(
        const_cast<typename std::remove_const<T>::type *>(data),
        width,
        height,
        ofGetNumChannelsFromGLFormat(format));
  }
  return pixels;
}

// Full template specialization for arbitrary type of pixels

template <>
class Pixels_<void> final {
 public:
  Pixels_();
  Pixels_(void * data, int width, int height, GLenum format);
  template <class T>
  Pixels_(const Pixels_<T>& other);

  // Copy semantics
  Pixels_(const Pixels_&) = default;
  Pixels_& operator=(const Pixels_&) = default;

  // Data
  void * getData() const { return data; }
  operator void *() const { return data; }
  template <class T>
  T * getData() const { return data; }
  template <class T>
  operator T *() const { return data; }

  // Attributes
  int getWidth() const { return width; }
  int getHeight() const { return height; }
  GLenum getFormat() const { return format; }

  // Iterator
  template <class T>
  T * begin() const;
  template <class T>
  T * end() const;

  // Pixels
  template <class T>
  ofPixels_<T>& getPixels() const;
  template <class T>
  operator ofPixels_<T>&() const { return getPixels<T>(); }

 private:
  void * data;
  int width;
  int height;
  GLenum format;
  mutable std::unordered_map<std::type_index, std::shared_ptr<void>> map;
};

inline Pixels_<void>::Pixels_() : data(), width(), height(), format() {}

inline Pixels_<void>::Pixels_(void * data,
                              int width,
                              int height,
                              GLenum format)
    : data(data),
      width(width),
      height(height),
      format(format) {}

template <class T>
inline Pixels_<void>::Pixels_(const Pixels_<T>& other)
    : data(other.data),
      width(other.width),
      height(other.height),
      format(other.format) {}

template <class T>
inline T * Pixels_<void>::begin() const {
  return data;
}

template <class T>
inline T * Pixels_<void>::end() const {
  return reinterpret_cast<T *>(data) +
      width * height * ofGetNumChannelsFromGLFormat(format);
}

template <class T>
inline ofPixels_<T>& Pixels_<void>::getPixels() const {
  using U = typename std::remove_cv<T>::type;
  const std::type_index index(typeid(U));
  auto itr = map.find(index);
  if (itr == std::end(map)) {
    const auto ptr = std::shared_ptr<void>(new ofPixels_<U>);
    const auto result = map.emplace(index, std::move(ptr));
    assert(result.second);
    itr = result.first;
  }
  const auto& ptr = itr->second;
  assert(ptr);
  const auto pixels = std::static_pointer_cast<ofPixels_<U>>(ptr);
  if (!pixels->isAllocated()) {
    pixels->setFromExternalPixels(
        reinterpret_cast<U *>(data),
        width,
        height,
        ofGetNumChannelsFromGLFormat(format));
  }
  return *pixels;
}

// Full template specialization for arbitrary const type of pixels

template <>
class Pixels_<const void> final {
 public:
  Pixels_();
  Pixels_(const void * data, int width, int height, GLenum format);
  template <class T>
  Pixels_(const Pixels_<T>& other);

  // Copy semantics
  Pixels_(const Pixels_&) = default;
  Pixels_& operator=(const Pixels_&) = default;

  // Data
  const void * getData() const { return data; }
  operator const void *() const { return data; }
  template <class T>
  const T * getData() const { return data; }
  template <class T>
  operator const T *() const { return data; }

  // Attributes
  int getWidth() const { return width; }
  int getHeight() const { return height; }
  GLenum getFormat() const { return format; }

  // Iterator
  template <class T>
  const T * begin() const;
  template <class T>
  const T * end() const;

  // Pixels
  template <class T>
  const ofPixels_<T>& getPixels() const;
  template <class T>
  operator const ofPixels_<T>&() const { return getPixels<T>(); }

 private:
  const void * data;
  int width;
  int height;
  GLenum format;
  mutable std::unordered_map<std::type_index, std::shared_ptr<void>> map;
};

inline Pixels_<const void>::Pixels_() : data(), width(), height(), format() {}

inline Pixels_<const void>::Pixels_(const void * data,
                                    int width,
                                    int height,
                                    GLenum format)
    : data(data),
      width(width),
      height(height),
      format(format) {}

template <class T>
inline Pixels_<const void>::Pixels_(const Pixels_<T>& other)
    : data(other.data),
      width(other.width),
      height(other.height),
      format(other.format) {}

template <class T>
inline const T * Pixels_<const void>::begin() const {
  return data;
}

template <class T>
inline const T * Pixels_<const void>::end() const {
  return reinterpret_cast<const T *>(data) +
      width * height * ofGetNumChannelsFromGLFormat(format);
}

template <class T>
inline const ofPixels_<T>& Pixels_<const void>::getPixels() const {
  using U = typename std::remove_cv<T>::type;
  const std::type_index index(typeid(U));
  auto itr = map.find(index);
  if (itr == std::end(map)) {
    const auto ptr = std::shared_ptr<void>(new ofPixels_<U>);
    const auto result = map.emplace(index, std::move(ptr));
    assert(result.second);
    itr = result.first;
  }
  const auto& ptr = itr->second;
  assert(ptr);
  const auto pixels = std::static_pointer_cast<ofPixels_<U>>(ptr);
  if (!pixels->isAllocated()) {
    pixels->setFromExternalPixels(
        reinterpret_cast<U *>(const_cast<void *>(data)),
        width,
        height,
        ofGetNumChannelsFromGLFormat(format));
  }
  return *pixels;
}

}  // namespace ofxasynctransfer

template <class T>
using ofxAsyncTransferPixels_ = ofxasynctransfer::Pixels_<T>;
