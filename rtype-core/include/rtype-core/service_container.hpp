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

  static std::optional<std::pair<std::string, std::vector<int>>> CreatePodAndService(const std::string& podName,
                                                                      const std::string& kubeApiUrl,
                                                                      const std::string& token) {
    std::cout << "[ServiceContainer] Starting Pod and Service creation for: "
              << podName << std::endl;

    const int dynamicTcpPort = GetNextAvailablePort();
    const int dynamicUdpPort = GetNextAvailablePort();

    if (!CreateKubernetesResource(kubeApiUrl, token, "pods",
            CreatePodSpec(podName, dynamicTcpPort, dynamicUdpPort))) {
      std::cerr << "[ServiceContainer][ERROR] Failed to create Pod: " << podName
                << std::endl;
      return std::nullopt;
    }

    const std::string serviceName = podName + "-service";
    if (!CreateKubernetesResource(kubeApiUrl, token, "services",
                                   CreateServiceSpec(podName, dynamicTcpPort, dynamicUdpPort))) {
      std::cerr << "[ServiceContainer][ERROR] Failed to create Service: " << serviceName << std::endl;
      return std::nullopt;
                                   }

    for (int attempt = 1; attempt <= 10; ++attempt) {
      const auto endpoint = GetServiceExternalEndpoint(serviceName, kubeApiUrl, token);
      if (endpoint) {
        std::cout << "External IP/ClusterIP: " << endpoint->first << std::endl;
        std::cout << "Ports: ";
        for (const auto& port : endpoint->second) {
          std::cout << port << " ";
        }
        std::cout << std::endl;
        return endpoint;
      }
      std::cerr << "Failed to retrieve service endpoint or ports." << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    std::cerr << "[ServiceContainer][ERROR] Service did not become ready in time: " << serviceName << std::endl;
    return std::nullopt;
  }


private:
  static bool CreateKubernetesResource(const std::string& kubeApiUrl,
                                         const std::string& token,
                                         const std::string& resourceType,
                                         const nlohmann::json& resourceSpec) {
        CURL* curl = curl_easy_init();
        if (!curl) {
          std::cerr
              << "[ServiceContainer][ERROR] Failed to initialize cURL" << std::endl;
          return false;
        }

        const std::string url =
            kubeApiUrl + "/api/v1/namespaces/default/" + resourceType;
        const std::string resourceData = resourceSpec.dump();

        curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, resourceData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "/var/run/secrets/kubernetes.io/serviceaccount/ca.crt");

        const CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "[ServiceContainer][ERROR] cURL Error: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

        return true;
    }

  static nlohmann::json CreatePodSpec(const std::string& podName, int tcpPort, int udpPort) {
    return {
          {"apiVersion", "v1"},
          {"kind", "Pod"},
          {"metadata", {
              {"name", podName},
              {"namespace", "default"},
              {"labels", {{"app", "server"}, {"instance", podName}}}
          }},
          {"spec", {
              {"serviceAccountName", "pod-creator-sa"},
              {"containers", {{
                {"name", "server"},
                {"image", "guillaumemichel1026/rtype-dev:latest"},
                {"command", {"/app/build/Release/bin/RTypeGame"}},
                {"args", {std::to_string(tcpPort), std::to_string(udpPort)}},
                {"ports", {
                      {{"containerPort", tcpPort}, {"protocol", "TCP"}},
                      {{"containerPort", udpPort}, {"protocol", "UDP"}}
                }}
              }}}
          }}
    };
  }

  static nlohmann::json CreateServiceSpec(const std::string& podName, int tcpPort, int udpPort) {
    return {
          {"apiVersion", "v1"},
          {"kind", "Service"},
          {"metadata", {
              {"name", podName + "-service"},
              {"namespace", "default"}
          }},
          {"spec", {
              {"type", "LoadBalancer"},
              {"externalTrafficPolicy", "Local"},
              {"selector", {{"app", "server"}, {"instance", podName}}},
              {"ports", {
                  {
                    {"name", "tcp-port"},
                    {"protocol", "TCP"},
                    {"port", tcpPort},
                    {"targetPort", tcpPort}
                  },
                  {
                      {"name", "udp-port"},
                      {"protocol", "UDP"},
                      {"port", udpPort},
                      {"targetPort", udpPort}
                  }
              }}
          }}
    };
  }

static std::size_t callback(const char* in, const std::size_t size,
                              const std::size_t num, std::string* out) {
      const std::size_t totalBytes = size * num;
      out->append(in, totalBytes);
      return totalBytes;
    }

    static std::size_t WriteCallback(const char* in, const std::size_t size,
                                     const std::size_t num, std::string* out) {
        const std::size_t totalBytes = size * num;
        out->append(in, totalBytes);
        return totalBytes;
    }

