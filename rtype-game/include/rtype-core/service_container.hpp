#ifndef SERVICE_CONTAINER_HPP_
#define SERVICE_CONTAINER_HPP_

#include <curl/curl.h>

#include <memory>
#include <nlohmann/json.hpp>

#include "database/dao/lobby_dao.hpp"
#include "database/dao/lobby_player_dao.hpp"
#include "database/dao/message_dao.hpp"
#include "database/dao/user_dao.hpp"
#include "database/database.hpp"
#include "domain/repositories/lobby_player_repository.hpp"
#include "domain/repositories/lobby_repository.hpp"
#include "domain/repositories/message_repository.hpp"
#include "domain/repositories/user_repository.hpp"
#include "domain/services/lobby_player_service.hpp"
#include "domain/services/lobby_service.hpp"
#include "domain/services/message_service.hpp"
#include "domain/services/user_service.hpp"

/**
 * @brief Service container to manage dependencies in the application.
 *
 * The `ServiceContainer` centralizes the creation and management of shared resources
 * such as the database connection, DAOs, repositories, and services. It ensures
 * that all dependencies are properly initialized and shared across the application.
 */
class ServiceContainer {
public:
  /**
   * @brief Constructs the service container and initializes services.
   *
   * @param db_connection_string The connection string for the PostgreSQL database.
   */
  explicit ServiceContainer(const std::string& db_connection_string)
      : database_(std::make_shared<Database>(db_connection_string)) {
    InitializeServices();
  }

  /**
   * @brief Retrieves the `UserService` instance.
   *
   * @return A shared pointer to the `UserService`.
   */
  [[nodiscard]] std::shared_ptr<UserService> GetUserService() const {
    return user_service_;
  }

  /**
   * @brief Retrieves the `MessageService` instance.
   *
   * @return A shared pointer to the `MessageService`.
   */
  [[nodiscard]] std::shared_ptr<MessageService> GetMessageService() const {
    return message_service_;
  }

  /**
   * @brief Retrieves the `LobbyService` instance.
   *
   * @return A shared pointer to the `LobbyPlayerService`.
   */
  [[nodiscard]] std::shared_ptr<LobbyService> GetLobbyService() const {
    return lobby_service_;
  }

  /**
   * @brief Retrieves the `LobbyPlayerService` instance.
   *
   * @return A shared pointer to the `LobbyPlayerService`.
   */
  [[nodiscard]] std::shared_ptr<LobbyPlayerService> GetLobbyPlayerService()
      const {
    return lobby_player_service_;
  }

  static bool CreatePod(const std::string& podName, const std::string& kubeApiUrl, const std::string& token) {
    using json = nlohmann::json;

    CURL* curl = curl_easy_init();
    if (!curl) {
      std::cerr << "[MainServer][ERROR] Error initializing cURL" << std::endl;
      return false;
    }

    // JSON payload for the Pod
    const json podJson = {{"apiVersion", "v1"},
                    {"kind", "Pod"},
                    {"metadata", {{"name", podName}}},
                    {"spec",
                     {{"containers",
                       {{{"name", "game-server"},
                         {"image", "mygame/server:latest"},
                         {"ports", {{{"containerPort", 12345}}}}}}}}}};

    const std::string podData = podJson.dump();

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, (kubeApiUrl + "/api/v1/namespaces/default/pods").c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, podData.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "/var/run/secrets/kubernetes.io/serviceaccount/ca.crt");

    const CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      std::cerr << "[MainServer][ERROR] cURL Error: " << curl_easy_strerror(res) << std::endl;
    } else {
      std::cout << "[MainServer] Pod creation request sent for room: " << podName << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res == CURLE_OK;
  }

private:
  /**
   * @brief Initializes all services and their dependencies.
   *
   * Creates DAOs, repositories, and services, wiring them together
   * to form the application's dependency graph.
   */
  void InitializeServices() {
    auto user_dao = std::make_shared<UserDAO>(database_);
    auto user_repository = std::make_shared<UserRepository>(user_dao);

    user_service_ = std::make_shared<UserService>(user_repository);

    auto message_dao = std::make_shared<MessageDAO>(database_);
    auto message_repository = std::make_shared<MessageRepository>(message_dao);

    message_service_ = std::make_shared<MessageService>(message_repository);

    auto lobby_dao = std::make_shared<LobbyDAO>();
    auto lobby_repository = std::make_shared<LobbyRepository>(lobby_dao);

    lobby_service_ = std::make_shared<LobbyService>(lobby_repository);

    auto lobby_player_dao = std::make_shared<LobbyPlayerDAO>();
    auto lobby_player_repository = std::make_shared<LobbyPlayerRepository>(lobby_player_dao);

    lobby_player_service_ = std::make_shared<LobbyPlayerService>(lobby_player_repository);
  }

  std::shared_ptr<Database> database_;            ///< Shared database connection.
  std::shared_ptr<UserService> user_service_;     ///< Shared UserService instance.
  std::shared_ptr<MessageService> message_service_;     ///< Shared MessageService instance.
  std::shared_ptr<LobbyService> lobby_service_;    ///< Shared LobbyService instance.
  std::shared_ptr<LobbyPlayerService> lobby_player_service_;    ///< Shared LobbyPlayerService instance.
};

#endif  // SERVICE_CONTAINER_HPP_
