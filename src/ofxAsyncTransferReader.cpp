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
#include <cstdint>
#include <iterator>

#include "ofConstants.h"
#include "ofFbo.h"
#include "ofGLUtils.h"
#include "ofTexture.h"
#include "ofxAsyncTransferPixels.h"
#include "ofxAsyncTransferUtils.h"

namespace ofxasynctransfer {

Reader::Reader(int frames) : frames(frames), bound() {}

void Reader::setup(int frames) {
  this->frames.setSize(frames);
}

GLenum Reader::getFormat(const ofFbo& fbo,
                         ofPixelFormat pixelFormat) const {
  if (pixelFormat != OF_PIXELS_UNKNOWN) {
    return ofGetGLFormatFromPixelFormat(pixelFormat);
  }
  if (!fbo.isAllocated()) {
    return GLenum();
  }
  const auto& texture = fbo.getTexture();
  if (!texture.isAllocated()) {
    return GLenum();
  }
  return ofGetGLFormatFromInternal(texture.getTextureData().glInternalFormat);
}

GLenum Reader::getFormat(const ofTexture& texture,
                         ofPixelFormat pixelFormat) const {
  if (pixelFormat != OF_PIXELS_UNKNOWN) {
    return ofGetGLFormatFromPixelFormat(pixelFormat);
  }
  if (!texture.isAllocated()) {
    return GLenum();
  }
  return ofGetGLFormatFromInternal(texture.getTextureData().glInternalFormat);
}

template <class T>
bool Reader::allocatePixels(const ofFbo& fbo,
                            ofPixels_<T>& pixels,
                            ofPixelFormat pixelFormat) const {
  const auto width = fbo.getWidth();
  const auto height = fbo.getHeight();
  if (!width || !height) {
    return false;
  }
  if (pixelFormat != OF_PIXELS_UNKNOWN) {
    pixels.allocate(width, height, pixelFormat);
    return pixels.isAllocated();
  }
  if (!fbo.isAllocated()) {
    return false;
  }
  const auto& texture = fbo.getTexture();
  if (!texture.isAllocated()) {
    return false;
  }
  pixels.allocate(width, height, ofGetImageTypeFromGLType(
      texture.getTextureData().glInternalFormat));
  return pixels.isAllocated();
}

template <class T>
bool Reader::allocatePixels(const ofTexture& texture,
                            ofPixels_<T>& pixels,
                            ofPixelFormat pixelFormat) const {
  const auto width = texture.getWidth();
  const auto height = texture.getHeight();
  if (!width || !height) {
    return false;
  }
  if (pixelFormat != OF_PIXELS_UNKNOWN) {
    pixels.allocate(width, height, pixelFormat);
    return pixels.isAllocated();
  }
  if (!texture.isAllocated()) {
    return false;
  }
  pixels.allocate(width, height, ofGetImageTypeFromGLType(
      texture.getTextureData().glInternalFormat));
  return pixels.isAllocated();
}

template <class T>
void Reader::copyToPixels(const ofTexture& texture,
                          ofPixels_<T>& pixels,
                          ofPixelFormat pixelFormat) {
  if (!allocatePixels(texture, pixels, pixelFormat)) {
    return;  // Failed to allocate pixels
  }
  const auto format = getFormat(texture, pixelFormat);
  if (!format) {
    return;  // Could not determine the buffer format
  }
  const auto result = bind<T>(texture, format);
  if (result) {
    const auto begin = std::begin(result);
    const auto end = std::end(result);
    assert(sizeof(T) * std::distance(begin, end) == pixels.getTotalBytes());
    std::copy(begin, end, pixels.getData());
    unbind();
  }
}

template void Reader::copyToPixels(const ofTexture&,
                                   ofPixels&,
                                   ofPixelFormat);
template void Reader::copyToPixels(const ofTexture&,
                                   ofShortPixels&,
                                   ofPixelFormat);
template void Reader::copyToPixels(const ofTexture&,
                                   ofFloatPixels&,
                                   ofPixelFormat);

template <class T>
void Reader::copyToPixels(const ofFbo& fbo,
                          ofPixels_<T>& pixels,
                          ofPixelFormat pixelFormat) {
  if (!allocatePixels(fbo, pixels, pixelFormat)) {
    return;  // Failed to allocate pixels
  }
  const auto format = getFormat(fbo, pixelFormat);
  if (!format) {
    return;  // Could not determine the buffer format
  }
  const auto result = bind<T>(fbo, format);
  if (result) {
    const auto begin = std::begin(result);
    const auto end = std::end(result);
    assert(sizeof(T) * std::distance(begin, end) == pixels.getTotalBytes());
    std::copy(begin, end, pixels.getData());
    unbind();
  }
}

template void Reader::copyToPixels(const ofFbo&,
                                   ofPixels&,
                                   ofPixelFormat);
template void Reader::copyToPixels(const ofFbo&,
                                   ofShortPixels&,
                                   ofPixelFormat);
template void Reader::copyToPixels(const ofFbo&,
                                   ofFloatPixels&,
                                   ofPixelFormat);

template <class T>
Pixels_<const T> Reader::bind(const ofTexture& texture, GLenum format) {
  if (bound) {
    unbind();
  }
  frames.nextFrame();  // Request a next available frame
  const auto width = texture.getWidth();
  const auto height = texture.getHeight();
  auto& current = allocate<T>(width, height, format);

  // Read the texture into a buffer object for the current frame
  const auto channels = ofGetNumChannelsFromGLFormat(format);
  const auto target = texture.getTextureData().textureTarget;
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

template Pixels_<const std::uint8_t> Reader::bind(const ofTexture&, GLenum);
template Pixels_<const std::int8_t> Reader::bind(const ofTexture&, GLenum);
template Pixels_<const std::uint16_t> Reader::bind(const ofTexture&, GLenum);
template Pixels_<const std::int16_t> Reader::bind(const ofTexture&, GLenum);
template Pixels_<const std::uint32_t> Reader::bind(const ofTexture&, GLenum);
template Pixels_<const std::int32_t> Reader::bind(const ofTexture&, GLenum);
template Pixels_<const float> Reader::bind(const ofTexture&, GLenum);

template <class T>
Pixels_<const T> Reader::bind(const ofFbo& fbo, GLenum format) {
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

template Pixels_<const std::uint8_t> Reader::bind(const ofFbo&, GLenum);
template Pixels_<const std::int8_t> Reader::bind(const ofFbo&, GLenum);
template Pixels_<const std::uint16_t> Reader::bind(const ofFbo&, GLenum);
template Pixels_<const std::int16_t> Reader::bind(const ofFbo&, GLenum);
template Pixels_<const std::uint32_t> Reader::bind(const ofFbo&, GLenum);
template Pixels_<const std::int32_t> Reader::bind(const ofFbo&, GLenum);
template Pixels_<const float> Reader::bind(const ofFbo&, GLenum);

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
  const auto bytes = width * height * channels * sizeof(T);
  auto& current = frames.getCurrentFrame();
  if (!current->isAllocated()) {
    current->allocate();
  }
  if (current->size() != bytes) {
    current->bind(GL_PIXEL_PACK_BUFFER);
    current->setData(bytes, nullptr, GL_STREAM_READ);
    current->unbind(GL_PIXEL_PACK_BUFFER);
  }
  return current;
}

template <class T>
Pixels_<const T> Reader::bind() {
  assert(!bound);
  auto& next = frames.getNextFrame();
  if (!next->isAllocated()) {
    return Pixels_<const T>();
  }
  next->bind(GL_PIXEL_PACK_BUFFER);
  const auto ptr = next->map<const T>(GL_READ_ONLY);
  if (!ptr) {
    next->unbind(GL_PIXEL_PACK_BUFFER);
    return Pixels_<const T>();
  }
  bound = true;
  const auto& data = next.getData();
  return Pixels_<const T>(ptr, data.width, data.height, data.format);
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
