#include "pomodoro.h"
#include "./ui_pomodoro.h"

Pomodoro::Pomodoro(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Pomodoro)
    , isPaused(false)
    , sessionsWork(0)
    , sessionsBreak(0)
{
    ui->setupUi(this);
    ui->skip_pauses->setCheckable(false);

    startBlocking();

    // loading save files
    loadData();

    connect(ui->time, &QLineEdit::editingFinished, this, &Pomodoro::time_changed);
    connect(ui->start_pomodoro, &QPushButton::clicked, this, &Pomodoro::start_pomodoro);
    connect(ui->meta_diaria, &QLineEdit::editingFinished, this, &Pomodoro::metaDiariaUpdate);
    connect(ui->skip_pauses, &QCheckBox::clicked, this, [=](bool checked){
        if (checked) {
            ui->label_interval->setText("Você não terá pausas");
            return;
        }
        this->time_changed();
    });
    connect(ui->lista_sites, &QListWidget::customContextMenuRequested, this, &Pomodoro::showContextMenu);

    connect(&pomodoro_timer, &PomodoroTimer::timeUpdated, this, &Pomodoro::labelUpdate);
    connect(&pomodoro_timer, &PomodoroTimer::timeUpdated, this, &Pomodoro::progressUpdate);

    connect(ui->pause_btn, &QPushButton::clicked, this, &Pomodoro::pause_resume);
    connect(ui->back_btn, &QPushButton::clicked, &pomodoro_timer, [=]{
        ui->stackedWidget->setCurrentWidget(ui->page_1);
        pomodoro_timer.stop();
    });
    connect(ui->add_btn, &QPushButton::clicked, this, [=]{ui->time->setText(QString::number(ui->time->text().toInt() + 10));time_changed();}); // adicionando mais 10 minutos
    connect(ui->sub_btn, &QPushButton::clicked, this, [=]{ui->time->setText(QString::number(ui->time->text().toInt() - 10));time_changed();}); // subtraindo 10 minutos

    connect(&pomodoro_timer, &PomodoroTimer::workFinished, this, &Pomodoro::workFinished);
    connect(&pomodoro_timer, &PomodoroTimer::sessionUpdated, this, [=](bool state){
        saveData(state);
    });
    //connect(&pomodoro_timer, &PomodoroTimer::sessionUpdated, this, &Pomodoro::sessionLabelUpdate);




    connect(ui->lineEdit, &QLineEdit::textChanged, &pomodoro_timer, &PomodoroTimer::changeRemainingTimeTest);
}

Pomodoro::~Pomodoro()
{
    delete ui;
    process.deleteLater();

    // salvando meta diária
    saveData();
}

void Pomodoro::saveJsonFile(const QJsonDocument &doc, const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Erro ao abrir o arquivo";
        return;
    }
    qDebug() << "salvando arquivo...";
    QTextStream out(&file);
    out << doc.toJson();
    file.close();
}

void Pomodoro::saveData(bool status) {
    QJsonObject dados;
    dados["date"] = QDate::currentDate().toString();
    dados["meta_diaria"] = ui->meta_diaria->text().toInt();
    dados["time"] = ui->time->text().toInt();
    dados["progresso"] = ui->progressBar->value();

    QJsonArray sites;
    for (int i=0; i < ui->lista_sites->count(); i++) {
        sites.append(ui->lista_sites->item(i)->text());
        qDebug() << "sites: " << sites;
    }

    dados["blocked-sites"] = sites;
    dados["status"] = status;
    QJsonDocument doc(dados);

    saveJsonFile(doc, "sites.json");
}

void Pomodoro::loadData() {
    QFile file("sites.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Falha ao abrir o arquivo";
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &error);
    if (jsonDoc.isNull()) {
        qDebug() << "Failed to parse JSON:" << error.errorString();
        return;
    }

    if (!jsonDoc.isObject()) {
        qDebug() << "JSON document is not an object";
        return;
    }

    QJsonObject json = jsonDoc.object();

    if (json["date"].toString() == QDate::currentDate().toString()) {
        ui->meta_diaria->setText(QString::number(json["meta_diaria"].toInt()));
        ui->progressBar->setValue(json["progresso"].toInt());
        ui->time->setText(QString::number(json["time"].toInt()));

        metaDiariaUpdate();
        time_changed();
    }

    // adicionando os sites da "database" na lista
    QJsonArray sites = json["blocked-sites"].toArray();
    for (int i = 0; i < sites.size(); i++) {
        QJsonValue value = sites.at(i);
        qDebug() << value;
        ui->lista_sites->addItem(value.toString());
    }
}

