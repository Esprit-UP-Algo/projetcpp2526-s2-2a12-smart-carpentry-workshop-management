-- ============================================================
-- WoodFlow — Full DB Migration Script
-- Oracle 11g XE | User: CPP_PROJECT
-- Run as: CPP_PROJECT user
-- ============================================================

-- ────────────────────────────────────────────────────────────
-- 0. DROP all tables (reverse dependency order)
-- ────────────────────────────────────────────────────────────
BEGIN
    FOR t IN (
        SELECT table_name FROM user_tables
        WHERE table_name IN (
            'COMPOSER','CONTENIR','UTILISER','AFFECTER',
            'COMMANDE','TRANSACTION_FIN','PRODUIT',
            'MATERIAU','EMPLOYE','PROJET'
        )
    ) LOOP
        EXECUTE IMMEDIATE 'DROP TABLE ' || t.table_name || ' CASCADE CONSTRAINTS';
    END LOOP;
END;
/

-- Drop sequences
BEGIN
    FOR s IN (
        SELECT sequence_name FROM user_sequences
        WHERE sequence_name IN (
            'SEQ_EMP','SEQ_PROJET','SEQ_MATERIAU',
            'SEQ_TRANSACTION','SEQ_PRODUIT','SEQ_COMMANDE'
        )
    ) LOOP
        EXECUTE IMMEDIATE 'DROP SEQUENCE ' || s.sequence_name;
    END LOOP;
END;
/

-- ────────────────────────────────────────────────────────────
-- 1. SEQUENCES (auto-increment simulation for Oracle 11g)
-- ────────────────────────────────────────────────────────────
CREATE SEQUENCE SEQ_EMP         START WITH 1 INCREMENT BY 1 NOCACHE NOCYCLE;
CREATE SEQUENCE SEQ_PROJET      START WITH 1 INCREMENT BY 1 NOCACHE NOCYCLE;
CREATE SEQUENCE SEQ_MATERIAU    START WITH 1 INCREMENT BY 1 NOCACHE NOCYCLE;
CREATE SEQUENCE SEQ_TRANSACTION START WITH 1 INCREMENT BY 1 NOCACHE NOCYCLE;
CREATE SEQUENCE SEQ_PRODUIT     START WITH 1 INCREMENT BY 1 NOCACHE NOCYCLE;
CREATE SEQUENCE SEQ_COMMANDE    START WITH 1 INCREMENT BY 1 NOCACHE NOCYCLE;

-- ────────────────────────────────────────────────────────────
-- 2. EMPLOYE
-- ────────────────────────────────────────────────────────────
CREATE TABLE EMPLOYE (
    ID_EMP        NUMBER          NOT NULL,
    CIN           VARCHAR2(20)    NOT NULL,
    NOM_EMP       VARCHAR2(50)    NOT NULL,
    PRENOM_EMP    VARCHAR2(50)    NOT NULL,
    POST_EMP      VARCHAR2(100),
    EMAIL_EMP     VARCHAR2(100),
    NUM_TEL       VARCHAR2(20),
    DATE_EMBAUCHE DATE            DEFAULT SYSDATE,
    SALAIRE       NUMBER(10,2),
    COMPETENCES   VARCHAR2(500),
    DISPO_EMP     VARCHAR2(20),
    PERFORMANCE   NUMBER(3,2),
    NJC           NUMBER          DEFAULT 0,
    NJA           NUMBER          DEFAULT 0,
    HDT           NUMBER(5,2)     DEFAULT 0,
    ROLE          VARCHAR2(50)    DEFAULT 'Employe',
    MOT_DE_PASSE  VARCHAR2(255),
    CONSTRAINT PK_EMPLOYE PRIMARY KEY (ID_EMP),
    CONSTRAINT UQ_EMPLOYE_CIN UNIQUE (CIN)
);

