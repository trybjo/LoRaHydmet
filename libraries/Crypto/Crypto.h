/*
    2  * Copyright (C) 2015 Southern Storm Software, Pty Ltd.
    3  *
    4  * Permission is hereby granted, free of charge, to any person obtaining a
    5  * copy of this software and associated documentation files (the "Software"),
    6  * to deal in the Software without restriction, including without limitation
    7  * the rights to use, copy, modify, merge, publish, distribute, sublicense,
    8  * and/or sell copies of the Software, and to permit persons to whom the
    9  * Software is furnished to do so, subject to the following conditions:
   10  *
   11  * The above copyright notice and this permission notice shall be included
   12  * in all copies or substantial portions of the Software.
   13  *
   14  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   15  * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   16  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   17  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   18  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   19  * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   20  * DEALINGS IN THE SOFTWARE.
   21  */

  #ifndef CRYPTO_h
  #define CRYPTO_h
  
  #include <inttypes.h>
  #include <stddef.h>
  
  void clean(void *dest, size_t size);
  
  template <typename T>
  inline void clean(T &var)
  {
      clean(&var, sizeof(T));
  }
  
  bool secure_compare(const void *data1, const void *data2, size_t len);
  
  #if defined(ESP8266)
  extern "C" void system_soft_wdt_feed(void);
  #define crypto_feed_watchdog() system_soft_wdt_feed()
  #else
  #define crypto_feed_watchdog() do { ; } while (0)
  #endif
  
  #endif