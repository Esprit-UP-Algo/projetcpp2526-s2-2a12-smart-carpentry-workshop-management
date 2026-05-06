#include "chatmessage.h"

ChatMessage::ChatMessage(Role role, const QString& content)
    : m_role(role)
    , m_content(content)
    , m_timestamp(QDateTime::currentDateTime()) {
}

ChatMessage::Role ChatMessage::role() const {
    return m_role;
}

QString ChatMessage::content() const {
    return m_content;
}

QDateTime ChatMessage::timestamp() const {
    return m_timestamp;
}

void ChatMessage::setRole(Role role) {
    m_role = role;
}

void ChatMessage::setContent(const QString& content) {
    m_content = content;
}

void ChatMessage::setTimestamp(const QDateTime& timestamp) {
    m_timestamp = timestamp;
}

QString ChatMessage::roleToString() const {
    switch(m_role) {
        case Role::User:
            return "User";
        case Role::Assistant:
            return "Assistant";
        case Role::System:
            return "System";
        default:
            return "Unknown";
    }
}

ChatMessage::Role ChatMessage::stringToRole(const QString& roleStr) {
    QString lowerStr = roleStr.toLower();
    if (lowerStr == "user") {
        return Role::User;
    } else if (lowerStr == "assistant") {
        return Role::Assistant;
    } else if (lowerStr == "system") {
        return Role::System;
    }
    return Role::User; // Default
}