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

#include <utility>

#include "ofConstants.h"
#include "ofFbo.h"
#include "ofTexture.h"
#include "ofxAsyncTransferFrames.h"
#include "ofxAsyncTransferPixels.h"
#include "ofxAsyncTransferUtils.h"

namespace ofxasynctransfer {

class Reader final {
 public:
  using Pixels = Pixels_<const unsigned char>;
  using ShortPixels = Pixels_<const unsigned short>;
  using FloatPixels = Pixels_<const float>;

 public:
  explicit Reader(int frames = 3);
  void setup(int frames);

  // Disallow copy semantics
  Reader(const Reader&) = delete;
  Reader& operator=(const Reader&) = delete;

  // Move semantics
  Reader(Reader&&) = default;
  Reader& operator=(Reader&&) = default;

  // Copies the contents of a target into pixels asynchronously with the cost
  // of GPU memory and some amount of CPU. Use bind() if you just want to
  // lookup the contents of the target. The format of the resulting pixels is
  // automatically chosen from the internal format of the target when the image
  // type is undefined or pixel format is unknown.
  template <class T>
  bool copyToPixels(const ofTexture& texture, ofPixels_<T>& pixels);
  template <class T>
  bool copyToPixels(const ofFbo& fbo, ofPixels_<T>& pixels);
  template <class T>
  bool copyToPixels(const ofTexture& texture,
                    ofPixels_<T>& pixels,
                    ofImageType imageType);
  template <class T>
  bool copyToPixels(const ofFbo& fbo,
                    ofPixels_<T>& pixels,
                    ofImageType imageType);
  template <class T>
  bool copyToPixels(const ofTexture& texture,
                    ofPixels_<T>& pixels,
                    ofPixelFormat pixelFormat);
  template <class T>
  bool copyToPixels(const ofFbo& fbo,
                    ofPixels_<T>& pixels,
                    ofPixelFormat pixelFormat);

  // Shorthand functions
  template <class... Args>
  Pixels bindAsPixels(Args&&... args);
  template <class... Args>
  ShortPixels bindAsShortPixels(Args&&... args);
  template <class... Args>
  FloatPixels bindAsFloatPixels(Args&&... args);

  // Binds the contents of a target to CPU memory addresses via a pixel buffer
  // for reading. The unbind() must be called after finishing with the data to
  // release the buffer. The format of the resulting pixels is automatically
  // chosen from the internal format of the target when the image type is
  // undefined or pixel format is unknown.
  Pixels_<const void> bind(const ofTexture& texture);
  Pixels_<const void> bind(const ofFbo& fbo);
  Pixels_<const void> bind(const ofTexture& texture, ofImageType imageType);
  Pixels_<const void> bind(const ofFbo& fbo, ofImageType imageType);
  Pixels_<const void> bind(const ofTexture& texture, ofPixelFormat pixelFormat);
  Pixels_<const void> bind(const ofFbo& fbo, ofPixelFormat pixelFormat);
  Pixels_<const void> bind(const ofTexture& texture, GLenum format);
  Pixels_<const void> bind(const ofFbo& fbo, GLenum format);
  template <class T>
  Pixels_<const T> bind(const ofTexture& texture);
  template <class T>
  Pixels_<const T> bind(const ofFbo& fbo);
  template <class T>
  Pixels_<const T> bind(const ofTexture& texture, ofImageType imageType);
  template <class T>
  Pixels_<const T> bind(const ofFbo& fbo, ofImageType imageType);
  template <class T>
  Pixels_<const T> bind(const ofTexture& texture, ofPixelFormat pixelFormat);
  template <class T>
  Pixels_<const T> bind(const ofFbo& fbo, ofPixelFormat pixelFormat);
  template <class T>
  Pixels_<const T> bind(const ofTexture& texture, GLenum format);
  template <class T>
  Pixels_<const T> bind(const ofFbo& fbo, GLenum format);

  // Unbinds the contents of the target bound by bind() from a pixel buffer.
  // This must be called after finishing with pixel data to release the buffer.
  void unbind();

  // Controls the number of frames to use for asynchronous reading. Setting
  // this to 1 means synchronous transfer. An optimal size depends on use cases
  // and the frequency of lookups, but the range between 1-3 should be enough
  // with respect to latency and memory consumption.
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
  // Allocates the pixels for the given pixel format or the one determined by
  // the internal format of a FBO or texture if the pixel format is unknown.
  template <class T, class U>
  bool allocatePixels(const Pixels_<U>& data,
                      ofPixels_<T>& pixels,
                      ofPixelFormat pixelFormat) const;
  template <class T>
  Frame<Data>& allocate(int width, int height, GLenum format);
  template <class... Args>
  Pixels_<const void> bind(GLenum type, Args&&... args);
  template <class T>
  Pixels_<const T> bind();

