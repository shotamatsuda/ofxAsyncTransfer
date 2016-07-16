//
//  ofxAsyncTransferFrames.h
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

#include <algorithm>
#include <deque>

#include "ofxAsyncTransferFrame.h"

namespace ofxasynctransfer {

// A circular pixel buffer with pointers for the current and next operations.

template <class Data>
class Frames final {
 public:
  explicit Frames(int size);

  // Disallow copy semantics
  Frames(const Frames&) = delete;
  Frames& operator=(const Frames&) = delete;

  // Move semantics
  Frames(Frames&&) = default;
  Frames& operator=(Frames&&) = default;

  // Controlling frame
  void nextFrame();
  int getSize() const { return size; }
  void setSize(int value);

  // Buffer objects
  Frame<Data>& getCurrentFrame() { return frames.at(current); }
  const Frame<Data>& getCurrentFrame() const { return frames.at(current); }
  Frame<Data>& getNextFrame() { return frames.at(next); }
  const Frame<Data>& getNextFrame() const { return frames.at(next); }

 private:
  std::deque<Frame<Data>> frames;
  int size;
  int current;
  int next;
};

template <class Data>
inline Frames<Data>::Frames(int size)
    : frames(std::max(1, size)),
      size(frames.size()),
      current(),
      next() {}

template <class Data>
inline void Frames<Data>::nextFrame() {
  if (size > frames.size()) {
    frames.resize(size);
  } else if (size < frames.size()) {
    const auto shift = std::min<int>(frames.size() - size, next);
    frames.erase(frames.begin(), frames.begin() + shift);
    frames.erase(frames.end() - (size - shift), frames.end());
    current -= shift;
  }
  current = (current + 1) % frames.size();
  next = (current + 1) % frames.size();
}

template <class Data>
inline void Frames<Data>::setSize(int value) {
  size = std::max(1, value);
}

}  // namespace ofxasynctransfer
