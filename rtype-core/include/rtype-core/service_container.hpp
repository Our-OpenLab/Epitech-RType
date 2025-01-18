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

  /*
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
  */

  static std::optional<std::pair<std::string, std::vector<int>>> CreatePodAndService(const std::string& podName,
                                                                      const std::string& kubeApiUrl,
                                                                      const std::string& token) {
    std::cout << "[ServiceContainer] Starting Pod and Service creation for: " << podName << std::endl;

    // Créer le Pod
    if (!CreateKubernetesResource(podName, kubeApiUrl, token, "pods", CreatePodSpec(podName))) {
      std::cerr << "[ServiceContainer][ERROR] Failed to create Pod: " << podName << std::endl;
      return std::nullopt;
    }

    // Créer le Service
    std::string serviceName = podName + "-service";
    if (!CreateKubernetesResource(serviceName, kubeApiUrl, token, "services", CreateServiceSpec(podName, serviceName))) {
      std::cerr << "[ServiceContainer][ERROR] Failed to create Service: " << serviceName << std::endl;
      return std::nullopt;
    }

    // Tenter de récupérer les informations du service
    for (int attempt = 1; attempt <= 10; ++attempt) {
      auto endpoint = GetServiceExternalEndpoint(serviceName, kubeApiUrl, token);
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
  static bool CreateKubernetesResource(const std::string& resourceName,
                                         const std::string& kubeApiUrl,
                                         const std::string& token,
                                         const std::string& resourceType,
                                         const nlohmann::json& resourceSpec) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "[ServiceContainer][ERROR] Failed to initialize cURL for " << resourceType << ": " << resourceName << std::endl;
            return false;
        }

        std::string url = kubeApiUrl + "/api/v1/namespaces/default/" + resourceType;
        std::string resourceData = resourceSpec.dump();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, resourceData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "/var/run/secrets/kubernetes.io/serviceaccount/ca.crt");

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "[ServiceContainer][ERROR] cURL Error: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

        return true;
    }

    static nlohmann::json CreatePodSpec(const std::string& podName) {
        return {
            {"apiVersion", "v1"},
            {"kind", "Pod"},
            {"metadata", {{"name", podName}, {"namespace", "default"}, {"labels", {{"app", "server"}}}}},
            {"spec", {
                {"serviceAccountName", "pod-creator-sa"},
                {"containers", {{
                    {"name", "server"},
                    {"image", "guillaumemichel1026/rtype-test:latest"},
                    {"command", {"sleep"}},
                    {"args", {"infinity"}},
                    {"ports", {
                        {{"containerPort", 4242}, {"protocol", "TCP"}},
                        {{"containerPort", 4243}, {"protocol", "UDP"}}
                    }}
                }}}
            }}
        };
    }

  static nlohmann::json CreateServiceSpec(const std::string& podName, const std::string& serviceName) {
    int dynamicTcpPort = GetNextAvailablePort();
    int dynamicUdpPort = GetNextAvailablePort();

    return {
          {"apiVersion", "v1"},
          {"kind", "Service"},
          {"metadata", {
              {"name", serviceName},
              {"namespace", "default"}
          }},
          {"spec", {
              {"type", "LoadBalancer"},
              {"externalTrafficPolicy", "Local"},
              {"selector", {{"app", "server"}}},
              {"ports", {
                  {
                    {"name", "tcp-port"},
                    {"protocol", "TCP"},
                    {"port", dynamicTcpPort},
                    {"targetPort", 4242}
                  },
                  {
                      {"name", "udp-port"},
                      {"protocol", "UDP"},
                      {"port", dynamicUdpPort},
                      {"targetPort", 4243}
                  }
              }}
          }}
    };
  }

static std::size_t callback(const char* in, std::size_t size,
                                std::size_t num, std::string* out) {
      const std::size_t totalBytes = size * num;
      out->append(in, totalBytes);
      return totalBytes;
    }

    static std::size_t WriteCallback(const char* in, std::size_t size, std::size_t num, std::string* out) {
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

    // Préparer les en-têtes pour l'autorisation et le type de contenu
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Conteneur pour stocker la réponse
    std::unique_ptr<std::string> httpData(new std::string());

    // Configurer cURL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "/var/run/secrets/kubernetes.io/serviceaccount/ca.crt");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

    // Exécuter la requête
    CURLcode res = curl_easy_perform(curl);

    // Récupérer le code HTTP
    long httpCode(0);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    // Nettoyer les ressources
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Log du résultat cURL
    if (res != CURLE_OK) {
        std::cerr << "[ServiceContainer][ERROR] cURL Error: " << curl_easy_strerror(res) << std::endl;
        return std::nullopt;
    }

    // Log du code HTTP
    std::cout << "[ServiceContainer][DEBUG] HTTP Response Code: " << httpCode << std::endl;

    // Vérifier le code HTTP
    if (httpCode != 200) {
        std::cerr << "[ServiceContainer][ERROR] HTTP request failed with code: " << httpCode << std::endl;
        std::cerr << "[ServiceContainer][DEBUG] Raw Response: " << *httpData << std::endl;
        return std::nullopt;
    }

    // Analyse de la réponse JSON
    try {
        auto response_json = nlohmann::json::parse(*httpData);

        // Log du JSON parsé
        std::cout << "[ServiceContainer][DEBUG] Parsed JSON: " << response_json.dump(4) << std::endl;

        std::string ip;
        std::vector<int> ports;

        // Récupérer l'IP depuis loadBalancer.ingress
        if (response_json.contains("status") &&
            response_json["status"].contains("loadBalancer") &&
            response_json["status"]["loadBalancer"].contains("ingress") &&
            !response_json["status"]["loadBalancer"]["ingress"].empty()) {
            ip = response_json["status"]["loadBalancer"]["ingress"][0]["ip"].get<std::string>();
            std::cout << "[ServiceContainer][DEBUG] LoadBalancer IP: " << ip << std::endl;
        } else {
            std::cerr << "[ServiceContainer][WARN] LoadBalancer ingress IP not found." << std::endl;
        }

        // Récupérer les ports TCP et UDP
        if (response_json.contains("spec") &&
            response_json["spec"].contains("ports") &&
            !response_json["spec"]["ports"].empty()) {
            for (const auto& port : response_json["spec"]["ports"]) {
                if (port.contains("port") && port.contains("protocol")) {
                    int extractedPort = port["port"].get<int>();
                    std::string protocol = port["protocol"].get<std::string>();
                    if (protocol == "TCP" || protocol == "UDP") {
                        ports.push_back(extractedPort);
                        std::cout << "[ServiceContainer][DEBUG] Found " << protocol << " port: " << extractedPort << std::endl;
                    }
                }
            }
        } else {
            std::cerr << "[ServiceContainer][WARN] No ports found in service spec." << std::endl;
        }

        if (!ip.empty() && !ports.empty()) {
            std::cout << "[ServiceContainer][INFO] Resolved Endpoint - IP: " << ip << ", Ports: ";
            for (const auto& p : ports) std::cout << p << " ";
            std::cout << std::endl;

            return std::make_pair(ip, ports);
        }

        std::cerr << "[ServiceContainer][WARN] Failed to retrieve valid IP and Ports." << std::endl;
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
