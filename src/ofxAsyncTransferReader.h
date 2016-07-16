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

#include "ofConstants.h"
#include "ofFbo.h"
#include "ofTexture.h"
#include "ofxAsyncTransferFrames.h"
#include "ofxAsyncTransferPixels.h"
#include "ofxAsyncTransferUtils.h"

namespace ofxasynctransfer {

class Reader final {
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
  void copyToPixels(const ofTexture& texture,
                    ofPixels_<T>& pixels,
                    ofImageType imageType = OF_IMAGE_UNDEFINED);
  template <class T>
  void copyToPixels(const ofFbo& fbo,
                    ofPixels_<T>& pixels,
                    ofImageType imageType = OF_IMAGE_UNDEFINED);
  template <class T>
  void copyToPixels(const ofTexture& texture,
                    ofPixels_<T>& pixels,
                    ofPixelFormat pixelFormat);
  template <class T>
  void copyToPixels(const ofFbo& fbo,
                    ofPixels_<T>& pixels,
                    ofPixelFormat pixelFormat);

  // Binds the contents of a target to CPU memory addresses via a pixel buffer
  // for reading. The unbind() must be called after finishing with the data to
  // release the buffer. The format of the resulting pixels is automatically
  // chosen from the internal format of the target when the image type is
  // undefined or pixel format is unknown.
  template <class T>
  Pixels_<const T> bind(const ofTexture& texture,
                        ofImageType imageType = OF_IMAGE_UNDEFINED);
  template <class T>
  Pixels_<const T> bind(const ofFbo& fbo,
                        ofImageType imageType = OF_IMAGE_UNDEFINED);
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
  // Converts the pixel format into GL's format, or determine an appropriate
  // format for a FBO or texture if the pixel format is unknown.
  GLenum getFormat(const ofFbo& fbo, ofPixelFormat pixelFormat) const;
  GLenum getFormat(const ofTexture& texture, ofPixelFormat pixelFormat) const;

  // Allocates the pixels for the given pixel format or the one determined by
  // the internal format of a FBO or texture if the pixel format is unknown.
  template <class T>
  bool allocatePixels(const ofFbo& fbo,
                      ofPixels_<T>& pixels,
                      ofPixelFormat pixelFormat) const;
  template <class T>
  bool allocatePixels(const ofTexture& texture,
                      ofPixels_<T>& pixels,
                      ofPixelFormat pixelFormat) const;
  template <class T>
  Frame<Data>& allocate(int width, int height, GLenum format);
  template <class T>
  Pixels_<const T> bind();

 private:
  Frames<Data> frames;
  bool bound;
};

template <class T>
inline void Reader::copyToPixels(const ofFbo& fbo,
                                 ofPixels_<T>& pixels,
                                 ofImageType imageType) {
  copyToPixels(fbo, pixels, getPixelFormatFromImageType(imageType));
}

template <class T>
inline void Reader::copyToPixels(const ofTexture& texture,
                                 ofPixels_<T>& pixels,
                                 ofImageType imageType) {
  copyToPixels(texture, pixels, getPixelFormatFromImageType(imageType));
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
  return bind<T>(fbo, getFormat(fbo, pixelFormat));
}

template <class T>
inline Pixels_<const T> Reader::bind(const ofTexture& texture,
                                     ofPixelFormat pixelFormat) {
  return bind<T>(texture, getFormat(texture, pixelFormat));
}

}  // namespace ofxasynctransfer

using ofxAsyncTransferReader = ofxasynctransfer::Reader;
