#include <curl/curl.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>  // Inclure la bibliothèque JSON
#include <string>

// Alias pour simplifier l'utilisation de JSON
using json = nlohmann::json;

// Fonction pour envoyer une requête POST à l'API Kubernetes
bool createPod(const std::string& podName, const std::string& kubeApiUrl, const std::string& token) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Error initializing cURL" << std::endl;
        return false;
    }

    // Construction du JSON pour la création du Pod
    json podJson = {
        {"apiVersion", "v1"},
        {"kind", "Pod"},
        {"metadata", {{"name", podName}}},
        {"spec", {
            {"containers", {{
                {"name", "game-server"},
                {"image", "mygame/server:latest"},
                {"ports", {{{"containerPort", 12345}}}}
            }}}
        }}
    };

    std::string podData = podJson.dump();

    // Configuration de cURL
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, (kubeApiUrl + "/api/v1/namespaces/default/pods").c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, podData.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_CAINFO, "/var/run/secrets/kubernetes.io/serviceaccount/ca.crt");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;
    } else {
        std::cout << "Pod creation request sent for room: " << podName << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res == CURLE_OK;
}

// Fonction principale du serveur
void runServer(const std::string& kubeApiUrl, const std::string& token) {
    std::string command;
    while (true) {
        std::cout << "Enter command (CREATE_ROOM <room_name> or EXIT): ";
        std::getline(std::cin, command);

        // Quitter si la commande est EXIT
        if (command == "EXIT") {
            std::cout << "Shutting down server..." << std::endl;
            break;
        }

        // Vérification de la commande CREATE_ROOM
        if (command.rfind("CREATE_ROOM", 0) == 0) {
            std::string roomName = command.substr(12); // Extraire le nom de la room
            if (roomName.empty()) {
                std::cerr << "Error: Room name cannot be empty!" << std::endl;
                continue;
            }
            std::cout << "Creating room: " << roomName << std::endl;

            // Appeler la fonction pour créer le Pod
            if (createPod(roomName, kubeApiUrl, token)) {
                std::cout << "Room " << roomName << " created successfully!" << std::endl;
            } else {
                std::cerr << "Failed to create room: " << roomName << std::endl;
            }
        } else {
            std::cerr << "Unknown command!" << std::endl;
        }
    }
}

// Point d'entrée du programme
int main() {
    // Exemple d'URL API Kubernetes (à adapter)
    std::string kubeApiUrl = "https://kubernetes.default.svc";

    // Charger le jeton d'accès
    std::string token;
    std::ifstream tokenFile("/var/run/secrets/kubernetes.io/serviceaccount/token");
    if (tokenFile.is_open()) {
        token.assign((std::istreambuf_iterator<char>(tokenFile)),
                      std::istreambuf_iterator<char>());
        tokenFile.close();
    } else {
        std::cerr << "Error: Unable to read Kubernetes token!" << std::endl;
        return 1;
    }

    // Lancer le serveur
    runServer(kubeApiUrl, token);
    return 0;
}
