#ifndef OPENROUTERSERVICE_H
#define OPENROUTERSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include "src/models/chatmessage.h"

class OpenRouterService : public QObject {
    Q_OBJECT
    
public:
    explicit OpenRouterService(QObject *parent = nullptr);
    ~OpenRouterService();
    
    void setApiKey(const QString& apiKey);
    void sendMessage(const QList<ChatMessage>& conversation);
    
    // Configuration
    void setModel(const QString& model);
    void setTemperature(double temperature);
    void setMaxTokens(int maxTokens);
    
    // Getters
    QString getApiKey() const { return m_apiKey; }
    
signals:
    void responseReceived(const QString& response);
    void errorOccurred(const QString& error);
    void streamingResponse(const QString& chunk);
    
private:
    QString generateMockResponse(const QString& userMessage);
    QJsonObject createRequestPayload(const QList<ChatMessage>& conversation);
    void onReplyFinished();
    void onReadyRead();
    
    QNetworkAccessManager* m_networkManager;
    QString m_apiKey;
    QString m_model;
    double m_temperature;
    int m_maxTokens;
    QNetworkReply* m_currentReply;
};

#endif // OPENROUTERSERVICE_H