 private:
  Frames<Data> frames;
  bool bound;
};

template <class T>
inline bool Reader::copyToPixels(const ofFbo& fbo,
                                 ofPixels_<T>& pixels) {
  return copyToPixels(fbo, pixels, OF_PIXELS_UNKNOWN);
}

template <class T>
inline bool Reader::copyToPixels(const ofTexture& texture,
                                 ofPixels_<T>& pixels) {
  return copyToPixels(texture, pixels, OF_PIXELS_UNKNOWN);
}

template <class T>
inline bool Reader::copyToPixels(const ofFbo& fbo,
                                 ofPixels_<T>& pixels,
                                 ofImageType imageType) {
  return copyToPixels(fbo, pixels, getPixelFormatFromImageType(imageType));
}

template <class T>
inline bool Reader::copyToPixels(const ofTexture& texture,
                                 ofPixels_<T>& pixels,
                                 ofImageType imageType) {
  return copyToPixels(texture, pixels, getPixelFormatFromImageType(imageType));
}

template <class... Args>
inline Reader::Pixels Reader::bindAsPixels(Args&&... args) {
  return bind<unsigned char>(std::forward<Args>(args)...);
}

template <class... Args>
inline Reader::ShortPixels Reader::bindAsShortPixels(Args&&... args) {
  return bind<unsigned short>(std::forward<Args>(args)...);
}

template <class... Args>
inline Reader::FloatPixels Reader::bindAsFloatPixels(Args&&... args) {
  return bind<float>(std::forward<Args>(args)...);
}

inline Pixels_<const void> Reader::bind(const ofTexture& texture) {
  return bind(getGLType(texture), texture);
}

inline Pixels_<const void> Reader::bind(const ofFbo& fbo) {
  return bind(getGLType(fbo), fbo);
}

inline Pixels_<const void> Reader::bind(const ofTexture& texture,
                                        ofImageType imageType) {
  return bind(getGLType(texture), texture, imageType);
}

inline Pixels_<const void> Reader::bind(const ofFbo& fbo,
                                        ofImageType imageType) {
  return bind(getGLType(fbo), fbo, imageType);
}

inline Pixels_<const void> Reader::bind(const ofTexture& texture,
                                        ofPixelFormat pixelFormat) {
  return bind(getGLType(texture), texture, pixelFormat);
}

inline Pixels_<const void> Reader::bind(const ofFbo& fbo,
                                        ofPixelFormat pixelFormat) {
  return bind(getGLType(fbo), fbo, pixelFormat);
}

inline Pixels_<const void> Reader::bind(const ofTexture& texture,
                                        GLenum format) {
  return bind(getGLType(texture), texture, format);
}

inline Pixels_<const void> Reader::bind(const ofFbo& fbo,
                                        GLenum format) {
  return bind(getGLType(fbo), fbo, format);
}

template <class... Args>
inline Pixels_<const void> Reader::bind(GLenum type, Args&&... args) {
  switch (type) {
    case GL_UNSIGNED_BYTE:
      return bind<unsigned char>(std::forward<Args>(args)...);
    case GL_UNSIGNED_SHORT:
      return bind<unsigned short>(std::forward<Args>(args)...);
    case GL_FLOAT:
      return bind<float>(std::forward<Args>(args)...);
    default:
      break;
  }
  return Pixels_<const void>();
}

template <class T>
inline Pixels_<const T> Reader::bind(const ofFbo& fbo) {
  return bind<T>(fbo, OF_PIXELS_UNKNOWN);
}

template <class T>
inline Pixels_<const T> Reader::bind(const ofTexture& texture) {
  return bind<T>(texture, OF_PIXELS_UNKNOWN);
}

template <class T>
inline Pixels_<const T> Reader::bind(const ofFbo& fbo,
                                     ofImageType imageType) {
  return bind<T>(fbo, getPixelFormatFromImageType(imageType));
}

template <class T>
inline Pixels_<const T> Reader::bind(const ofTexture& texture,
                                     ofImageType imageType) {
  return bind<T>(texture, getPixelFormatFromImageType(imageType));
}

template <class T>
inline Pixels_<const T> Reader::bind(const ofFbo& fbo,
                                     ofPixelFormat pixelFormat) {
  return bind<T>(fbo, getGLFormat(fbo, pixelFormat));
}

template <class T>
inline Pixels_<const T> Reader::bind(const ofTexture& texture,
                                     ofPixelFormat pixelFormat) {
  return bind<T>(texture, getGLFormat(texture, pixelFormat));
}

}  // namespace ofxasynctransfer

using ofxAsyncTransferReader = ofxasynctransfer::Reader;
