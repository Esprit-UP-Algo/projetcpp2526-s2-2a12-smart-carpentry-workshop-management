#include "chatbotwidget.h"
#include "src/config/configmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>
#include <QTextCursor>
#include <QTimer>

ChatbotWidget::ChatbotWidget(QWidget *parent) 
    : QWidget(parent)
{
    setupUI();
    
    // Initialiser le service
    m_openRouterService = new OpenRouterService(this);
    connect(m_openRouterService, &OpenRouterService::responseReceived, 
            this, &ChatbotWidget::onResponseReceived);
    connect(m_openRouterService, &OpenRouterService::errorOccurred, 
            this, &ChatbotWidget::onErrorOccurred);
    
    // Charger la configuration RÉELLE
    ConfigManager& config = ConfigManager::instance();
    
    QString apiKey = config.getApiKey();
    QString model = config.getModel();
    double temperature = config.getTemperature();
    int maxTokens = config.getMaxTokens();
    
    m_openRouterService->setApiKey(apiKey);
    m_openRouterService->setModel(model);
    m_openRouterService->setTemperature(temperature);
    m_openRouterService->setMaxTokens(maxTokens);
    
    // Message système
    m_conversation.clear();
    ChatMessage systemMsg(ChatMessage::Role::System, config.getSystemPrompt());
    m_conversation.append(systemMsg);
    
    // Status - simple et professionnel
    if (config.isConfigValid()) {
        m_statusLabel->setText("Connecte");
        m_statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
        m_sendButton->setEnabled(true);
    } else {
        m_statusLabel->setText("API invalide");
        m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
        m_sendButton->setEnabled(false);
    }
    
    qDebug() << "Chatbot Widget initialized with REAL API";
}

ChatbotWidget::~ChatbotWidget()
{
    qDebug() << "ChatbotWidget destroyed";
}

void ChatbotWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Header
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* titleLabel = new QLabel("WoodFlow AI Assistant", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50;");
    
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("QLabel { font-size: 11px; font-weight: bold; }");
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_statusLabel);
    
    // Chat display area
    m_chatDisplay = new QTextEdit(this);
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setMinimumHeight(400);
    m_chatDisplay->setStyleSheet(R"(
        QTextEdit {
            background-color: #f8f9fa;
            font-family: 'Segoe UI', Arial;
            font-size: 12pt;
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 10px;
        }
    )");
    
    // Input area
    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_messageInput = new QLineEdit(this);
    m_messageInput->setPlaceholderText("Posez votre question sur WoodFlow...");
    m_messageInput->setMinimumHeight(40);
    m_messageInput->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 12pt;
        }
        QLineEdit:focus {
            border-color: #8A9A5B;
        }
    )");
    
    m_sendButton = new QPushButton("Envoyer", this);
    m_sendButton->setMinimumHeight(40);
    m_sendButton->setCursor(Qt::PointingHandCursor);
    m_sendButton->setStyleSheet(R"(
        QPushButton {
            background-color: #8A9A5B;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #9aaa6b;
        }
        QPushButton:pressed {
            background-color: #6a8040;
        }
        QPushButton:disabled {
            background-color: #cccccc;
        }
    )");
    
    QPushButton* clearButton = new QPushButton("Effacer", this);
    clearButton->setMinimumHeight(40);
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setStyleSheet(R"(
        QPushButton {
            background-color: #e2e8f0;
            color: #4a5568;
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
        }
        QPushButton:hover {
            background-color: #cbd5e0;
        }
    )");
    
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    inputLayout->addWidget(clearButton);
    
    // Assemble layout
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(m_chatDisplay);
    mainLayout->addLayout(inputLayout);
    
    // Connect signals
    connect(m_sendButton, &QPushButton::clicked, this, &ChatbotWidget::onSendMessage);
    connect(clearButton, &QPushButton::clicked, this, &ChatbotWidget::onClearChat);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &ChatbotWidget::onSendMessage);
}

