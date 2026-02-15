# Atelier Connexion — Qt / Oracle (CMake)

Projet C++ Qt connecté à une base de données Oracle 11g via le driver natif QOCI.  
Implémente le **patron de conception Singleton** pour la gestion de la connexion.

---

## Structure du projet

```
Atelier_Connexion_CMake/
├── CMakeLists.txt              ← Configuration de build (remplace le .pro)
├── connection.h                ← Classe Connection (Singleton)
├── connection.cpp              ← Logique des drivers QOCI / QODBC
├── main.cpp                    ← Point d'entrée
├── mainwindow.h/cpp            ← Formulaire d'ajout d'employé
├── mainwindow.ui               ← Interface Qt
└── oracle-cpp-project.tar      ← (optionnel) image Docker exportée localement
```

---

## Prérequis

| Outil | Version |
|-------|---------|
| Qt | 6.x (avec le module `Qt::Sql`) |
| CMake | 3.16+ |
| GCC | 15+ (ou tout compilateur C++17) |
| Docker | toute version récente |

---

## Étape 1 — Obtenir l'image Docker Oracle

L'image Oracle 11g est disponible publiquement sur Docker Hub :

```bash
docker pull oracleinanutshell/oracle-xe-11g
```

Vérifier qu'elle est bien téléchargée :
```bash
docker images
# doit afficher : oracleinanutshell/oracle-xe-11g
```

> Alternatively, si vous avez le fichier tar exporté localement :
> ```bash
> docker load -i oracle-cpp-project.tar
> ```

---

## Étape 2 — Démarrer le conteneur Oracle

```bash
docker run -d --name oracle11g -p 1521:1521 -p 8080:8080 oracleinanutshell/oracle-xe-11g
```

Attendre ~30 secondes que Oracle démarre complètement, puis vérifier :
```bash
docker ps
```

Pour suivre les logs de démarrage :
```bash
docker logs -f oracle11g
```

Attendre le message `DATABASE IS READY TO USE` avant de continuer.

---

## Étape 3 — Vérifier la base de données

```bash
docker exec -it oracle11g bash
su - oracle
sqlplus CPP_PROJECT/Eoseos69@XE
```

À l'invite SQL :
```sql
SELECT table_name FROM user_tables ORDER BY table_name;
EXIT
```

La table `EMPLOYE` doit apparaître dans la liste.

---

## Étape 4 — Compiler le projet

```bash
cd Atelier_Connexion_CMake

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Ou avec l'alias `qtbuild` :
```bash
qtbuild
```

---

## Étape 5 — Lancer l'application

```bash
./build/Atelier_Connexion
```

Ou :
```bash
qtrun
```

En cas de connexion réussie, le terminal affiche :
```
[Connection] Connected via QOCI (Oracle native).
```

Et la fenêtre **Ajouter un Employé** s'ouvre.

---

## Identifiants de connexion

| Champ | Valeur |
|-------|--------|
| Driver | QOCI (natif Oracle), repli sur QODBC |
| Hôte | localhost |
| Port | 1521 |
| SID | XE |
| Utilisateur | CPP_PROJECT |
| Mot de passe | Eoseos69 |

Les identifiants sont définis en haut de `connection.cpp`.

---

## Schéma de la table EMPLOYE

| Colonne | Type | Remarques |
|---------|------|-----------|
| ID_EMP | NUMBER | Clé primaire (auto) |
| CIN | VARCHAR2(20) | Unique, obligatoire |
| NOM_EMP | VARCHAR2(50) | Obligatoire |
| PRENOM_EMP | VARCHAR2(50) | Obligatoire |
| POST_EMP | VARCHAR2(100) | |
| EMAIL_EMP | VARCHAR2(100) | Unique |
| NUM_TEL | VARCHAR2(20) | |
| DATE_EMBAUCHE | DATE | |
| SALAIRE | NUMBER(10,2) | |
| COMPETENCES | VARCHAR2(500) | |
| DISPO_EMP | VARCHAR2(20) | `Disponible` / `Indisponible` / `En conge` |
| PERFORMANCE | NUMBER(3,2) | Entre 0 et 10 |
| NJC | NUMBER | |
| NJA | NUMBER | |
| HDT | NUMBER(5,2) | |
| PASSWORD_EMP | VARCHAR2(255) | |

---

## Arrêter / Redémarrer le conteneur

```bash
# Arrêter
docker stop oracle11g

# Redémarrer plus tard
docker start oracle11g

# Supprimer le conteneur (l'image reste)
docker rm oracle11g
```

---

## Patron Singleton

La classe `Connection` utilise le **Singleton de Meyer** :

```cpp
// Obtenir l'instance unique depuis n'importe où dans le projet
Connection::createInstance().createconnect();

// Accéder à la connexion ouverte
QSqlDatabase::database(Connection::CONN_NAME);
```

| Condition | Implémentation |
|-----------|----------------|
| Instance unique | Variable statique locale dans `createInstance()` |
| Accès global contrôlé | Méthode statique `createInstance()` |
| Constructeur privé | `Connection() = default` (privé) |
| Copie interdite | `Connection(const&) = delete` |
| Affectation interdite | `operator= = delete` |
