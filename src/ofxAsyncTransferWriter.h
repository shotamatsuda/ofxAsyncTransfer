//
//  ofxAsyncTransferWriter.h
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
#include <utility>

#include "ofConstants.h"
#include "ofGLUtils.h"
#include "ofPixels.h"
#include "ofTexture.h"
#include "ofxAsyncTransferFrames.h"
#include "ofxAsyncTransferPixels.h"
#include "ofxAsyncTransferUtils.h"

namespace ofxasynctransfer {

class Writer final {
 public:
  explicit Writer(int frames = 2);

  // Disallow copy semantics
  Writer(const Writer&) = delete;
  Writer& operator=(const Writer&) = delete;

  // Move semantics
  Writer(Writer&&) = default;
  Writer& operator=(Writer&&) = default;

  // Modifiers
  template <class T>
  Pixels<T> bind(ofTexture& texture);
  template <class T>
  Pixels<T> bind(ofTexture& texture, ofImageType imageType);
  template <class T>
  Pixels<T> bind(ofTexture& texture, ofPixelFormat pixelFormat);
  template <class T>
  Pixels<T> bind(ofTexture& texture, GLenum internalFormat);
  template <class T>
  Pixels<T> bind(ofTexture& texture, GLenum internalFormat, GLenum format);
  template <class T>
  ofPixels_<T> bindAsPixels(ofTexture& texture, ofImageType imageType);
  template <class T>
  ofPixels_<T> bindAsPixels(ofTexture& texture, ofPixelFormat pixelFormat);
  void unbind();

  // Controls the number of frames to use for asynchronous writing.
  int getFrameSize() const;
  void setFrameSize(int value);

 private:
  struct Data {
    int width;
    int height;
    GLenum internalFormat;
    GLenum format;
    Data() : width(), height(), internalFormat(), format() {}
    Data(int width, int height, GLenum internalFormat, GLenum format)
        : width(width),
          height(height),
          internalFormat(internalFormat),
          format(format) {}
  };

 private:
  Frames<Data> frames;
  bool bound;
};

template <class T>
inline Pixels<T> Writer::bind(ofTexture& texture) {
  // The texture must be allocated because we do not know what kind of internal
  // format is desired for it.
  if (!texture.isAllocated()) {
    return Pixels<T>();
  }
  return bind<T>(texture, texture.getTextureData().glInternalFormat);
}

template <class T>
inline Pixels<T> Writer::bind(ofTexture& texture, ofImageType imageType) {
  return bind<T>(texture, getPixelFormatFromImageType(imageType));
}

template <class T>
inline Pixels<T> Writer::bind(ofTexture& texture, ofPixelFormat pixelFormat) {
  const auto internalFormat = ofGetGLInternalFormatFromPixelFormat(pixelFormat);
  const auto format = ofGetGLFormatFromPixelFormat(pixelFormat);
  return bind<T>(texture, internalFormat, format);
}

template <class T>
inline Pixels<T> Writer::bind(ofTexture& texture, GLenum internalFormat) {
  const auto format = ofGetGLFormatFromInternal(internalFormat);
  return bind<T>(texture, internalFormat, format);
}

template <class T>
inline ofPixels_<T> bindAsPixels(ofTexture& texture, ofImageType imageType) {
  return bindAsPixels<T>(texture, getPixelFormatFromImageType(imageType));
}

template <class T>
inline ofPixels_<T> bindAsPixels(ofTexture& texture,
                                 ofPixelFormat pixelFormat) {
  const auto result = bind<T>(texture, pixelFormat);
  ofPixels_<T> pixels;
  pixels.setFromExternalPixels(result.getData(),
                               result.getWidth(),
                               result.getHeight(),
                               pixelFormat);
  return std::move(pixels);
}

}  // namespace ofxasynctransfer

using ofxAsyncTransferWriter = ofxasynctransfer::Writer;
