//
//  ofxAsyncTransferUtils.h
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

#include <cstdint>

#include "ofConstants.h"

namespace ofxasynctransfer {

template <class T>
struct GLType {};

template <>
struct GLType<std::uint8_t> {
  static constexpr const GLenum value = GL_UNSIGNED_BYTE;
};
template <>
struct GLType<std::int8_t> {
  static constexpr const GLenum value = GL_BYTE;
};
template <>
struct GLType<std::uint16_t> {
  static constexpr const GLenum value = GL_UNSIGNED_SHORT;
};
template <>
struct GLType<std::int16_t> {
  static constexpr const GLenum value = GL_SHORT;
};
template <>
struct GLType<std::uint32_t> {
  static constexpr const GLenum value = GL_UNSIGNED_INT;
};
template <>
struct GLType<std::int32_t> {
  static constexpr const GLenum value = GL_INT;
};
template <>
struct GLType<float> {
  static constexpr const GLenum value = GL_FLOAT;
};

// A image type is just a number of channels with semantic meaning. Conversions
// from image types to pixel formats are upcasting and do not loose information.
// It's a matter of interpretation.
ofPixelFormat getPixelFormatFromImageType(ofImageType imageType);

}  // namespace ofxasynctransfer
