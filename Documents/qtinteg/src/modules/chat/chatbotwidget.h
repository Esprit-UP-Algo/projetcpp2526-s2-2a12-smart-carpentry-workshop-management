#ifndef CHATBOTWIDGET_H
#define CHATBOTWIDGET_H

#include <QWidget>
#include <QList>
#include <QLabel>
#include "src/models/chatmessage.h"
#include "src/services/openrouterservice.h"

QT_BEGIN_NAMESPACE
class QTextEdit;
class QLineEdit;
class QPushButton;
QT_END_NAMESPACE

class ChatbotWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit ChatbotWidget(QWidget *parent = nullptr);
    ~ChatbotWidget();
    
private slots:
    void onSendMessage();
    void onResponseReceived(const QString& response);
    void onErrorOccurred(const QString& error);
    void onClearChat();

    
private:
    void setupUI();
    void addMessageToChat(const ChatMessage& message);
    
    QTextEdit* m_chatDisplay;
    QLineEdit* m_messageInput;
    QPushButton* m_sendButton;
    QLabel* m_statusLabel;
    
    OpenRouterService* m_openRouterService;
    QList<ChatMessage> m_conversation;
};

#endif // CHATBOTWIDGET_H