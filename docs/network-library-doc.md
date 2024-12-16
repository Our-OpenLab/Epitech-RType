# Documentation de la bibliothèque Network

La bibliothèque **Network** fournit une abstraction simple et performante pour gérer des connexions réseau asynchrones en utilisant `ASIO`. Elle est conçue pour traiter des paquets typés et gérer la communication de manière asynchrone avec une file d'attente concurrente.

---

## Table des matières

- [Introduction](#introduction)
- [Classes principales](#classes-principales)
  - [ClientConnection](#clientconnection)
  - [ConcurrentQueue](#concurrentqueue)
  - [Connection](#connection)
  - [ConnectionInterface](#connectioninterface)
- [Structures auxiliaires](#structures-auxiliaires)
- [Fonctionnalités principales](#fonctionnalités-principales)
- [Exemple d'utilisation](#exemple-dutilisation)

---

## Introduction

La bibliothèque Network repose sur l'utilisation d'`ASIO` pour gérer des connexions TCP/IP asynchrones. Elle permet :
- La gestion de l'envoi et de la réception de paquets typés.
- Une interface facile à utiliser pour gérer les connexions client-serveur.
- Une file d'attente thread-safe pour stocker et manipuler les données reçues.

---

## Classes principales

### ClientConnection

`ClientConnection` gère les connexions réseau côté client. Elle hérite de `Connection` et fournit des fonctionnalités spécifiques pour établir une connexion avec un serveur et lire/écrire des paquets.

#### Méthodes

- **`connect(endpoints, connection_result)`**
  - **Description :** Initialise une connexion avec un serveur via une liste d'endpoints.
  - **Paramètres :**
    - `endpoints` : Résultats de résolution des endpoints (type `asio::ip::tcp::resolver::results_type`).
    - `connection_result` : Une promesse indiquant si la connexion a réussi ou échoué.
  
- **`read_header()`** *(Override)* :
  - Lit l'en-tête d'un paquet reçu.

- **`read_body()`** *(Override)* :
  - Lit le corps du paquet en fonction de la taille spécifiée dans l'en-tête.

- **`write_packet()`** *(Override)* :
  - Envoie un paquet à partir de la file d'attente d'envoi.

- **`handle_error(context, ec)`** *(Privée)* :
  - Gère les erreurs spécifiques à une opération réseau donnée.

#### Membres privés
- `received_queue_` : File d'attente des paquets reçus, partagée avec d'autres composants du système.

---

### ConcurrentQueue

Une classe utilitaire thread-safe pour gérer des files d'attente concurrentes.

#### Méthodes

- **`push(const T& item)`**
  - Insère un élément dans la file.
- **`push(T&& item)`**
  - Insère un élément déplacé dans la file.
- **`pop()`**
  - Récupère et supprime le premier élément de la file.
  - **Retourne :** `std::optional<T>` contenant l'élément, ou `std::nullopt` si la file est vide.
- **`try_pop(T& value)`**
  - Tente de récupérer un élément sans bloquer.
  - **Retourne :** `true` si un élément a été récupéré, `false` sinon.
- **`front()`**
  - Accède au premier élément sans le supprimer.
- **`size()`**
  - Renvoie la taille actuelle de la file.
- **`clear()`**
  - Vide complètement la file.
- **`empty()`**
  - Vérifie si la file est vide.

#### Membres privés

- `std::queue<T> queue_` : La file sous-jacente.
- `std::mutex mutex_` : Mutex pour protéger l'accès concurrent.

---

### Connection

`Connection` est une classe abstraite définissant une interface commune pour les connexions réseau asynchrones. Elle inclut des méthodes pour envoyer des paquets et gérer la communication.

#### Méthodes

- **`send(const Packet<PacketType>& packet)`**
  - Envoie un paquet via la connexion.
- **`disconnect()`**
  - Ferme proprement la connexion.
- **`is_connected()`**
  - Indique si la connexion est active.

#### Membres protégés
- `io_context_` : Contexte d'exécution ASIO.
- `socket_` : Socket TCP associé à la connexion.
- `send_queue_` : File d'attente des paquets à envoyer.
- `incoming_packet_` : Paquet en cours de réception.

#### Constantes

- `MAX_BODY_SIZE` : Taille maximale d'un corps de paquet (1 Mo par défaut).

---

### ConnectionInterface

Interface abstraite pour définir les méthodes nécessaires à toute connexion réseau.

#### Méthodes

- **`is_connected()`** *(Virtuelle pure)* :
  - Renvoie `true` si la connexion est active.
- **`disconnect()`** *(Virtuelle pure)* :
  - Déconnecte la connexion.
- **`send(const Packet<PacketType>& packet)`** *(Virtuelle pure)* :
  - Envoie un paquet par la connexion.

---

## Structures auxiliaires

- **`Packet<PacketType>`**
  - Structure générique représentant un paquet, incluant un en-tête et un corps.

---

## Fonctionnalités principales

- **Gestion des paquets :**
  - Support pour les en-têtes et corps personnalisés avec vérification de la taille.
- **Asynchronisme :**
  - Lecture et écriture asynchrones des données réseau.
- **Fiabilité :**
  - Gestion des erreurs avec déconnexion propre en cas de problème.
- **Concurrence :**
  - Utilisation de `ConcurrentQueue` pour gérer les paquets reçus de manière thread-safe.

---

## Exemple d'utilisation

### Exemple de connexion client

```cpp
#include "client_connection.hpp"

int main() {
    asio::io_context io_context;
    asio::ip::tcp::socket socket(io_context);
    network::ConcurrentQueue<network::Packet<int>> received_queue;

    auto client = std::make_shared<network::ClientConnection<int>>(io_context, std::move(socket), received_queue);

    asio::ip::tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve("localhost", "8080");

    std::promise<bool> connection_result;
    client->connect(endpoints, connection_result);

    if (connection_result.get_future().get()) {
        std::cout << "Connexion réussie !" << std::endl;
    } else {
        std::cerr << "Échec de la connexion." << std::endl;
    }

    io_context.run();
    return 0;
}