void Pomodoro::pause_resume() {
    qDebug() << "clicou";
    if (isPaused) {
        pomodoro_timer.resume();
        isPaused = false;
        ui->pause_btn->setIcon(QIcon(":/icons/pause.png"));
        return;
    }
    pomodoro_timer.pause();
    ui->pause_btn->setIcon(QIcon(":/icons/play.png"));
    isPaused = true;
}

void Pomodoro::time_changed() { // muda os valores de label/progress bar
    time = ui->time->text().toInt();

    if (time >= 60) {
        int time_hours = time / 60;
        ui->label_hours_left->setText(QString::number(time_hours) + " hora(s) e " + QString::number(time % 60) + " minuto(s)");
    }

    else {
        ui->label_hours_left->setText("");
    }

    if ((time / 30) >= 1) {
        ui->label_interval->setText("Você terá " + QString::number(time / 30) + " intervalos");
        ui->skip_pauses->setCheckable(true);
    }

    else {
        ui->label_interval->setText("Você não terá intervalos");
        ui->skip_pauses->setCheckable(false);
    }
}

void Pomodoro::start_pomodoro() {
    bool skip = false;
    if (ui->skip_pauses->isChecked() || (time / 30) < 1) {
        skip = true;
    }

    qDebug() << time;

    pomodoro_timer.start(time * 60, skip);
    qDebug() << "Iniciando pomodoro";
    qDebug() << skip;
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}

void Pomodoro::labelUpdate(int seconds) {
    QTime time(0, 0);
    time = time.addSecs(seconds);
    ui->time_label->setText("Tempo restante: " + time.toString("mm:ss"));
}

void Pomodoro::progressUpdate(int seconds, int workDuration) {
    ui->time_progressbar->setMaximum(workDuration * 60);
    ui->time_progressbar->setValue(workDuration * 60 - seconds); // adicionando os segundos ao progressbar
}

void Pomodoro::workFinished() {
    qDebug() << "Terminou";
    ui->stackedWidget->setCurrentWidget(ui->page_1);
    ui->progressBar->setValue(ui->progressBar->value() + time);
}

void Pomodoro::metaDiariaUpdate() {
    ui->progressBar->setMaximum(ui->meta_diaria->text().toInt() * 60);
}
/*
void Pomodoro::sessionLabelUpdate(unsigned int state) {
    switch (state) {
    case 0:
        sessionsWork += 1;
        ui->session_label->setText("Período de foco ")
    }
}
*/

void Pomodoro::startBlocking() {
    QDir::setCurrent("/home/kaio/Desktop/Storage/projetos_qt/Pomodoro-timer/");

    process.moveToThread(QCoreApplication::instance()->thread());

    QStringList args;
    process.start("release/block_sites", args);
    qDebug() << "começando processo";

    if (!process.waitForStarted()){
        qDebug() << process.errorString();
        return;
    }

    connect(qApp, &QCoreApplication::aboutToQuit, &process, &QProcess::terminate); // assim, o processo vai fechar quando o aplicativo for fechado

    process.waitForFinished(-1);
}

void Pomodoro::showContextMenu(const QPoint &pos) {
    QMenu menu;

    QAction *adicionar = menu.addAction("Adicionar");
    QAction *excluir = menu.addAction("Excluir");

    QAction *selectedAction = menu.exec(ui->lista_sites->viewport()->mapToGlobal(pos));
    if (selectedAction == adicionar) {
        bool ok;
        QString novo_site = QInputDialog::getText(this, tr("Link do site:"), tr("Site:"), QLineEdit::Normal, tr(""), &ok);
        if (ok && !novo_site.isEmpty()){
            ui->lista_sites->addItem(novo_site);
            saveData();
        }
    }
    if (selectedAction == excluir) {
        delete ui->lista_sites->itemAt(pos);
        saveData();
    }
}
