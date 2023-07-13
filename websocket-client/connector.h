#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QObject>
#include <QWebSocket>

class QTimer;

class Connector : public QObject
{
    Q_OBJECT
public:
    explicit Connector(QObject *parent = nullptr);

public slots:
    void open();
    void onSocketDisconnected();
    void onSocketConnected();
    void onSocketMessageReceived(const QString &message);
    void sendSocketMessage(const QByteArray &data);

private:
    QWebSocket *m_socket;
    QUrl m_url;
    QTimer *m_reconnectTimer;
};

#endif // CONNECTOR_H