CREATE OR REPLACE TRIGGER TRG_EMPLOYE_ID
    BEFORE INSERT ON EMPLOYE
    FOR EACH ROW
BEGIN
    IF :NEW.ID_EMP IS NULL THEN
        SELECT SEQ_EMP.NEXTVAL INTO :NEW.ID_EMP FROM DUAL;
    END IF;
END;
/

-- ────────────────────────────────────────────────────────────
-- 3. PROJET
-- ────────────────────────────────────────────────────────────
CREATE TABLE PROJET (
    ID_PROJET         NUMBER          NOT NULL,
    NOM_PROJET        VARCHAR2(200)   NOT NULL,
    DATE_DEBUT        DATE,
    DEADLINE          DATE,
    STATUT            VARCHAR2(50)    DEFAULT 'En attente',
    BUDGET            NUMBER(12,2),
    ADRESSE_CHANTIER  VARCHAR2(300),
    CLIENT            VARCHAR2(200),
    TYPE_PROJET       VARCHAR2(100),
    CONSTRAINT PK_PROJET PRIMARY KEY (ID_PROJET)
);

CREATE OR REPLACE TRIGGER TRG_PROJET_ID
    BEFORE INSERT ON PROJET
    FOR EACH ROW
BEGIN
    IF :NEW.ID_PROJET IS NULL THEN
        SELECT SEQ_PROJET.NEXTVAL INTO :NEW.ID_PROJET FROM DUAL;
    END IF;
END;
/

-- ────────────────────────────────────────────────────────────
-- 4. MATERIAU
-- ────────────────────────────────────────────────────────────
CREATE TABLE MATERIAU (
    ID_MAT          NUMBER          NOT NULL,
    NOM_MAT         VARCHAR2(100)   NOT NULL,
    TYPE_MAT        VARCHAR2(100),
    QUANTITE_MAT    NUMBER(10,2)    DEFAULT 0,
    UNITE           VARCHAR2(20),
    PRIX_UNITAIRE   NUMBER(10,2),
    FOURNISSEUR     VARCHAR2(200),
    SEUIL_ALERTE    NUMBER(10,2)    DEFAULT 0,
    LAST_ORDER      DATE,
    CONSO_MENSUELLE NUMBER(10,2)    DEFAULT 0,
    CONSTRAINT PK_MATERIAU PRIMARY KEY (ID_MAT)
);

CREATE OR REPLACE TRIGGER TRG_MATERIAU_ID
    BEFORE INSERT ON MATERIAU
    FOR EACH ROW
BEGIN
    IF :NEW.ID_MAT IS NULL THEN
        SELECT SEQ_MATERIAU.NEXTVAL INTO :NEW.ID_MAT FROM DUAL;
    END IF;
END;
/

-- ────────────────────────────────────────────────────────────
-- 5. TRANSACTION_FIN (renamed to avoid Oracle reserved word)
-- ────────────────────────────────────────────────────────────
CREATE TABLE TRANSACTION_FIN (
    ID_TRAN         NUMBER          NOT NULL,
    MONTANT_TRAN    NUMBER(12,2)    NOT NULL,
    DATE_TRAN       DATE            DEFAULT SYSDATE,
    TYPE_TRAN       VARCHAR2(50),
    MODE_PAIEMENT   VARCHAR2(50),
    STATUT_TRAN     VARCHAR2(50)    DEFAULT 'En attente',
    CATEGORIE_TRAN  VARCHAR2(50),
    DESCRIPTION     VARCHAR2(500),
    CLIENT_TRAN     VARCHAR2(200),
    ID_PROJET       NUMBER,
    CONSTRAINT PK_TRANSACTION PRIMARY KEY (ID_TRAN),
    CONSTRAINT FK_TRAN_PROJET FOREIGN KEY (ID_PROJET)
        REFERENCES PROJET(ID_PROJET) ON DELETE SET NULL
);

