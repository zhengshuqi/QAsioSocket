﻿#ifndef QASIOTCPSOCKET_H
#define QASIOTCPSOCKET_H

#include <QBuffer>
#include "ioserverthread.h"
#include <QPointer>
#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <array>

class QAsioTcpServer;

class QAsioTcpSocket : public QObject
{
    Q_OBJECT
public:
    explicit QAsioTcpSocket(QObject *parent = 0);
    virtual ~QAsioTcpSocket();

    enum SocketState {
        UnconnectedState = 0X00,
        ConnectingState = 0X01,
        ConnectedState = 0X02
    };
    enum SocketErroSite{
        ConnectEorro = 0X00,
        ReadError = 0X01,
        WriteEorro = 0X02,
        FindHostError = 0X03
    };

Q_SIGNALS:
    void readReadly();
    void connected();
    void disconnected();
    void sentError(const QString & site,const asio::error_code & erro_code);
    void stateChange(SocketState state);
    void hostFound();
public Q_SLOTS:
    void connectToHost(const QString &hostName, quint16 port);
    void connectToHost(const asio::ip::tcp::endpoint &peerPoint);

    void disconnectFromHost();

    qint64 bytesAvailable() const {return buffer.size() - buffer.pos();}
    QByteArray read(qint64 maxsize = 0)
    {
        QMutexLocker mutexLocker(&bufferMutex);
        return buffer.read(maxsize);
    }
    QByteArray readAll()
    {
        QMutexLocker mutexLocker(&bufferMutex);
        return buffer.readAll();
    }
    QByteArray readLine()
    {
        QMutexLocker mutexLocker(&bufferMutex);
        return buffer.readLine();
    }
    bool atEnd() const{return buffer.atEnd();}

    bool write(const QByteArray & data = QByteArray());

    asio::ip::tcp::endpoint peerEndPoint() const {return this->peerPoint;}
    SocketState state() const {return this->state_;}
    asio::error_code error() const {return this->erro_code;}
protected:
    void readHandler(const asio::error_code& error, std::size_t bytes_transferred);
    void writeHandler(const asio::error_code& error,std::size_t bytes_transferred);
    void connectedHandler(const asio::error_code& error, asio::ip::tcp::resolver::iterator iterator);
    void resolverHandle(const asio::error_code & error, asio::ip::tcp::resolver::iterator iterator);

    QAsioTcpSocket(asio::ip::tcp::socket * socket , QObject *parent = 0);//server类才能访问
    friend class QAsioTcpServer;

protected:
    void customEvent(QEvent * event);

    static QPointer<IOServerThread> ioserver;
private:
    QMutex writeMutex;
    QQueue<QByteArray> writeQueue;
private:
    QBuffer buffer;
    std::array<char,4096> data_;
    QMutex bufferMutex;
private:
    SocketState state_;
    asio::ip::tcp::socket * socket_ = nullptr;
    asio::error_code erro_code;
    asio::ip::tcp::endpoint peerPoint;
    asio::ip::tcp::resolver * resolver_ = nullptr;

    Q_DISABLE_COPY(QAsioTcpSocket)
};

#endif // QASIOTCPSOCKET_H
