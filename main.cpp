#include "pomodoro.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Pomodoro w;
    w.show();
    return a.exec();
}
