/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <ametsuchi/exception.h>
#include <cmath>

namespace ametsuchi {
namespace buffer {

/**
 * Basic circular buffer
 */
template <typename T>
class CircularStack {
 public:
  CircularStack(size_t);
  CircularStack(CircularStack &&);
  CircularStack(const CircularStack &) = delete;
  ~CircularStack();

  /**
   * Performs reallocation of the data
   * so the buffer able to store provided
   * number of elems
   * O(n)
   */
  void resize(size_t);

  /**
   * O(1)
   * Add element to the buffer end pointer
   */
  void push(T &&);

  /**
   * O(1)
   * Add element to the buffer end pointer
   */
  void push(const T &);
  /**
   * O(1)
   * @param n elemens for removing
   */
  void pop(size_t n = 1);


  /**
   * O(1)
   * Accessing element by front.
   */
  T &front();
  T front() const;

  /**
   * O(1)
   * Accessing element by back.
   */
  T &back();
  T back() const;


  /**
   * O(1)
   * Accessing element by index
   */
  T &operator[](size_t);
  T operator[](size_t) const;

  /**
   * Number of elements
   */
  inline size_t size() const { return sz; }
  /**
   * Maximum storage capacity
   */
  inline size_t capacity() const { return cap; }

 private:
  // Raw data array
  T *v;
  // Storage capacity
  size_t cap;
  // Number of elements stored so far, can be in range [0;cap]
  size_t sz;
  // Index of after last element, can be in range [0;sz)
  size_t i_end;

  constexpr size_t start() const { return diff(i_end, size()); }

  // Determines n position before p
  constexpr size_t diff(size_t p, size_t n) const {
    return p < n ? p + capacity() - n : p - n;
  }

  constexpr size_t overflowed() { return capacity(); }

 public:
  class ForwardIter {
   public:
    ForwardIter(CircularStack<T> *, size_t);
    bool operator==(const ForwardIter &i) const;
    bool operator!=(const ForwardIter &i) const;
    ForwardIter &operator++();
    ForwardIter operator+(size_t t);
    ForwardIter &operator+=(size_t t);
    T &operator*();
    T &operator[](size_t t);
    size_t size() const { return cs->size(); }
    ForwardIter &to_last() {
      pos = cs->diff(cs->i_end, 1);
      return *this;
    }

   private:
    CircularStack<T> *cs;
    size_t pos;

    size_t pos_inc(size_t t);
  };


  ForwardIter begin() { return ForwardIter(this, diff(i_end, size())); }

  ForwardIter end() { return ForwardIter(this, overflowed()); }

  ForwardIter last() { return ForwardIter(this, diff(i_end, 1)); }

 private:
};

template <typename T>
CircularStack<T>::CircularStack(size_t s) : cap(s), sz(0), i_end(0) {
  if (s == 0)
    throw exception::Exception::Exception("Buffer size cannot be zero");
  v = (T *)malloc(sizeof(T) * capacity());
}

template <typename T>
CircularStack<T>::CircularStack(CircularStack &&cs) {
  v = cs.v;
  cap = cs.cap;
  sz = cs.sz;
  i_end = cs.i_end;
  cs.v = nullptr;
}

template <typename T>
CircularStack<T>::~CircularStack() {
  if (v) free(v);
  v = nullptr;
}

template <typename T>
void CircularStack<T>::resize(size_t s) {
  cap = s;
  sz = std::min(capacity(), size());
  v = (T *)realloc(v, sizeof(T) * capacity());
}

template <typename T>
void CircularStack<T>::push(T &&t) {
  v[i_end] = std::move(t);
  i_end = (i_end + 1) % capacity();
  if (size() < capacity()) sz++;
}

template <typename T>
void CircularStack<T>::push(const T &t) {
  v[i_end] = t;
  i_end = (i_end + 1) % capacity();
  if (size() < capacity()) sz++;
}

template <typename T>
void CircularStack<T>::pop(size_t n) {
  n = std::min(sz, n);
  sz = sz - n;
  i_end = (i_end + capacity() - n) % capacity();
}

template <typename T>
T &CircularStack<T>::front() {
  if (size() == 0) throw exception::Exception("Buffer accessing out of size");
  return v[diff(i_end, size())];
}
template <typename T>
T CircularStack<T>::front() const {
  if (size() == 0) throw exception::Exception("Buffer accessing out of size");
  return v[diff(i_end, size())];
}

template <typename T>
T &CircularStack<T>::back() {
  if (size() == 0) throw exception::Exception("Buffer accessing out of size");
  return v[diff(i_end, 1)];
}
template <typename T>
T CircularStack<T>::back() const {
  if (size() == 0) throw exception::Exception("Buffer accessing out of size");
  return v[diff(i_end, 1)];
}

template <typename T>
T &CircularStack<T>::operator[](size_t s) {
  if (s >= size()) throw exception::Exception("Buffer accessing out of size");
  return v[diff(i_end, size() - s)];
}

template <typename T>
T CircularStack<T>::operator[](size_t s) const {
  if (s >= size()) throw exception::Exception("Buffer accessing out of size");
  return v[diff(i_end, size() - s)];
}

template <typename T>
CircularStack<T>::ForwardIter::ForwardIter(CircularStack<T> *cs, size_t pos)
    : cs(cs), pos(pos) {}

template <typename T>
bool CircularStack<T>::ForwardIter::operator==(const ForwardIter &i) const {
  return cs->v == i.cs->v && pos == i.pos;
}

template <typename T>
bool CircularStack<T>::ForwardIter::operator!=(const ForwardIter &i) const {
  return !this->operator==(i);
}

template <typename T>
typename CircularStack<T>::ForwardIter
    &CircularStack<T>::ForwardIter::operator++() {
  pos = pos_inc(1);
  return *this;
}

template <typename T>
typename CircularStack<T>::ForwardIter CircularStack<T>::ForwardIter::operator+(
    size_t t) {
  return {cs, pos_inc(t)};
}

template <typename T>
typename CircularStack<T>::ForwardIter &CircularStack<T>::ForwardIter::
operator+=(size_t t) {
  pos = pos_inc(t);
  return *this;
}

template <typename T>
T &CircularStack<T>::ForwardIter::operator*() {
  return (*cs)[pos];
}

template <typename T>
T &CircularStack<T>::ForwardIter::operator[](size_t t) {
  return (*cs)[pos_inc(t)];
}

template <typename T>
size_t CircularStack<T>::ForwardIter::pos_inc(size_t t) {
  // temporal end, that bigger current position
  auto t_end = cs->i_end + pos >= cs->i_end ? cs->size() : 0;
  // calculate new position
  auto n_pos = pos + t;
  // ensure it doesn't overflow
  return n_pos >= t_end ? cs->overflowed() : n_pos % cs->cap;
}
}  // namespace buffer
}  // namespace ametsuchi
