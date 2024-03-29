#ifndef POMODOROTIMER_H
#define POMODOROTIMER_H

#include <QObject>
#include <QWidget>
#include <QTimer>

class PomodoroTimer : public QObject
{
    Q_OBJECT
public:
    PomodoroTimer();

    void start(int totalTime, bool skip_pauses);
    void stop();
    void pause();
    void resume();
    void changeRemainingTimeTest(QString value);

private slots:
    void timerTimeout();

signals:
    void timeUpdated(int seconds, int workDuration);
    void sessionUpdated(unsigned int state);
    void workFinished();

private:
    enum State {Work, Break};
    QTimer timer;
    int workDuration;
    int breakDuration;
    int totalWorkTime;
    int remainingTime;
    State currentState;

    bool skip_pauses;
    bool isPaused;
};

#endif // POMODOROTIMER_H
