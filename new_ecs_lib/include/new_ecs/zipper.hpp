#ifndef ZIPPER_HPP_
#define ZIPPER_HPP_

#include <tuple>
#include <optional>
#include <iterator>
#include <functional>
#include <vector>
#include <stdexcept>

namespace ecs {

template <typename Entities, typename... Containers>
class Zipper {
public:
  explicit Zipper(Entities& entities, Containers&... containers)
      : entities_(entities), containers_(std::tie(containers...)) {}

  class Iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<typename Containers::value_type&...>;
    using reference = value_type;

    Iterator(Entities& entities, std::tuple<Containers&...>& containers, size_t index)
        : entities_(entities), containers_(containers), index_(index) {}

    reference operator*() {
      return dereference(std::make_index_sequence<sizeof...(Containers)>{});
    }

    Iterator& operator++() {
      ++index_;
      std::cout << "[DEBUG] Zipper Iterator advanced to index " << index_ << "\n";
      return *this;
    }

    bool operator!=(const Iterator& other) const {
      return index_ != other.index_;
    }

  private:
    Entities& entities_;
    std::tuple<Containers&...>& containers_;
    size_t index_;

    template <size_t... Is>
  reference dereference(std::index_sequence<Is...>) {
      auto entity = entities_[index_];

      // Log l'entité et ses composants
      std::cout << "[DEBUG] Accessing entity " << entity << " at index " << index_ << ": ";

      // Validez les composants avant de les accéder
      ((void)check_validity(std::get<Is>(containers_), entity), ...);

      // Log la validité des composants
      ((std::cout << "Component " << Is << " valid: "
                  << std::get<Is>(containers_).is_valid(entity) << ", "), ...);
      std::cout << "\n";

      return std::tie(std::get<Is>(containers_)[entity]...);
    }

    template <typename Container>
    void check_validity(Container& container, size_t entity) const {
      if (entity >= container.size() || !container.is_valid(entity)) {
        throw std::runtime_error("Invalid component access in Zipper for entity " + std::to_string(entity));
      }
    }


  };

  Iterator begin() {
    std::cout << "[DEBUG] Zipper begin()\n";
    return Iterator(entities_, containers_, 0);
  }

  Iterator end() {
    std::cout << "[DEBUG] Zipper end()\n";
    return Iterator(entities_, containers_, entities_.size());
  }

private:
  Entities& entities_;
  std::tuple<Containers&...> containers_;
};

}  // namespace ecs

#endif  // ZIPPER_HPP_
