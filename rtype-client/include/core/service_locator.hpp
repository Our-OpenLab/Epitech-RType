#ifndef SERVICE_LOCATOR_HPP_
#define SERVICE_LOCATOR_HPP_

#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

/**
 * @brief A generic service locator for dependency injection.
 *
 * This utility provides a way to register and access shared services
 * across the application. It is useful in scenarios where you want to decouple
 * the instantiation and usage of services.
 */
class ServiceLocator {
public:
  /**
   * @brief Registers a shared service instance.
   *
   * @tparam T The type of the service to register.
   * @param service The shared pointer to the service instance.
   */
  template <typename T>
  static void Provide(std::shared_ptr<T> service) {
    GetStorage<T>() = std::move(service);
  }

  /**
   * @brief Retrieves the registered service instance.
   *
   * Throws an exception if the service is not found.
   *
   * @tparam T The type of the service to retrieve.
   * @return A reference to the registered service instance.
   */
  template <typename T>
  static T& Get() {
    auto& service = GetStorage<T>();
    if (!service) {
      throw std::runtime_error("Service not found: " + std::string(typeid(T).name()));
    }
    return *service;
  }

  /**
   * @brief Retrieves the shared pointer of the registered service instance.
   *
   * Throws an exception if the service is not found.
   *
   * @tparam T The type of the service to retrieve.
   * @return A shared pointer to the registered service instance.
   */
  template <typename T>
  static std::shared_ptr<T> GetShared() {
    auto& service = GetStorage<T>();
    if (!service) {
      throw std::runtime_error("Service not found: " + std::string(typeid(T).name()));
    }
    return service;
  }

  /**
   * @brief Checks if a service of the specified type is registered.
   *
   * @tparam T The type of the service to check.
   * @return True if the service is registered, false otherwise.
   */
  template <typename T>
  static bool Has() {
    return GetStorage<T>() != nullptr;
  }

  /**
   * @brief Removes the registered service instance.
   *
   * If the service is not registered, this operation does nothing.
   *
   * @tparam T The type of the service to remove.
   */
  template <typename T>
  static void Remove() {
    GetStorage<T>().reset();
  }

private:
  /**
   * @brief Retrieves the storage for a specific service type.
   *
   * This method uses a static variable to store the shared pointer
   * for the service of the specified type.
   *
   * @tparam T The type of the service.
   * @return A reference to the shared pointer storage.
   */
  template <typename T>
  static std::shared_ptr<T>& GetStorage() {
    static std::shared_ptr<T> storage;
    return storage;
  }
};

#endif  // SERVICE_LOCATOR_HPP_
