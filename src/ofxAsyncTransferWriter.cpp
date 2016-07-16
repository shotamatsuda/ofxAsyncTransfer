//
//  ofxAsyncTransferWriter.cpp
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
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//

#include "ofxAsyncTransferWriter.h"

#include <cstdint>

#include "ofConstants.h"
#include "ofGLUtils.h"
#include "ofTexture.h"
#include "ofxAsyncTransferPixels.h"
#include "ofxAsyncTransferUtils.h"

namespace ofxasynctransfer {

Writer::Writer(int frames) : frames(frames), bound() {}

void Writer::setup(int frames) {
  this->frames.setSize(frames);
}

template <class T>
Pixels_<T> Writer::bind(ofTexture& texture, ofPixelFormat pixelFormat) {
  GLenum format;
  if (pixelFormat != OF_PIXELS_UNKNOWN) {
    format = ofGetGLFormatFromPixelFormat(pixelFormat);
  } else {
    if (!texture.isAllocated()) {
      // The texture must be allocated because we cannot determine appropriate
      // format when both pixel format and internal format are unknown.
      return Pixels_<T>();
    }
    format = ofGetGLFormatFromInternal(
        texture.getTextureData().glInternalFormat);
  }
  return bind<T>(texture, format);
}

template Pixels_<std::uint8_t> Writer::bind(ofTexture&, ofPixelFormat);
template Pixels_<std::int8_t> Writer::bind(ofTexture&, ofPixelFormat);
template Pixels_<std::uint16_t> Writer::bind(ofTexture&, ofPixelFormat);
template Pixels_<std::int16_t> Writer::bind(ofTexture&, ofPixelFormat);
template Pixels_<std::uint32_t> Writer::bind(ofTexture&, ofPixelFormat);
template Pixels_<std::int32_t> Writer::bind(ofTexture&, ofPixelFormat);
template Pixels_<float> Writer::bind(ofTexture&, ofPixelFormat);

template <class T>
Pixels_<T> Writer::bind(ofTexture& texture, GLenum format) {
  if (bound) {
    unbind();
  }
  frames.nextFrame();  // Request a next available frame

  // Read the buffer object into the texture.
  auto& current = frames.getCurrentFrame();
  if (current->isAllocated()) {
    // Let texture allocate itself.
    texture.loadData(current, current.getData().format, GLType<T>::value);
  }

  // Allocate a new buffer object for the first use.
  auto& next = frames.getNextFrame();
  if (!next->isAllocated()) {
    next->allocate();
  }

  // Remember the dimensions, internal format and format of the data.
  const auto width = texture.getWidth();
  const auto height = texture.getHeight();
  next.setData(width, height, format);

  // Always discard the current content and request a new memory.
  const auto channels = ofGetNumChannelsFromGLFormat(format);
  const auto bytes = width * height * channels * sizeof(T);
  next->bind(GL_PIXEL_UNPACK_BUFFER);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, bytes, nullptr, GL_STREAM_DRAW);
  const auto ptr = next->map<T>(GL_WRITE_ONLY);
  if (!ptr) {
    next->unbind(GL_PIXEL_UNPACK_BUFFER);
    return Pixels_<T>();
  }
  bound = true;
  return Pixels_<T>(ptr, width, height, format);
}

template Pixels_<std::uint8_t> Writer::bind(ofTexture&, GLenum);
template Pixels_<std::int8_t> Writer::bind(ofTexture&, GLenum);
template Pixels_<std::uint16_t> Writer::bind(ofTexture&, GLenum);
template Pixels_<std::int16_t> Writer::bind(ofTexture&, GLenum);
template Pixels_<std::uint32_t> Writer::bind(ofTexture&, GLenum);
template Pixels_<std::int32_t> Writer::bind(ofTexture&, GLenum);
template Pixels_<float> Writer::bind(ofTexture&, GLenum);

void Writer::unbind() {
  if (bound) {
    auto& next = frames.getNextFrame();
    next->unmap();
    next->unbind(GL_PIXEL_UNPACK_BUFFER);
    bound = false;
  }
}

int Writer::getFrameSize() const {
  return frames.getSize();
}

void Writer::setFrameSize(int value) {
  frames.setSize(value);
}

}  // namespace ofxasynctransfer