CREATE OR REPLACE TRIGGER TRG_TRANSACTION_ID
    BEFORE INSERT ON TRANSACTION_FIN
    FOR EACH ROW
BEGIN
    IF :NEW.ID_TRAN IS NULL THEN
        SELECT SEQ_TRANSACTION.NEXTVAL INTO :NEW.ID_TRAN FROM DUAL;
    END IF;
END;
/

-- ────────────────────────────────────────────────────────────
-- 6. PRODUIT
-- ────────────────────────────────────────────────────────────
CREATE TABLE PRODUIT (
    ID_PROD         NUMBER          NOT NULL,
    NOM_PROD        VARCHAR2(200)   NOT NULL,
    CATEGORIE_PROD  VARCHAR2(100),
    PRIX_PROD       NUMBER(10,2),
    DIMENSIONS      VARCHAR2(100),
    STYLE           VARCHAR2(100),
    DATE_CREATION   DATE            DEFAULT SYSDATE,
    STATUT_PROD     VARCHAR2(50)    DEFAULT 'Actif',
    IMAGE_URL       VARCHAR2(500),
    MODELE_3D_URL   VARCHAR2(500),
    CONSTRAINT PK_PRODUIT PRIMARY KEY (ID_PROD)
);

CREATE OR REPLACE TRIGGER TRG_PRODUIT_ID
    BEFORE INSERT ON PRODUIT
    FOR EACH ROW
BEGIN
    IF :NEW.ID_PROD IS NULL THEN
        SELECT SEQ_PRODUIT.NEXTVAL INTO :NEW.ID_PROD FROM DUAL;
    END IF;
END;
/

-- ────────────────────────────────────────────────────────────
-- 7. COMMANDE
-- ────────────────────────────────────────────────────────────
CREATE TABLE COMMANDE (
    ID_CMD          NUMBER          NOT NULL,
    DATE_CMD        DATE            DEFAULT SYSDATE,
    QUANTITE_CMD    NUMBER(10,2),
    PRIX_TOTAL      NUMBER(12,2),
    STATUT_CMD      VARCHAR2(50)    DEFAULT 'En attente',
    FOURNISSEUR_CMD VARCHAR2(200),
    ID_MAT          NUMBER,
    CONSTRAINT PK_COMMANDE PRIMARY KEY (ID_CMD),
    CONSTRAINT FK_CMD_MAT FOREIGN KEY (ID_MAT)
        REFERENCES MATERIAU(ID_MAT) ON DELETE SET NULL
);

CREATE OR REPLACE TRIGGER TRG_COMMANDE_ID
    BEFORE INSERT ON COMMANDE
    FOR EACH ROW
BEGIN
    IF :NEW.ID_CMD IS NULL THEN
        SELECT SEQ_COMMANDE.NEXTVAL INTO :NEW.ID_CMD FROM DUAL;
    END IF;
END;
/

-- ────────────────────────────────────────────────────────────
-- 8. JUNCTION TABLES (N,M associations)
-- ────────────────────────────────────────────────────────────

-- EMPLOYE <-> PROJET
CREATE TABLE AFFECTER (
    ID_EMP          NUMBER      NOT NULL,
    ID_PROJET       NUMBER      NOT NULL,
    DATE_DEBUT_AFF  DATE,
    DATE_FIN_AFF    DATE,
    ROLE_CHANTIER   VARCHAR2(100),
    CONSTRAINT PK_AFFECTER PRIMARY KEY (ID_EMP, ID_PROJET),
    CONSTRAINT FK_AFF_EMP    FOREIGN KEY (ID_EMP)    REFERENCES EMPLOYE(ID_EMP)  ON DELETE CASCADE,
    CONSTRAINT FK_AFF_PROJET FOREIGN KEY (ID_PROJET) REFERENCES PROJET(ID_PROJET) ON DELETE CASCADE
);

