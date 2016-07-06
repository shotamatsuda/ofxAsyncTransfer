//
//  ofxAsyncTransferReader.cpp
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

#include "ofxAsyncTransferReader.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>

#include "ofConstants.h"
#include "ofFbo.h"
#include "ofGLUtils.h"
#include "ofTexture.h"
#include "ofxAsyncTransferFrames.h"
#include "ofxAsyncTransferPixels.h"
#include "ofxAsyncTransferUtils.h"

namespace ofxasynctransfer {

Reader::Reader(int frames) : frames(frames), bound() {}

template <class T>
Pixels<const T> Reader::bind(const ofFbo& fbo, GLenum format) {
  if (bound) {
    unbind();
  }
  frames.nextFrame();  // Request a next available frame
  const auto width = fbo.getWidth();
  const auto height = fbo.getHeight();
  auto& current = allocate<T>(width, height, format);

  // Read the framebuffer into a buffer object for the current frame
  current->bind(GL_PIXEL_PACK_BUFFER);
  fbo.bind();
  glReadPixels(0, 0, width, height, format, GLType<T>::value, nullptr);
  fbo.unbind();
  current->unbind(GL_PIXEL_PACK_BUFFER);

  // Remember the dimensions and format of the data
  current.setData(width, height, format);
  return bind<T>();
}

template Pixels<const std::uint8_t> Reader::bind(const ofFbo&, GLenum);
template Pixels<const std::int8_t> Reader::bind(const ofFbo&, GLenum);
template Pixels<const std::uint16_t> Reader::bind(const ofFbo&, GLenum);
template Pixels<const std::int16_t> Reader::bind(const ofFbo&, GLenum);
template Pixels<const std::uint32_t> Reader::bind(const ofFbo&, GLenum);
template Pixels<const std::int32_t> Reader::bind(const ofFbo&, GLenum);
template Pixels<const float> Reader::bind(const ofFbo&, GLenum);

template <class T>
Pixels<const T> Reader::bind(const ofTexture& texture, GLenum format) {
  if (bound) {
    unbind();
  }
  frames.nextFrame();  // Request a next available frame
  const auto width = texture.getWidth();
  const auto height = texture.getHeight();
  auto& current = allocate<T>(width, height, format);

  // Read the texture into a buffer object for the current frame
  const auto channels = ofGetNumChannelsFromGLFormat(format);
  const auto& target = texture.getTextureData().textureTarget;
  ofSetPixelStoreiAlignment(GL_PACK_ALIGNMENT, width, sizeof(T), channels);
  current->bind(GL_PIXEL_PACK_BUFFER);
  texture.bind();
  glGetTexImage(target, 0, format, GLType<T>::value, nullptr);
  texture.unbind();
  current->unbind(GL_PIXEL_PACK_BUFFER);

  // Remember the dimensions and format of the data
  current.setData(width, height, format);
  return bind<T>();
}

template Pixels<const std::uint8_t> Reader::bind(const ofTexture&, GLenum);
template Pixels<const std::int8_t> Reader::bind(const ofTexture&, GLenum);
template Pixels<const std::uint16_t> Reader::bind(const ofTexture&, GLenum);
template Pixels<const std::int16_t> Reader::bind(const ofTexture&, GLenum);
template Pixels<const std::uint32_t> Reader::bind(const ofTexture&, GLenum);
template Pixels<const std::int32_t> Reader::bind(const ofTexture&, GLenum);
template Pixels<const float> Reader::bind(const ofTexture&, GLenum);

template <class T>
void Reader::copyTo(const ofFbo& fbo,
                    ofPixels_<T>& pixels,
                    ofPixelFormat pixelFormat) {
  const auto format = ofGetGLFormatFromPixelFormat(pixelFormat);
  const auto result = bind<T>(fbo, format);
  pixels.allocate(result.getWidth(), result.getHeight(), pixelFormat);
  const auto begin = std::begin(result);
  const auto end = std::end(result);
  assert(sizeof(T) * std::distance(begin, end) == pixels.getTotalBytes());
  std::copy(begin, end, pixels.getData());
  unbind();
}

template void Reader::copyTo(const ofFbo&, ofPixels&, ofPixelFormat);
template void Reader::copyTo(const ofFbo&, ofShortPixels&, ofPixelFormat);
template void Reader::copyTo(const ofFbo&, ofFloatPixels&, ofPixelFormat);

template <class T>
void Reader::copyTo(const ofTexture& texture,
                    ofPixels_<T>& pixels,
                    ofPixelFormat pixelFormat) {
  const auto format = ofGetGLFormatFromPixelFormat(pixelFormat);
  const auto result = bind<T>(texture, format);
  pixels.allocate(result.getWidth(), result.getHeight(), pixelFormat);
  const auto begin = std::begin(result);
  const auto end = std::end(result);
  assert(sizeof(T) * std::distance(begin, end) == pixels.getTotalBytes());
  std::copy(begin, end, pixels.getData());
  unbind();
}

template void Reader::copyTo(const ofTexture&, ofPixels&, ofPixelFormat);
template void Reader::copyTo(const ofTexture&, ofShortPixels&, ofPixelFormat);
template void Reader::copyTo(const ofTexture&, ofFloatPixels&, ofPixelFormat);

int Reader::getFrameSize() const {
  return frames.getSize();
}

void Reader::setFrameSize(int value) {
  frames.setSize(value);
}

template <class T>
Frame<Reader::Data>& Reader::allocate(int width, int height, GLenum format) {
  // Allocate a new buffer object for the first use, or discard the current
  // content and request a new memory when the size differs.
  const auto channels = ofGetNumChannelsFromGLFormat(format);
  const auto size = width * height * channels * sizeof(T);
  auto& current = frames.getCurrentFrame();
  if (!current->isAllocated()) {
    current->allocate();
  }
  if (current->size() != size) {
    current->bind(GL_PIXEL_PACK_BUFFER);
    current->setData(size, nullptr, GL_STREAM_READ);
    current->unbind(GL_PIXEL_PACK_BUFFER);
  }
  return current;
}

template <class T>
Pixels<const T> Reader::bind() {
  assert(!bound);
  auto& next = frames.getNextFrame();
  if (!next->isAllocated()) {
    return Pixels<const T>();
  }
  next->bind(GL_PIXEL_PACK_BUFFER);
  const auto ptr = next->map<const T>(GL_READ_ONLY);
  if (!ptr) {
    next->unbind(GL_PIXEL_PACK_BUFFER);
    return Pixels<const T>();
  }
  bound = true;
  const auto& data = next.getData();
  return Pixels<const T>(ptr, data.width, data.height, data.format);
}

void Reader::unbind() {
  if (bound) {
    auto& next = frames.getNextFrame();
    next->unmap();
    next->unbind(GL_PIXEL_PACK_BUFFER);
    bound = false;
  }
}

}  // namespace ofxasynctransfer
