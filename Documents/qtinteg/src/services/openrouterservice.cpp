#include "openrouterservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QTimer>

OpenRouterService::OpenRouterService(QObject *parent) 
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_model("openai/gpt-3.5-turbo")
    , m_temperature(0.7)
    , m_maxTokens(1000)
    , m_currentReply(nullptr)
{
    qDebug() << "OpenRouterService initialized (REAL API mode)";
}

OpenRouterService::~OpenRouterService()
{
}

void OpenRouterService::setApiKey(const QString& apiKey)
{
    m_apiKey = apiKey;
    qDebug() << "API Key set (length):" << m_apiKey.length();
}

void OpenRouterService::setModel(const QString& model)
{
    m_model = model;
}

void OpenRouterService::setTemperature(double temperature)
{
    m_temperature = temperature;
}

void OpenRouterService::setMaxTokens(int maxTokens)
{
    m_maxTokens = maxTokens;
}

QJsonObject OpenRouterService::createRequestPayload(const QList<ChatMessage>& conversation)
{
    QJsonObject payload;
    payload["model"] = m_model;
    payload["temperature"] = m_temperature;
    payload["max_tokens"] = m_maxTokens;
    
    QJsonArray messages;
    for (const auto& msg : conversation) {
        QJsonObject message;
        QString role = msg.roleToString().toLower();
        message["role"] = role;
        message["content"] = msg.content();
        messages.append(message);
    }
    
    payload["messages"] = messages;
    return payload;
}

void OpenRouterService::sendMessage(const QList<ChatMessage>& conversation)
{
    // PAS DE MODE MOCK - APPEL API RÉEL
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API key not set");
        return;
    }
    
    if (conversation.isEmpty()) {
        emit errorOccurred("No conversation provided");
        return;
    }
    
    QNetworkRequest request;
    request.setUrl(QUrl("https://openrouter.ai/api/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    request.setRawHeader("HTTP-Referer", "http://localhost:3000");
    request.setRawHeader("X-Title", "WoodFlow");
    
    QJsonObject payload = createRequestPayload(conversation);
    QJsonDocument doc(payload);
    
    QByteArray postData = doc.toJson();
    
    qDebug() << "=== SENDING REAL API REQUEST ===";
    qDebug() << "URL:" << request.url().toString();
    qDebug() << "Model:" << m_model;
    qDebug() << "Payload size:" << postData.size();
    
    m_currentReply = m_networkManager->post(request, postData);
    connect(m_currentReply, &QNetworkReply::finished, this, &OpenRouterService::onReplyFinished);
    connect(m_currentReply, &QIODevice::readyRead, this, &OpenRouterService::onReadyRead);
}

void OpenRouterService::onReplyFinished()
{
    if (!m_currentReply) return;
    
    QByteArray responseData = m_currentReply->readAll();
    
    qDebug() << "=== OPENROUTER RESPONSE ===";
    qDebug() << "HTTP Status:" << m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "Response:" << QString(responseData);
    
    if (m_currentReply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        
        if (doc.isNull()) {
            emit errorOccurred("Invalid JSON response");
            m_currentReply->deleteLater();
            m_currentReply = nullptr;
            return;
        }
        
        QJsonObject obj = doc.object();
        
        if (obj.contains("error")) {
            QString errorMsg = obj["error"].toObject()["message"].toString();
            emit errorOccurred("API Error: " + errorMsg);
            m_currentReply->deleteLater();
            m_currentReply = nullptr;
            return;
        }
        
        QString response = obj["choices"].toArray()[0].toObject()
                          ["message"].toObject()["content"].toString();
        
        if (response.isEmpty()) {
            emit errorOccurred("Empty response");
        } else {
            emit responseReceived(response);
        }
    } else {
        QString error = QString("Network error: %1").arg(m_currentReply->errorString());
        emit errorOccurred(error);
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void OpenRouterService::onReadyRead()
{
    // Optional: handle streaming
}