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

#include "ofConstants.h"
#include "ofTexture.h"
#include "ofxAsyncTransferFrames.h"
#include "ofxAsyncTransferPixels.h"
#include "ofxAsyncTransferUtils.h"

namespace ofxasynctransfer {

class Writer final {
 public:
  explicit Writer(int frames = 3);
  void setup(int frames);

  // Disallow copy semantics
  Writer(const Writer&) = delete;
  Writer& operator=(const Writer&) = delete;

  // Move semantics
  Writer(Writer&&) = default;
  Writer& operator=(Writer&&) = default;

  // Binds the contents of a texture to CPU memory addresses via a pixel buffer
  // for writing. The unbind() must be called after finishing with the data to
  // release the buffer. The format of the resulting pixels is automatically
  // chosen from the internal format of the target when the image type is
  // undefined or pixel format is unknown.
  template <class T>
  Pixels_<T> bind(ofTexture& texture,
                  ofImageType imageType = OF_IMAGE_UNDEFINED);
  template <class T>
  Pixels_<T> bind(ofTexture& texture, ofPixelFormat pixelFormat);
  template <class T>
  Pixels_<T> bind(ofTexture& texture, GLenum format);

  // Unbinds the contents of the target bound by bind() from a pixel buffer.
  // This must be called after finishing with the data to release the buffer.
  void unbind();

  // Controls the number of frames to use for asynchronous writing. In contrast
  // to reading, setting this to 1 does not necessarily mean synchronous
  // transfer, because every time bind() is called, the contents of pixel
  // buffer is discarded while it is being uploaded. An optimal size depends on
  // use cases and the frequency of lookups, but the range between 1-3 should
  // be enough with respect to latency and memory consumption.
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
  Frames<Data> frames;
  bool bound;
};

template <class T>
inline Pixels_<T> Writer::bind(ofTexture& texture, ofImageType imageType) {
  return bind<T>(texture, getPixelFormatFromImageType(imageType));
}

}  // namespace ofxasynctransfer

using ofxAsyncTransferWriter = ofxasynctransfer::Writer;