void ChatbotWidget::onSendMessage()
{
    QString userMessage = m_messageInput->text().trimmed();
    if (userMessage.isEmpty()) return;
    
    qDebug() << "=== SENDING MESSAGE ===";
    qDebug() << "User message:" << userMessage;
    
    // Add user message
    ChatMessage userMsg(ChatMessage::Role::User, userMessage);
    addMessageToChat(userMsg);
    m_conversation.append(userMsg);
    
    // Clear input and disable button
    m_messageInput->clear();
    m_sendButton->setEnabled(false);
    m_statusLabel->setText("Reflexion...");
    m_statusLabel->setStyleSheet("QLabel { color: orange; font-weight: bold; }");
    
    // Envoyer au service
    m_openRouterService->sendMessage(m_conversation);
}

void ChatbotWidget::onResponseReceived(const QString& response)
{
    qDebug() << "=== RESPONSE RECEIVED ===";
    qDebug() << "Response:" << response.left(100);
    
    ChatMessage assistantMsg(ChatMessage::Role::Assistant, response);
    addMessageToChat(assistantMsg);
    m_conversation.append(assistantMsg);
    
    m_sendButton->setEnabled(true);
    m_statusLabel->setText("Connecte");
    m_statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
}

void ChatbotWidget::onErrorOccurred(const QString& error)
{
    qDebug() << "=== ERROR OCCURRED ===";
    qDebug() << "Error:" << error;
    
    // Message d'erreur simple
    QString errorResponse = "Erreur: " + error + "\n\nVeuillez verifier votre connexion et votre clé API.";
    
    ChatMessage assistantMsg(ChatMessage::Role::Assistant, errorResponse);
    addMessageToChat(assistantMsg);
    m_conversation.append(assistantMsg);
    
    m_sendButton->setEnabled(true);
    m_statusLabel->setText("Erreur");
    m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
}

void ChatbotWidget::addMessageToChat(const ChatMessage& message)
{
    QString alignment = (message.role() == ChatMessage::Role::User) ? "right" : "left";
    QString bgColor = (message.role() == ChatMessage::Role::User) ? "#DCF8C6" : "#FFFFFF";
    QString roleName = (message.role() == ChatMessage::Role::User) ? "Vous" : "Assistant";
    
    QString html = QString(R"(
        <div style="text-align: %1; margin: 10px 0;">
            <div style="display: inline-block; max-width: 80%%; 
                        background-color: %2; border-radius: 12px; 
                        padding: 12px; box-shadow: 0 1px 2px rgba(0,0,0,0.1);
                        text-align: left;">
                <div style="color: #666; font-size: 11px; margin-bottom: 5px; font-weight: bold;">
                    %3
                </div>
                <div style="color: #333; line-height: 1.5;">
                    %4
                </div>
                <div style="color: #999; font-size: 9px; margin-top: 8px;">
                    %5
                </div>
            </div>
        </div>
    )").arg(alignment, bgColor, roleName,
           message.content().toHtmlEscaped().replace("\n", "<br>"),
           message.timestamp().toString("hh:mm:ss"));
    
    m_chatDisplay->append(html);
    
    // Auto-scroll to bottom
    QTextCursor cursor = m_chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_chatDisplay->setTextCursor(cursor);
}

void ChatbotWidget::onClearChat()
{
    // Keep only system message
    while (m_conversation.size() > 1) {
        m_conversation.removeLast();
    }
    m_chatDisplay->clear();
    
    // Add welcome message simple
    addMessageToChat(ChatMessage(ChatMessage::Role::Assistant, 
        "Bonjour! Je suis l'assistant WoodFlow.\n\n"
        "Je peux vous aider avec:\n"
        "- Gestion des projets\n"
        "- Gestion des employes\n"
        "- Gestion des stocks\n"
        "- Gestion financiere\n"
        "- Gestion des designs \n\n"
        "Comment puis-je vous aider?"));
}
