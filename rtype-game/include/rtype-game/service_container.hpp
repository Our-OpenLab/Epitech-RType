#ifndef SERVICE_CONTAINER_HPP_
#define SERVICE_CONTAINER_HPP_

#include <curl/curl.h>

#include <memory>
#include <nlohmann/json.hpp>

/**
 * @brief Service container to manage dependencies in the application.
 *
 * The `ServiceContainer` centralizes the creation and management of shared resources
 * such as the database connection, DAOs, repositories, and services. It ensures
 * that all dependencies are properly initialized and shared across the application.
 */
class ServiceContainer {
public:

private:

};

#endif  // SERVICE_CONTAINER_HPP_
