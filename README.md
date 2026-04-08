# WoodFlow — Smart Carpentry Management

A Qt/C++ desktop application for managing a carpentry workshop, backed by Oracle 11g XE running in Docker.

---

## Prerequisites

| Tool | Purpose |
|------|---------|
| Qt 6 (or Qt 5.15+) | Build the app |
| CMake 3.16+ | Build system |
| Docker | Run the Oracle database |
| Git | Version control |

---

## 1. Start the Database

```bash
# First time — pull and run
docker pull lain456/oracle-cpp-project:v1
docker run -d --name oracle11g -p 1522:1521 -p 8081:8080 lain456/oracle-cpp-project:v1
```

- **Oracle APEX** (web UI): http://localhost:8081/apex
  - Workspace: `INTERNAL` → User: `ADMIN`
- **ODBC/Qt connection**: `localhost:1522`, SID `XE`, user `CPP_PROJECT`, password `Eoseos69`

---

## 2. Build & Run the App

```bash
git clone <repo-url>
cd woodflow

cmake -B build
cmake --build build

# Run
./build/WoodFlow
```

Or use the `qtbuild` / `qtrun` aliases if configured.

---

## 3. Edit the Code

```bash
# Make your changes, then build
cmake --build build

# Run to test
./build/WoodFlow
```


---

## 4. Save DB Changes

Any `ALTER TABLE`, `INSERT`, or data changes made in Oracle APEX are only in the **running container**, not in the image. Save them before stopping:

```bash
docker commit oracle11g lain456/oracle-cpp-project:v1
docker push lain456/oracle-cpp-project:v1
```

---

## 5. Sync Code to GitHub

```bash
git add .
git commit -m "your message"
git push origin employer-wf
```

If the push is rejected due to history conflicts:
```bash
git push origin employer-wf --force-with-lease
```

---


## 6. Restart with Updated Image

```bash
# Stop and remove old container
docker stop oracle11g
docker rm oracle11g

# Pull latest image
docker pull lain456/oracle-cpp-project:v1

# Start fresh
docker run -d --name oracle11g -p 1522:1521 -p 8081:8080 lain456/oracle-cpp-project:v1
```
---

## Docker Hub

Image: **`lain456/oracle-cpp-project:v1`**

Contains Oracle XE 11g with the `CPP_PROJECT` schema, all tables, and sample data.
