#pragma once

// afterhours includes this file as ../../expected.hpp from vendor/afterhours/src/.
// Provide a minimal `tl::expected` + `tl::unexpected` (std::expected is not
// reliably available in all standard library builds shipped with this project).

#include <utility>

namespace tl {

template <class E>
class unexpected {
 public:
  E error;
  explicit unexpected(const E &e) : error(e) {}
  explicit unexpected(E &&e) : error(std::move(e)) {}
};

template <class E>
unexpected(E) -> unexpected<E>;

template <class T, class E>
class expected {
 public:
  expected(const T &v) : has_(true) { new (&storage_.value) T(v); }
  expected(T &&v) : has_(true) { new (&storage_.value) T(std::move(v)); }

  expected(const unexpected<E> &u) : has_(false) {
    new (&storage_.error) E(u.error);
  }
  expected(unexpected<E> &&u) : has_(false) {
    new (&storage_.error) E(std::move(u.error));
  }

  expected(const expected &other) : has_(other.has_) {
    if (has_) {
      new (&storage_.value) T(other.storage_.value);
    } else {
      new (&storage_.error) E(other.storage_.error);
    }
  }

  expected(expected &&other) noexcept : has_(other.has_) {
    if (has_) {
      new (&storage_.value) T(std::move(other.storage_.value));
    } else {
      new (&storage_.error) E(std::move(other.storage_.error));
    }
  }

  expected &operator=(const expected &other) {
    if (this == &other) return *this;
    this->~expected();
    new (this) expected(other);
    return *this;
  }

  expected &operator=(expected &&other) noexcept {
    if (this == &other) return *this;
    this->~expected();
    new (this) expected(std::move(other));
    return *this;
  }

  ~expected() {
    if (has_) {
      storage_.value.~T();
    } else {
      storage_.error.~E();
    }
  }

  [[nodiscard]] bool has_value() const { return has_; }
  explicit operator bool() const { return has_; }

  T &value() & { return storage_.value; }
  const T &value() const & { return storage_.value; }
  T &&value() && { return std::move(storage_.value); }

  E &error() & { return storage_.error; }
  const E &error() const & { return storage_.error; }

 private:
  bool has_ = false;
  union Storage {
    T value;
    E error;
    Storage() {}
    ~Storage() {}
  } storage_;
};

}  // namespace tl
