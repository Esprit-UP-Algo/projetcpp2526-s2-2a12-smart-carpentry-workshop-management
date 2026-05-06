#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <QString>
#include <QDateTime>

class ChatMessage {
public:
    enum class Role {
        User,
        Assistant,
        System
    };
    
    ChatMessage() = default;
    explicit ChatMessage(Role role, const QString& content);
    
    // Getters
    Role role() const;
    QString content() const;
    QDateTime timestamp() const;
    
    // Setters
    void setRole(Role role);
    void setContent(const QString& content);
    void setTimestamp(const QDateTime& timestamp);
    
    // Conversion helpers
    QString roleToString() const;
    static Role stringToRole(const QString& roleStr);
    
private:
    Role m_role = Role::User;
    QString m_content;
    QDateTime m_timestamp;
};

#endif // CHATMESSAGE_H