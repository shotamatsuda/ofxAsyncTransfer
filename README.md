ofxAsyncTransfer
================

Addon for asynchronous pixel transfers using Pixel Buffer Objects for techniques like [streaming texture uploads](http://www.songho.ca/opengl/gl_pbo.html#unpack) and [asynchronous read-back](http://www.songho.ca/opengl/gl_pbo.html#pack)

[![License](http://img.shields.io/badge/license-MIT-lightgrey.svg?style=flat)](http://mit-license.org)

## Usage

### Asynchronous Reading

Suppose a dynamic texture source like `ofVideoGrabber`, persistent pixels for storing the contents of the texture, and an instance of `ofxAsyncTransferReader`:

```cpp
ofVideoGrabber grabber;
ofPixels pixels;
ofxAsyncTransferReader reader;
```

In `update()` or other functions periodically called:

```cpp
grabber.update();
reader.copyToPixels(grabber.getTexture(), pixels);
```

When the frame size of `reader` is more than 1, `copyToPixels()` asynchronously transfers the contents of a texture to a front pixel buffer in its circular buffer, and copies the contents of a back pixel buffer to `pixels`, so that tranfering GPU memory to CPU one does not block execution of copying pixels. This will give a boost in data transfer throughput and frame rate with the cost of some amount of CPU usage, GPU memory, and importantly, latency.

`copyToPixels()` is just a shorthand function, which is equivalent to the code below except an automatic allocation of `pixels`:

```cpp
grabber.update();
const auto bound = reader.bind<unsigned char>(grabber.getTexture());
std::copy(std::begin(bound), std::end(bound), pixels.getData());
reader.unbind();
```

Or using a shorthand function:

```cpp
grabber.update();
const auto bound = reader.bindAsPixels(grabber.getTexture());
std::copy(std::begin(bound), std::end(bound), pixels.getData());
reader.unbind();
```

### Reading from GPU Targets

TBD

```cpp
ofTexture texture;
ofxAsyncTransferReader reader;
```

```cpp
const auto pixels = reader.bind<unsigned char>(texture);
processPixels(pixels);
reader.unbind();
```

```cpp
reader.copyToPixels<unsigned char>(texture, pixels);
```

### Writing to GPU Targets

TBD

```cpp
ofTexture texture;
ofxAsyncTransferWriter writer;
```

```cpp
const auto pixels = writer.bind<unsigned char>(texture);
updatePixels(pixels);
writer.unbind();
```

## Implementation Details

### Constness of Pixels

There is a workaround in `ofxAsyncTransferPixels_<T>` for avoiding the violation of constness of its data, since `ofPixels_<T>` does not accept const types in its template argument.

When a `ofxAsyncTransferPixels_` is specialized with `const unsigned char`, the returned type of `getPixels()` will be `const ofPixels_<unsigned char>` so that it retains constness for most of the time except in cases where `const_cast` is used for it.

### Temporary Returned Value and Implicit Conversion

`ofxAsyncTransferPixels_<T>` is implicitly convertible to `ofPixels_<T>`. Note that the returned type of `bind()` is a temporary object and that of `getPixels()` is a Lvalue reference of a member variable of `Pixels_<T>`. The following code does not work because the returned value will be immediately destructed.

```cpp
const ofPixels& pixels = reader.bindAsPixels(target);
// Do something to the pixels
```

Instead of the code above, you should do like:

```cpp
const auto bound = reader.bindAsPixels(target);
const ofPixels& pixels = bound;
// Do something to the pixels
```

so that the returned value remains alive.

### Shorthand Functions

Not really shorthands with respect to the number of characters, but for those who are not familiar with template meta programming, those will be easier to handle.

Shorthand | Full
--- | ---
`bindAsPixels()` | `bind<unsigned char>()`
`bindAsShortPixels()` | `bind<unsigned short>()`
`bindAsFloatPixels()` | `bind<float>()`

## References

- [Official documentation on Pixel Buffer Object](https://www.opengl.org/wiki/Pixel_Buffer_Object)
- [Descriptions about async tranfers](http://www.songho.ca/opengl/gl_pbo.html)

## License

The MIT License

Copyright (C) 2016 Shota Matsuda

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