-- PROJET <-> MATERIAU (consumption on site)
CREATE TABLE UTILISER (
    ID_PROJET           NUMBER      NOT NULL,
    ID_MAT              NUMBER      NOT NULL,
    DATE_UTILISATION    DATE        DEFAULT SYSDATE,
    QUANTITE_UTILISEE   NUMBER(10,2),
    CONSTRAINT PK_UTILISER PRIMARY KEY (ID_PROJET, ID_MAT),
    CONSTRAINT FK_UTIL_PROJET FOREIGN KEY (ID_PROJET) REFERENCES PROJET(ID_PROJET)  ON DELETE CASCADE,
    CONSTRAINT FK_UTIL_MAT    FOREIGN KEY (ID_MAT)    REFERENCES MATERIAU(ID_MAT)    ON DELETE RESTRICT
);

-- PROJET <-> PRODUIT (designs in project)
CREATE TABLE CONTENIR (
    ID_PROJET   NUMBER      NOT NULL,
    ID_PROD     NUMBER      NOT NULL,
    QUANTITE    NUMBER(10,2) DEFAULT 1,
    CONSTRAINT PK_CONTENIR PRIMARY KEY (ID_PROJET, ID_PROD),
    CONSTRAINT FK_CONT_PROJET FOREIGN KEY (ID_PROJET) REFERENCES PROJET(ID_PROJET) ON DELETE CASCADE,
    CONSTRAINT FK_CONT_PROD   FOREIGN KEY (ID_PROD)   REFERENCES PRODUIT(ID_PROD)  ON DELETE CASCADE
);

-- PRODUIT <-> MATERIAU (materials needed to make product)
CREATE TABLE COMPOSER (
    ID_PROD             NUMBER      NOT NULL,
    ID_MAT              NUMBER      NOT NULL,
    QUANTITE_NECESSAIRE NUMBER(10,2),
    UNITE               VARCHAR2(20),
    CONSTRAINT PK_COMPOSER PRIMARY KEY (ID_PROD, ID_MAT),
    CONSTRAINT FK_COMP_PROD FOREIGN KEY (ID_PROD) REFERENCES PRODUIT(ID_PROD)  ON DELETE CASCADE,
    CONSTRAINT FK_COMP_MAT  FOREIGN KEY (ID_MAT)  REFERENCES MATERIAU(ID_MAT)  ON DELETE RESTRICT
);

-- ────────────────────────────────────────────────────────────
-- 9. SEED DATA — initial employee for login
-- ────────────────────────────────────────────────────────────
INSERT INTO EMPLOYE (CIN, NOM_EMP, PRENOM_EMP, POST_EMP, EMAIL_EMP, SALAIRE, DISPO_EMP, PERFORMANCE, ROLE, MOT_DE_PASSE)
VALUES ('87654321', 'Mejri', 'Fatima', 'Chef de Projet', 'fatima.mejri@woodflow.tn', 2500, 'Disponible', 8.5, 'Admin', 'test123');

COMMIT;

-- ────────────────────────────────────────────────────────────
-- 10. VERIFY
-- ────────────────────────────────────────────────────────────
SELECT 'EMPLOYE'        AS tbl, COUNT(*) AS rows FROM EMPLOYE        UNION ALL
SELECT 'PROJET'         AS tbl, COUNT(*) AS rows FROM PROJET         UNION ALL
SELECT 'MATERIAU'       AS tbl, COUNT(*) AS rows FROM MATERIAU       UNION ALL
SELECT 'TRANSACTION_FIN'AS tbl, COUNT(*) AS rows FROM TRANSACTION_FIN UNION ALL
SELECT 'PRODUIT'        AS tbl, COUNT(*) AS rows FROM PRODUIT        UNION ALL
SELECT 'COMMANDE'       AS tbl, COUNT(*) AS rows FROM COMMANDE;

SELECT ID_EMP, CIN, NOM_EMP, PRENOM_EMP, POST_EMP, MOT_DE_PASSE FROM EMPLOYE;
