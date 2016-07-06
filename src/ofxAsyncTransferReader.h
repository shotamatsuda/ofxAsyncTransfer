//
//  ofxAsyncTransferReader.h
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
#include "ofFbo.h"
#include "ofTexture.h"
#include "ofxAsyncTransferFrames.h"
#include "ofxAsyncTransferPixels.h"
#include "ofxAsyncTransferUtils.h"

namespace ofxasynctransfer {

class Reader final {
 public:
  explicit Reader(int frames = 2);

  // Disallow copy semantics
  Reader(const Reader&) = delete;
  Reader& operator=(const Reader&) = delete;

  // Move semantics
  Reader(Reader&&) = default;
  Reader& operator=(Reader&&) = default;

  // Modifiers
  template <class T>
  Pixels<const T> bind(const ofFbo& fbo, ofImageType imageType);
  template <class T>
  Pixels<const T> bind(const ofTexture& texture, ofImageType imageType);
  template <class T>
  Pixels<const T> bind(const ofFbo& fbo, ofPixelFormat pixelFormat);
  template <class T>
  Pixels<const T> bind(const ofTexture& texture, ofPixelFormat pixelFormat);
  template <class T>
  Pixels<const T> bind(const ofFbo& fbo, GLenum format);
  template <class T>
  Pixels<const T> bind(const ofTexture& texture, GLenum format);
  void unbind();

  // Copies the contents of a target into pixels.
  template <class T>
  void copyTo(const ofFbo& fbo,
              ofPixels_<T>& pixels,
              ofPixelFormat pixelFormat);
  template <class T>
  void copyTo(const ofTexture& texture,
              ofPixels_<T>& pixels,
              ofPixelFormat pixelFormat);

  // Controls the number of frames to use for asynchronous reading.
  int getFrameSize() const;
  void setFrameSize(int value);

 private:
  struct Data {
    int width;
    int height;
    GLenum format;
    Data() : width(), height(), format() {}
    Data(int width, int height, GLenum format)
        : width(width),
          height(height),
          format(format) {}
  };

 private:
  template <class T>
  Frame<Data>& allocate(int width, int height, GLenum format);
  template <class T>
  Pixels<const T> bind();

 private:
  Frames<Data> frames;
  bool bound;
};

template <class T>
inline Pixels<const T> Reader::bind(const ofFbo& fbo,
                                    ofImageType imageType) {
  return bind<T>(fbo, getPixelFormatFromImageType(imageType));
}

template <class T>
inline Pixels<const T> Reader::bind(const ofTexture& texture,
                                    ofImageType imageType) {
  return bind<T>(texture, getPixelFormatFromImageType(imageType));
}

template <class T>
inline Pixels<const T> Reader::bind(const ofFbo& fbo,
                                    ofPixelFormat pixelFormat) {
  return bind<T>(fbo, ofGetGLFormatFromPixelFormat(pixelFormat));
}

template <class T>
inline Pixels<const T> Reader::bind(const ofTexture& texture,
                                    ofPixelFormat pixelFormat) {
  return bind<T>(texture, ofGetGLFormatFromPixelFormat(pixelFormat));
}

}  // namespace ofxasynctransfer

using ofxAsyncTransferReader = ofxasynctransfer::Reader;
