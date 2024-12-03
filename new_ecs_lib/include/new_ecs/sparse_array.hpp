#include <optional>
#include <stdexcept>
#include <vector>
#include <iostream>

namespace ecs {

template <typename Component>
class SparseArray final {
 public:
  using value_type = Component;
  using container_t = std::vector<std::optional<Component>>;
  using size_type = std::size_t;

  SparseArray() = default;
  SparseArray(const SparseArray&) = default;
  SparseArray(SparseArray&&) noexcept = default;
  ~SparseArray() = default;

  SparseArray& operator=(const SparseArray&) = default;
  SparseArray& operator=(SparseArray&&) noexcept = default;

  Component& insert_at(size_type idx, const Component& component) {
    ensure_size(idx);
    data_[idx] = component;
    set_bit(valid_mask_, idx);
    return *data_[idx];
  }

  template <typename... Params>
  Component& emplace_at(size_type idx, Params&&... params) {
    ensure_size(idx);
    data_[idx].emplace(std::forward<Params>(params)...);
    set_bit(valid_mask_, idx);
    return *data_[idx];
  }

  void erase(size_type idx) {
    if (idx >= data_.size() || !test_bit(valid_mask_, idx)) return;
    clear_bit(valid_mask_, idx);
    data_[idx].reset();  // Réinitialise explicitement le composant
  }

  Component& operator[](size_type idx) {
    if (!test_bit(valid_mask_, idx))
      throw std::runtime_error("Invalid component access");
    return *data_[idx];
  }

  const Component& operator[](size_type idx) const {
    if (!test_bit(valid_mask_, idx))
      throw std::runtime_error("Invalid component access");
    return *data_[idx];
  }

  bool is_valid(const size_type idx) const {
    bool valid = idx < data_.size() && test_bit(valid_mask_, idx);
    std::cout << "[DEBUG] SparseArray is_valid(" << idx << "): " << valid << "\n";
    return valid;
  }

  [[nodiscard]] size_type size() const { return data_.size(); }

 private:
  container_t data_;
  std::vector<uint64_t> valid_mask_;

  void ensure_size(size_type idx) {
    if (idx >= data_.size()) {
      size_type new_size = idx + 1;
      data_.resize(new_size);
      size_type mask_size = (new_size + 63) / 64;
      valid_mask_.resize(mask_size, 0);

      std::cout << "[DEBUG] SparseArray resized to " << new_size << " elements.\n";
    }
  }

  static void set_bit(std::vector<uint64_t>& mask, size_type idx) {
    const size_type word = idx / 64;
    const size_type bit = idx % 64;
    if (word >= mask.size()) mask.resize(word + 1, 0);
    mask[word] |= (1ULL << bit);
  }

  static void clear_bit(std::vector<uint64_t>& mask, size_type idx) {
    const size_type word = idx / 64;
    const size_type bit = idx % 64;
    if (word < mask.size()) mask[word] &= ~(1ULL << bit);
  }

  static bool test_bit(const std::vector<uint64_t>& mask, const size_type idx) {
    const size_type word = idx / 64;
    const size_type bit = idx % 64;
    return word < mask.size() && (mask[word] & (1ULL << bit));
  }
};

}  // namespace ecs
