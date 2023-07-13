#include "connector.h"
#include <QTimer>
#include <QDebug>

Connector::Connector(QObject *parent) : QObject(parent),
    m_socket(new QWebSocket()),
    m_url("ws://127.0.0.1:8000"),
    m_reconnectTimer(new QTimer(this))
{
    connect(m_socket, &QWebSocket::connected, this, &Connector::onSocketConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &Connector::onSocketDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &Connector::onSocketMessageReceived);
    connect(m_reconnectTimer, &QTimer::timeout, this, [=]() {
        m_socket->abort();
        m_socket->open(m_url);
    });
}

void Connector::open()
{
    m_socket->open(m_url);
//    qDebug() << "url : " << m_url;
}

void Connector::onSocketDisconnected()
{
    qDebug() << "disconnected ...";
//    m_reconnectTimer->start(15000);
}

void Connector::onSocketConnected()
{
    qDebug() << "connected ...";
}

void Connector::onSocketMessageReceived(const QString &message)
{
    qDebug() << "message : " << message;
}

void Connector::sendSocketMessage(const QByteArray &data)
{
    qDebug() << "size : " << m_socket->sendBinaryMessage(data);
}
