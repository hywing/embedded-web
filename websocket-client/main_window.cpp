#include "main_window.h"
#include "./ui_main_window.h"
#include "connector.h"
#include <QJsonObject>
#include <QJsonDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_connector(new Connector(this))
{
    ui->setupUi(this);
    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(ui->sendButton, &QPushButton::clicked, this, [=]() {
        QJsonObject obj;
        obj.insert("msg", "Calibration");
        obj.insert("Peak_Power", 80);
        QJsonDocument json_document;
        json_document.setObject(obj);
        QByteArray byte_array = json_document.toJson(QJsonDocument::Compact);
        QString json_str(byte_array);
        qDebug()<< json_str;
        m_connector->sendSocketMessage(byte_array);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnectButtonClicked()
{
    m_connector->open();
}

