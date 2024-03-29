#ifndef POMODORO_H
#define POMODORO_H

#include <QMainWindow>
#include <QTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include "pomodorotimer.h"


QT_BEGIN_NAMESPACE
namespace Ui { class Pomodoro; }
QT_END_NAMESPACE

class Pomodoro : public QMainWindow
{
    Q_OBJECT

public:
    Pomodoro(QWidget *parent = nullptr);
    ~Pomodoro();

private slots:
    void time_changed();
    void start_pomodoro();
    void metaDiariaUpdate();

    void labelUpdate(int seconds);
    void progressUpdate(int seconds, int workDuration);
    //void sessionLabelUpdate(unsigned int state);

    void pause_resume();
    void workFinished();

private:
    Ui::Pomodoro *ui;
    int time;
    bool isPaused;

    PomodoroTimer pomodoro_timer;
    void saveJsonFile(const QJsonDocument &doc, const QString &filePath);
    void loadData();

    int sessionsWork;
    int sessionsBreak;
};
#endif // POMODORO_H
