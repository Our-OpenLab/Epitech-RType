# Epitech-RType

A network video game in C++

## Introduction

This video game is inspired by R-type, a shoot them up game released in 1987 on arcade terminal. It was developed during the 3rd year of 5 students from Epitech Nancy

## Install

### Prerequisites
- [CMake](https://cmake.org/)
- [Conan](https://conan.io/)
- C++ compiler (g++, clang...)

### [Instalation gide](instalation.md)

## Documentation

[Link of the GitHub Pages](link)

## Credit

### Developed by:
- [Guillaume Michel](https://github.com/michelguillaume)
- [Pierre Connes](https://github.com/Scorpierre)
- [Abderrhamane Fajli](https://github.com/AbderF)
- [Victorien Conde](https://github.com/VictorienConde)
- [Arthur Aubry](https://github.com/arthur489)


Le type d'architecture que tu utilises est une variante de **l'architecture hexagonale** (ou architecture en couches). Cette approche est largement utilisée dans les projets modulaires pour séparer les préoccupations et permettre une meilleure évolutivité, maintenabilité et testabilité.

Voici une explication des rôles de chaque couche et leur relation dans ton projet :

---

### **1. DAO (Data Access Object)**
#### **Rôle :**
Le DAO est une couche qui encapsule la logique d'accès direct à la base de données. Il fournit une interface simplifiée pour effectuer des opérations CRUD sur les tables de la base de données. Cela permet de centraliser et d'abstraire les interactions SQL.

#### **Pourquoi ?**
- Réduire la duplication du code SQL dans l'ensemble de l'application.
- Faciliter la gestion des exceptions spécifiques à la base de données.
- Découpler les couches supérieures des détails de la base de données.

#### **Position dans l'architecture :**
- **Couche la plus proche de la base de données.**
- **Dépend uniquement de la couche infrastructurelle (base de données).**

---

### **2. Entities**
#### **Rôle :**
Les "entities" (entités) représentent les objets métier fondamentaux de ton domaine. Elles encapsulent les données et les règles métier associées. Par exemple, dans ton cas, un `User` représente un utilisateur avec un ID, un nom d'utilisateur et un mot de passe haché.

#### **Pourquoi ?**
- Centraliser les structures de données utilisées dans l'application.
- Fournir des représentations cohérentes des objets métier dans toutes les couches.
- Permettre de réutiliser ces entités dans les différents cas d'utilisation.

#### **Position dans l'architecture :**
- **Couche du domaine métier.**
- **Indépendante des couches inférieures comme la base de données ou les services externes.**

---

### **3. Repositories**
#### **Rôle :**
Les repositories agissent comme des médiateurs entre les entités du domaine et la couche d'accès aux données (DAO). Ils offrent une abstraction plus élevée qui mappe les données brutes de la base de données vers des entités métier.

#### **Pourquoi ?**
- Encapsuler la logique de transformation entre les entités et les données brutes.
- Fournir une interface pour interagir avec les entités sans exposer les détails des DAO ou de la base de données.
- Faciliter la substitution (mocking) pour les tests.

#### **Position dans l'architecture :**
- **Couche intermédiaire entre la base de données et la logique métier.**
- **Dépend des DAO, mais pas des services.**

---

### **4. Services**
#### **Rôle :**
Les services contiennent la logique métier de haut niveau. Ils orchestrent les appels aux repositories et implémentent les règles métier complexes. Par exemple, vérifier si un utilisateur existe avant de l'enregistrer ou valider un mot de passe lors d'une authentification.

#### **Pourquoi ?**
- Centraliser la logique métier en un seul endroit, indépendamment des détails de stockage ou d'interface utilisateur.
- Fournir des méthodes réutilisables pour répondre aux besoins de l'application (enregistrement, authentification, etc.).
- Faciliter l'évolution des règles métier sans modifier les couches inférieures.

#### **Position dans l'architecture :**
- **Couche logique métier de haut niveau.**
- **Dépend des repositories pour interagir avec les données.**

---

### **Type d'architecture :**
Ton projet adopte une **architecture en couches** (layered architecture), inspirée de l'**architecture hexagonale** :

#### **Description :**
- Les couches sont strictement séparées, chaque couche ayant une responsabilité spécifique.
- Les dépendances vont uniquement du haut vers le bas : les services dépendent des repositories, qui dépendent des DAO, qui dépendent de la base de données.
- Cela permet de remplacer une couche sans impacter les autres (ex. changer de base de données ou de technologie DAO sans modifier les services).

#### **Diagramme simplifié :**
```
       [ Services ]
           |
       [ Repositories ]
           |
       [ DAO ]
           |
  [ Base de données ]
```

---

### **Avantages de cette architecture :**
1. **Séparation des préoccupations :**
   Chaque couche a une responsabilité claire, ce qui rend le code plus lisible et maintenable.

2. **Facilité de test :**
   Tu peux tester chaque couche indépendamment. Par exemple, tu peux simuler un Repository pour tester les Services sans avoir besoin de la base de données.

3. **Extensibilité :**
   Tu peux facilement ajouter de nouvelles fonctionnalités en modifiant ou en ajoutant des couches spécifiques.

4. **Portabilité :**
   La logique métier (Services) et les entités (Entities) sont indépendantes de la base de données ou des frameworks sous-jacents, ce qui facilite le portage vers d'autres systèmes.

---

### **Conclusion :**
Cette architecture est robuste, flexible et largement adoptée dans les grands projets. Elle permet une évolution fluide de ton application et une adaptation aux nouvelles exigences sans refactorisations coûteuses.


La **méthode universelle** pour accéder à un conteneur Docker depuis le navigateur ou d'autres services, quelle que soit la plateforme (macOS, Linux, Windows), consiste à éviter de s'appuyer sur des particularités spécifiques à l'OS ou au réseau Docker. Voici comment mettre en place une configuration robuste et indépendante :

---

### **Méthode Universelle : Utilisation de Ports Publis avec Réseau Bridge**
La configuration par défaut de Docker (réseau `bridge`) et la publication explicite de ports fonctionnent sur toutes les plateformes.

#### **1. Lancer le conteneur avec un port publié**
Au lieu d'utiliser le mode réseau `host` (qui n'est pas pris en charge sur macOS ou Windows), vous publiez explicitement un port local avec `-p`. Par exemple :

```bash
docker run -d \
  -e 'PGADMIN_DEFAULT_EMAIL=admin@example.com' \
  -e 'PGADMIN_DEFAULT_PASSWORD=admin' \
  -p 8080:80 \
  dpage/pgadmin4
```

Dans cet exemple :
- `-p 8080:80` redirige le port 80 du conteneur vers le port 8080 sur votre machine locale.
- Vous pouvez accéder à PgAdmin via `http://localhost:8080`.

---

#### **2. Vérifier les conteneurs en cours d'exécution**
Utilisez `docker ps` pour vous assurer que votre conteneur PgAdmin est en cours d’exécution :

```bash
docker ps
```

Cela devrait montrer une ligne similaire :
```
CONTAINER ID   IMAGE           COMMAND                  PORTS                   NAMES
abcd1234       dpage/pgadmin4  "/entrypoint.sh"         0.0.0.0:8080->80/tcp    pgadmin
```

Assurez-vous que le port `8080` est mappé comme attendu.

---

#### **3. Tester la connectivité**
- Ouvrez un navigateur et allez sur `http://localhost:8080`.
- Vous devriez voir l'interface de connexion de PgAdmin.

---

#### **4. Connecter PgAdmin à votre base PostgreSQL**
Lors de l'ajout de votre base dans PgAdmin, utilisez ces informations (par exemple pour une base sur Kubernetes avec port-forwarding) :
- **Nom du serveur** : Ce que vous voulez (ex. `Postgres`)
- **Host/Adresse** : `host.docker.internal` (macOS/Windows) ou `localhost` (Linux).
- **Port** : 5432 (ou celui que vous avez configuré pour PostgreSQL).
- **Username** : `postgres`
- **Password** : Le mot de passe PostgreSQL que vous avez défini.

---

### **Pourquoi cette méthode est universelle ?**
1. **Compatibilité multiplateforme :** Pas de dépendance au mode réseau `host` ou à des configurations spécifiques.
2. **Simples redirections de ports :** Fonctionne avec tous les conteneurs Docker.
3. **Facilité de déploiement :** Pas besoin de modifier les règles réseau ou de dépendre de configurations spécifiques à Docker Desktop.

En suivant cette méthode, vous pouvez accéder à PgAdmin (ou d'autres services Docker) de manière fiable sur toutes les plateformes.



Oui, vous pouvez utiliser l'image Docker officielle `dpage/pgadmin4` pour exécuter pgAdmin dans un conteneur. Voici comment procéder pour installer et configurer `dpage/pgadmin4` afin de vous connecter à votre base de données PostgreSQL exposée via Kubernetes.

---

### **Étapes pour utiliser `dpage/pgadmin4` avec Docker**

#### 1. **Télécharger et exécuter l'image Docker**
Lancez un conteneur Docker avec `dpage/pgadmin4`. Vous pouvez personnaliser les variables d'environnement pour définir vos identifiants d'accès à pgAdmin.

```bash
docker run -d \
  --name pgadmin \
  -e 'PGADMIN_DEFAULT_EMAIL=admin@example.com' \
  -e 'PGADMIN_DEFAULT_PASSWORD=admin' \
  -p 8080:80 \
  dpage/pgadmin4
```

- **`PGADMIN_DEFAULT_EMAIL`** : L'adresse e-mail pour vous connecter à l'interface pgAdmin.
- **`PGADMIN_DEFAULT_PASSWORD`** : Le mot de passe pour vous connecter.

Cela exposera l'interface pgAdmin sur [http://localhost:8080](http://localhost:8080).

---

#### 2. **Exposer PostgreSQL localement via `kubectl port-forward`**
Exposez votre base de données PostgreSQL sur un port local pour permettre à pgAdmin de s’y connecter :

```bash
kubectl port-forward svc/my-postgres-postgresql 5432:5432
```

Cela rendra votre base PostgreSQL accessible via `localhost:5432`.

---

#### 3. **Accéder à pgAdmin**
Ouvrez [http://localhost:8080](http://localhost:8080) dans votre navigateur et connectez-vous avec les identifiants définis (e.g., `admin@example.com` et `admin`).

---

#### 4. **Ajouter la base de données dans pgAdmin**
1. Dans l'interface pgAdmin :
   - Cliquez sur **"Add New Server"**.
   - Dans l'onglet **General**, donnez un nom à votre connexion (e.g., `My PostgreSQL`).

2. Dans l'onglet **Connection** :
   - **Host name/address** : `localhost`
   - **Port** : `5432`
   - **Maintenance database** : `postgres`
   - **Username** : `postgres` (ou votre utilisateur PostgreSQL).
   - **Password** : Le mot de passe PostgreSQL que vous avez défini dans le secret Kubernetes.

---

#### 5. **Tester la connexion**
Une fois la connexion configurée, cliquez sur **Save**. Si tout est correctement configuré, vous devriez voir votre base de données PostgreSQL listée dans l'arborescence.

---

### **Option sans exposer PostgreSQL avec `port-forward`**
Si vous souhaitez connecter pgAdmin directement au service PostgreSQL dans Kubernetes :
1. Assurez-vous que votre PostgreSQL est accessible via un **LoadBalancer** ou un **NodePort**.
2. Configurez pgAdmin avec l'adresse IP ou le nom DNS du service PostgreSQL, par exemple :
   - **Host name/address** : `my-postgres-postgresql.default.svc.cluster.local`
   - **Port** : `5432`

---

### **Avantages d'utiliser pgAdmin avec Docker**
- Pas besoin d'installer pgAdmin localement.
- Facilité de configuration et suppression.
- Interface web accessible via navigateur.

C'est une solution flexible et rapide pour visualiser et gérer vos données PostgreSQL !



L'erreur indique que l'exécution des scripts PowerShell est désactivée sur votre système en raison des **politiques d'exécution**. Pour résoudre ce problème, vous devez modifier la politique d'exécution pour permettre l'exécution de scripts.

Voici les étapes pour le faire :

---

### **Étape 1 : Vérifiez la politique d'exécution actuelle**
1. Ouvrez PowerShell **en tant qu'administrateur**.
2. Exécutez la commande suivante :
   ```powershell
   Get-ExecutionPolicy
   ```
   Cette commande retourne la politique actuelle (exemple : `Restricted`).

---

### **Étape 2 : Modifier la politique d'exécution**
Si la politique est définie sur `Restricted`, vous devez la changer. Pour permettre l'exécution de scripts non signés dans votre session, utilisez cette commande :

```powershell
Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned
```

- **RemoteSigned** : Permet les scripts locaux non signés, mais exige que les scripts téléchargés soient signés.
- **-Scope CurrentUser** : Change la politique uniquement pour l'utilisateur actuel (pas besoin d'accès administrateur).

Si vous avez besoin d'un accès administrateur pour toutes les sessions, utilisez cette commande :

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned
```

---

### **Étape 3 : Exécutez votre script**
Essayez de relancer votre script :

```powershell
.\install_windows.ps1
```

---

### **Étape 4 : Revenir à la politique précédente (optionnel)**
Si vous préférez remettre la politique par défaut après exécution du script, exécutez :

```powershell
Set-ExecutionPolicy Restricted
```

---

### **Attention :**
Soyez prudent lors de la modification des politiques d'exécution. Laisser les politiques sur `Unrestricted` peut exposer votre système à des risques de sécurité.

Si vous avez des questions supplémentaires, faites-le-moi savoir !
