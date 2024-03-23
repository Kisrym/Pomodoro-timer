#include "pomodorotimer.h"

PomodoroTimer::PomodoroTimer()
    : workDuration(25), breakDuration(5), currentState(Work), isPaused(false)
{   
    connect(&timer, &QTimer::timeout, this, &PomodoroTimer::timerTimeout);
}

void PomodoroTimer::start(int totalTime, bool skip_pauses) {
    this->totalWorkTime = totalTime;
    this->skip_pauses = skip_pauses;

    if (totalWorkTime < 25*60) {
        workDuration = totalWorkTime / 60;
    }

    currentState = Work;
    remainingTime = workDuration * 60;
    timer.start(1000); // Inicia o temporizador para atualizar a cada segundo
}

void PomodoroTimer::stop() {
    this->timer.stop();
}

void PomodoroTimer::pause() {
    isPaused = true;
    stop();
}

void PomodoroTimer::resume() {
    isPaused = false;
    timer.start(1000);
}

void PomodoroTimer::timerTimeout() {
    if (isPaused) {
        return;
    }
    if ((totalWorkTime - 25 * 60) <= 0) {
        workDuration = totalWorkTime; // se o tempo restante for menor que 25 minutos, ele vai ser a nova duração
    }  
    remainingTime--;
    if (remainingTime <= 0) {
        if (currentState == Work) {
            totalWorkTime -= 25 * 60;

            currentState = (skip_pauses) ? Work : Break;
            remainingTime = breakDuration * 60;
        } else {
            totalWorkTime -= 5 * 60;

            currentState = Work;
            remainingTime = workDuration * 60;
        }
    }

    qDebug() << remainingTime;

    if (totalWorkTime <= 0) {
        emit workFinished();
        stop();
        return;
    }

    emit timeUpdated(remainingTime, (currentState == Work) ? workDuration : breakDuration); // vai mandar a duração para o valor máximo do progressbar
}
