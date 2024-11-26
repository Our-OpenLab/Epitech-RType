#ifndef SPARSE_ARRAY_HPP_
#define SPARSE_ARRAY_HPP_

#include <vector>
#include <optional>
#include <iostream>

namespace ecs {
  template <typename Component>
  class SparseArray {
    using value_type = std::optional<Component>;
    using container_t = std::vector<value_type>;
    using size_type = typename container_t::size_type;

  public:
    class Iterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = std::pair<size_t, Component>;
      using pointer = value_type*;
      using reference = value_type&;

      Iterator(typename container_t::Iterator it, typename container_t::Iterator end)
            : _current(it), _end(end) {
        advance_to_valid();
      }

      reference operator*() const {
        return _current->value();
      }

      pointer operator->() const {
        return &(_current->value());
      }

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
      typename container_t::Iterator _current;
      typename container_t::Iterator _end;

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

    value_type& operator[](size_type idx) {
      if (idx >= _data.size()) _data.resize(idx + 1);
      return _data[idx];
    }

    const value_type& operator[](size_type idx) const {
      return (idx < _data.size()) ? _data[idx] : _empty;
    }

    void insert_at(size_type idx, const Component& component) {
      if (idx >= _data.size()) _data.resize(idx + 1);
      _data[idx] = component;
    }

    void erase(size_type idx) {
      if (idx < _data.size()) _data[idx].reset();
    }

    Iterator begin() {
      return iterator(_data.begin(), _data.end());
    }

    Iterator end() {
      return iterator(_data.end(), _data.end());
    }

    size_type size() const {
      return _data.size();
    }

  private:
    container_t _data;
    static inline const value_type _empty{};
  };
}

#endif // SPARSE_ARRAY_HPP_
