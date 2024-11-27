#ifndef SPARSE_ARRAY_HPP_
#define SPARSE_ARRAY_HPP_

#include <optional>
#include <stdexcept>
#include <vector>

namespace ecs {

template <typename Component>
class SparseArray {
 public:
  using value_type = std::optional<Component>;
  using reference_type = value_type&;
  using const_reference_type = const value_type&;
  using container_t = std::vector<value_type>;
  using size_type = typename container_t::size_type;

  class Iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Component;
    using pointer = Component*;
    using reference = Component&;

    Iterator(typename container_t::iterator current,
             typename container_t::iterator end)
        : _current(current), _end(end) {
      advance_to_valid();
    }

    reference operator*() const { return _current->value(); }

    pointer operator->() const { return &(_current->value()); }

    Iterator& operator++() {
      ++_current;
      advance_to_valid();
      return *this;
    }

    Iterator operator++(int) {
      Iterator temp = *this;
      ++(*this);
      return temp;
    }

    friend bool operator==(const Iterator& a, const Iterator& b) {
      return a._current == b._current;
    }

    friend bool operator!=(const Iterator& a, const Iterator& b) {
      return !(a == b);
    }

   private:
    typename container_t::iterator _current;
    typename container_t::iterator _end;

    void advance_to_valid() {
      while (_current != _end && !_current->has_value()) {
        ++_current;
      }
    }
  };

  class ConstIterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = Component;
  using pointer = const Component*;
  using reference = const Component&;

  ConstIterator(typename container_t::const_iterator current,
                typename container_t::const_iterator end)
      : _current(current), _end(end) {
    advance_to_valid();
  }

  reference operator*() const { return _current->value(); }

  pointer operator->() const { return &(_current->value()); }

  ConstIterator& operator++() {
    ++_current;
    advance_to_valid();
    return *this;
  }

  ConstIterator operator++(int) {
    ConstIterator temp = *this;
    ++(*this);
    return temp;
  }

  friend bool operator==(const ConstIterator& a, const ConstIterator& b) {
    return a._current == b._current;
  }

  friend bool operator!=(const ConstIterator& a, const ConstIterator& b) {
    return !(a == b);
  }

 private:
  typename container_t::const_iterator _current;
  typename container_t::const_iterator _end;

  void advance_to_valid() {
    while (_current != _end && !_current->has_value()) {
      ++_current;
    }
  }
};

  SparseArray() = default;
  SparseArray(const SparseArray&) = default;
  SparseArray(SparseArray&&) noexcept = default;
  ~SparseArray() = default;

  SparseArray& operator=(const SparseArray&) = default;
  SparseArray& operator=(SparseArray&&) noexcept = default;

  reference_type operator[](size_type idx) {
    if (idx >= data_.size()) data_.resize(idx + 1);
    return data_[idx];
  }

  const_reference_type operator[](size_type idx) const {
    if (idx >= data_.size()) {
      throw std::out_of_range("Index out of range");
    }
    return data_[idx];
  }

  reference_type insert_at(size_type idx, const Component& component) {
    if (idx >= data_.size()) data_.resize(idx + 1);
    data_[idx] = component;
    return data_[idx];
  }

  template <typename... Params>
  reference_type emplace_at(size_type idx, Params&&... params) {
    if (idx >= data_.size()) data_.resize(idx + 1);
    data_[idx].emplace(std::forward<Params>(params)...);
    return data_[idx];
  }

  void erase(size_type idx) {
    if (idx < data_.size()) {
      data_[idx].reset();
    }
  }

  size_type size() const { return data_.size(); }

  Iterator begin() { return Iterator(data_.begin(), data_.end()); }

  Iterator end() { return Iterator(data_.end(), data_.end()); }

 private:
  container_t data_;
};

}  // namespace ecs

#endif  // SPARSE_ARRAY_HPP_
