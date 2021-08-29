#ifndef QT_TEST_HPP
#define QT_TEST_HPP

// main.cpp
#include <QtCore>

class Task : public QObject
{
    Q_OBJECT
public:
    Task(QObject *parent = 0) : QObject(parent) {}

public slots:
    void run()
    {
        printf("run\n");
        // Do processing here

        emit finished();
    }

signals:
    void finished();
};

// #include "moc_qt_main.cpp"

#endif