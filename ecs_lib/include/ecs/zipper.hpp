#ifndef ZIPPER_HPP_
#define ZIPPER_HPP_

#include <tuple>
#include <optional>
#include <iterator>
#include <functional>
#include <stdexcept>

namespace ecs {
template <typename... Containers>
class Zipper final {
public:
  explicit Zipper(Containers&... containers)
      : containers_(std::tie(containers...)),
        max_size_(compute_max_size(containers...)) {}


  class Iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<typename Containers::value_type...>;
    using reference = std::tuple<typename Containers::value_type&...>;

    Iterator(std::tuple<Containers&...>& containers, size_t index, size_t max_size)
        : containers_(containers), index_(index), max_size_(max_size) {
      skip_invalid();
    }

    reference operator*() {
      return dereference(std::make_index_sequence<sizeof...(Containers)>{});
    }

    Iterator& operator++() {
      ++index_;
      skip_invalid();
      return *this;
    }

    bool operator!=(const Iterator& other) const {
      return index_ != other.index_;
    }

    size_t get_index() const {
      return index_;
    }

  private:
    std::tuple<Containers&...>& containers_;
    size_t index_;
    size_t max_size_;

    template <size_t... Is>
    reference dereference(std::index_sequence<Is...>) {
      return std::tie(std::get<Is>(containers_)[index_]...);
    }

    void skip_invalid() {
      while (index_ < max_size_ && !all_valid()) {
        ++index_;
      }
    }

    bool all_valid() {
      return std::apply(
          [&](auto&... containers) {
            return (... && std::get<Containers&>(containers_)[index_].has_value());
          },
          containers_);
    }
  };

  Iterator begin() { return Iterator(containers_, 0, max_size_); }
  Iterator end() { return Iterator(containers_, max_size_, max_size_); }

private:
  std::tuple<Containers&...> containers_;
  size_t max_size_;

  static size_t compute_max_size(Containers&... containers) {
    return std::min({containers.size()...});
  }

};

template <typename... Containers>
Zipper(Containers&...) -> Zipper<Containers...>;

}

#endif // ZIPPER_HPP_
