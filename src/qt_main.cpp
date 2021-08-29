// main.cpp
#include "qt_main.hpp"

int main(int argc, char *argv[])
{
    printf("QtTest - BEGIN\n");
    QCoreApplication a(argc, argv);

    // Task parented to the application so that it
    // will be deleted by the application.
    Task *task = new Task(&a);

    // This will cause the application to exit when
    // the task signals finished.    
    QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));

    // This will run the task from the application event loop.
    printf("sending timer\n");
    QTimer::singleShot(0, task, SLOT(run()));
    printf("timer sent\n");

    return a.exec();
}