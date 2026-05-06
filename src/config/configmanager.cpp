#include "configmanager.h"
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

ConfigManager& ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager()
    : m_temperature(0.7)
    , m_maxTokens(1000)
    , m_historySize(50)
    , m_isValid(false)
{
    // Le fichier config.ini sera dans le MÊME DOSSIER que l'exécutable
    // Cela fonctionne sur n'importe quelle machine !
    QString appPath = QCoreApplication::applicationDirPath();
    m_configPath = appPath + "/config.ini";
    
    qDebug() << "Looking for config at:" << m_configPath;
    
    if (QFile::exists(m_configPath)) {
        qDebug() << "Config file found!";
        loadConfig();
    } else {
        qDebug() << "Config file NOT found, creating default...";
        createDefaultConfigFile();
        loadDefaults();
    }
}

void ConfigManager::createDefaultConfigFile()
{
    QFile file(m_configPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "# WoodFlow Configuration File\n";
        stream << "# Get your API key from: https://openrouter.ai/keys\n\n";
        stream << "[OpenRouter]\n";
        stream << "api_key = \n";
        stream << "model = openai/gpt-3.5-turbo\n";
        stream << "temperature = 0.7\n";
        stream << "max_tokens = 1000\n\n";
        stream << "[Chatbot]\n";
        stream << "system_prompt = Vous êtes WoodFlow Assistant, un expert en gestion de menuiserie.\n";
        stream << "history_size = 50\n";
        file.close();
        qDebug() << "Default config created at:" << m_configPath;
        qDebug() << "Please edit this file and add your OpenRouter API key!";
    }
}

ConfigManager::~ConfigManager()
{
}

void ConfigManager::loadConfig()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open config file:" << m_configPath;
        loadDefaults();
        return;
    }
    
    QTextStream stream(&file);
    QString currentSection;
    
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        
        if (line.isEmpty() || line.startsWith("#") || line.startsWith(";")) {
            continue;
        }
        
        if (line.startsWith("[") && line.endsWith("]")) {
            currentSection = line.mid(1, line.length() - 2);
            continue;
        }
        
        int equalPos = line.indexOf("=");
        if (equalPos > 0) {
            QString key = line.left(equalPos).trimmed();
            QString value = line.mid(equalPos + 1).trimmed();
            
            if (value.startsWith("\"") && value.endsWith("\"")) {
                value = value.mid(1, value.length() - 2);
            }
            
            if (currentSection == "OpenRouter") {
                if (key == "api_key") {
                    m_apiKey = value;
                    qDebug() << "API Key loaded (length:" << m_apiKey.length() << ")";
                } else if (key == "model") {
                    m_model = value;
                } else if (key == "temperature") {
                    m_temperature = value.toDouble();
                } else if (key == "max_tokens") {
                    m_maxTokens = value.toInt();
                }
            } else if (currentSection == "Chatbot") {
                if (key == "system_prompt") {
                    m_systemPrompt = value;
                } else if (key == "history_size") {
                    m_historySize = value.toInt();
                }
            }
        }
    }
    
    file.close();
    
    // Validate - la clé est valide si elle n'est pas vide et pas la valeur par défaut
    m_isValid = !m_apiKey.isEmpty() && m_apiKey.length() > 20;
    
    qDebug() << "Config loaded from:" << m_configPath;
    qDebug() << "  API Key:" << (m_isValid ? "✓ Present" : "✗ Missing or invalid");
    qDebug() << "  Model:" << m_model;
}

void ConfigManager::loadDefaults()
{
    m_apiKey = "";
    m_model = "openai/gpt-3.5-turbo";
    m_temperature = 0.7;
    m_maxTokens = 1000;
    m_systemPrompt = "Vous êtes WoodFlow Assistant, un expert en gestion de menuiserie.";
    m_historySize = 50;
    m_isValid = false;
    
    qDebug() << "Using default configuration";
}

QString ConfigManager::getApiKey() const
{
    return m_apiKey;
}

QString ConfigManager::getModel() const
{
    return m_model;
}

double ConfigManager::getTemperature() const
{
    return m_temperature;
}

int ConfigManager::getMaxTokens() const
{
    return m_maxTokens;
}

QString ConfigManager::getSystemPrompt() const
{
    return m_systemPrompt;
}

int ConfigManager::getHistorySize() const
{
    return m_historySize;
}

bool ConfigManager::isConfigValid() const
{
    return m_isValid;
}

void ConfigManager::reload()
{
    loadConfig();
}