static std::optional<std::pair<std::string, std::vector<int>>> GetServiceExternalEndpoint(
    const std::string& serviceName,
    const std::string& kubeApiUrl,
    const std::string& token) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[ServiceContainer][ERROR] Failed to initialize cURL for service: " << serviceName << std::endl;
        return std::nullopt;
    }

    const std::string url = kubeApiUrl + "/api/v1/namespaces/default/services/" + serviceName;

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    const std::unique_ptr<std::string> httpData(new std::string());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "/var/run/secrets/kubernetes.io/serviceaccount/ca.crt");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

    const CURLcode res = curl_easy_perform(curl);

    long httpCode(0);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "[ServiceContainer][ERROR] cURL Error: " << curl_easy_strerror(res) << std::endl;
        return std::nullopt;
    }

    std::cout << "[ServiceContainer][DEBUG] HTTP Response Code: " << httpCode << std::endl;

    if (httpCode != 200) {
        std::cerr << "[ServiceContainer][ERROR] HTTP request failed with code: " << httpCode << std::endl;
        std::cerr << "[ServiceContainer][DEBUG] Raw Response: " << *httpData << std::endl;
        return std::nullopt;
    }

    try {
        auto response_json = nlohmann::json::parse(*httpData);

        std::cout << "[ServiceContainer][DEBUG] Parsed JSON: " << response_json.dump(4) << std::endl;

        std::string ip;
        std::vector<int> ports;

        if (response_json.contains("status") &&
          response_json["status"].contains("loadBalancer") &&
          response_json["status"]["loadBalancer"].contains("ingress") &&
          !response_json["status"]["loadBalancer"]["ingress"].empty() &&
          response_json["status"]["loadBalancer"]["ingress"][0].contains("ip")) {
          ip = response_json["status"]["loadBalancer"]["ingress"][0]["ip"].get<std::string>();
          std::cout << "[ServiceContainer][DEBUG] LoadBalancer IP: " << ip << std::endl;
        } else {
          std::cerr << "[ServiceContainer][INFO] No LoadBalancer ingress IP available." << std::endl;
        }


        if (response_json.contains("spec") &&
            response_json["spec"].contains("ports") &&
            !response_json["spec"]["ports"].empty()) {
            for (const auto& port : response_json["spec"]["ports"]) {
                if (port.contains("port") && port.contains("protocol")) {
                  int extractedPort = port["port"].get<int>();
                  const auto protocol = port["protocol"].get<std::string>();
                    if (protocol == "TCP" || protocol == "UDP") {
                        ports.push_back(extractedPort);
                        std::cout << "[ServiceContainer][DEBUG] Found " << protocol << " port: " << extractedPort << std::endl;
                    }
                }
            }
        } else {
            std::cerr << "[ServiceContainer][WARN] No ports found in service spec." << std::endl;
        }

        //if (!ip.empty() && !ports.empty()) {
        //    std::cout << "[ServiceContainer][INFO] Resolved Endpoint - IP: " << ip << ", Ports: ";
        //    for (const auto& p : ports) std::cout << p << " ";
        //    std::cout << std::endl;

        //    return std::make_pair(ip, ports);
        //}

        //std::cerr << "[ServiceContainer][WARN] Failed to retrieve valid IP and Ports." << std::endl;

        if (!ports.empty()) {
          std::cout << "[ServiceContainer][INFO] Resolved Ports: ";
          for (const auto& p : ports) std::cout << p << " ";
          std::cout << std::endl;
        }

        return std::make_pair(ip, ports);
    } catch (const std::exception& e) {
        std::cerr << "[ServiceContainer][ERROR] Parsing error: " << e.what() << std::endl;
    }

    return std::nullopt;
}

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

  static int GetNextAvailablePort() {
    if (currentPort < PORT_START || currentPort > PORT_END) {
      currentPort = PORT_START;
    }
    return currentPort++;
  }

  static int currentPort;
  static constexpr int PORT_START = 30000;
  static constexpr int PORT_END = 60000;

  std::shared_ptr<Database> database_;            ///< Shared database connection.
  std::shared_ptr<UserService> user_service_;     ///< Shared UserService instance.
  std::shared_ptr<MessageService> message_service_;     ///< Shared MessageService instance.
  std::shared_ptr<LobbyService> lobby_service_;    ///< Shared LobbyService instance.
  std::shared_ptr<LobbyPlayerService> lobby_player_service_;    ///< Shared LobbyPlayerService instance.
};

int ServiceContainer::currentPort = PORT_START;

#endif  // SERVICE_CONTAINER_HPP_
