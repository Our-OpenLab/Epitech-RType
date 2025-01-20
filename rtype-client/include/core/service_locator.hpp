#ifndef SERVICE_LOCATOR_HPP_
#define SERVICE_LOCATOR_HPP_

#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>

/**
 * @brief A generic service locator for dependency injection with support for multiple instances.
 *
 * This utility provides a way to register and access shared services across the application.
 * Each service can be identified by a unique key.
 */
class ServiceLocator {
public:
  /**
 * @brief Registers a shared service instance with a key.
 *
 * This method allows you to register a service with a unique key.
 * The service can later be retrieved using the same key.
 *
 * @tparam T The type of the service to register.
 * @param key A unique key to identify the service instance.
 * @param service The shared pointer to the service instance.
 */
  template <typename T>
  static void Provide(const std::string& key, std::shared_ptr<T> service) {
    auto& storage = GetStorage<T>();
    storage[key] = std::move(service);
  }

  /**
   * @brief Registers a raw service instance with a key.
   *
   * This method registers a raw pointer to a service instance with a unique key.
   * The raw pointer is wrapped in a `std::shared_ptr` with a custom deleter
   * that prevents deletion, assuming the service's lifetime is managed elsewhere.
   *
   * Note: Use this method with caution, as it bypasses `shared_ptr` ownership.
   *
   * @tparam T The type of the service to register.
   * @param key A unique key to identify the service instance.
   * @param service A raw pointer to the service instance.
   */
  template <typename T>
  static void Provide(const std::string& key, T* service) {
    GetStorage<T>()[key] = std::shared_ptr<T>(service, [](T*) {});
  }

  /**
   * @brief Retrieves the registered service instance by key.
   *
   * Throws an exception if the service is not found.
   *
   * @tparam T The type of the service to retrieve.
   * @param key The unique key identifying the service instance.
   * @return A reference to the registered service instance.
   */
  template <typename T>
  static T& Get(const std::string& key) {
    auto& storage = GetStorage<T>();
    auto it = storage.find(key);
    if (it == storage.end()) {
      throw std::runtime_error("Service not found: " + std::string(typeid(T).name()) + " with key: " + key);
    }
    return *(it->second);
  }

  /**
   * @brief Retrieves the shared pointer of the registered service instance by key.
   *
   * Throws an exception if the service is not found.
   *
   * @tparam T The type of the service to retrieve.
   * @param key The unique key identifying the service instance.
   * @return A shared pointer to the registered service instance.
   */
  template <typename T>
  static std::shared_ptr<T> GetShared(const std::string& key) {
    auto& storage = GetStorage<T>();
    auto it = storage.find(key);
    if (it == storage.end()) {
      throw std::runtime_error("Service not found: " + std::string(typeid(T).name()) + " with key: " + key);
    }
    return it->second;
  }

  /**
   * @brief Checks if a service of the specified type and key is registered.
   *
   * @tparam T The type of the service to check.
   * @param key The unique key identifying the service instance.
   * @return True if the service is registered, false otherwise.
   */
  template <typename T>
  static bool Has(const std::string& key) {
    auto& storage = GetStorage<T>();
    return storage.find(key) != storage.end();
  }

  /**
   * @brief Removes the registered service instance by key.
   *
   * If the service is not registered, this operation does nothing.
   *
   * @tparam T The type of the service to remove.
   * @param key The unique key identifying the service instance.
   */
  template <typename T>
  static void Remove(const std::string& key) {
    auto& storage = GetStorage<T>();
    storage.erase(key);
  }

private:
  /**
   * @brief Retrieves the storage for a specific service type.
   *
   * This method uses a static unordered_map to store shared pointers
   * for the services of the specified type, identified by keys.
   *
   * @tparam T The type of the service.
   * @return A reference to the unordered_map storage.
   */
  template <typename T>
  static std::unordered_map<std::string, std::shared_ptr<T>>& GetStorage() {
    static std::unordered_map<std::string, std::shared_ptr<T>> storage;
    return storage;
  }
};

#endif  // SERVICE_LOCATOR_HPP_
