#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QDebug>

class ConfigManager
{
public:
    static ConfigManager& instance();
    
    // OpenRouter settings
    QString getApiKey() const;
    QString getModel() const;
    double getTemperature() const;
    int getMaxTokens() const;
    
    // Chatbot settings
    QString getSystemPrompt() const;
    int getHistorySize() const;
    
    // Check if config file exists
    bool isConfigValid() const;
    
    // Reload configuration
    void reload();
    
private:
    ConfigManager();
    ~ConfigManager();
    
    void loadConfig();
    void loadDefaults();
    void createDefaultConfigFile();
    
    QString m_apiKey;
    QString m_model;
    double m_temperature;
    int m_maxTokens;
    QString m_systemPrompt;
    int m_historySize;
    QString m_configPath;
    
    bool m_isValid;
};

#endif // CONFIGMANAGER_H