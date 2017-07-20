#ifndef DIALOG_RUN_H
#define DIALOG_RUN_H

#include <QDialog>
#include <QPushButton>
#include <QSpacerItem>
#include <QPlainTextEdit>
#include <QLayout>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QMessageBox>


#include "dialog_view.h"
#include "dialog_fileview.h"
#include "console_redir.h"
#include "dialog_runoptions.h"
#include "chartdialog.h"

#define BUFFER_SIZE 256


namespace Ui {
class Dialog_run;
}

class Dialog_run : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_run(QWidget *parent = 0);
    ~Dialog_run();

public slots:
    void runSS();
    void startRun();
    void pauseRun();
    void stopRun();
    void rejected();
    void runStarted();
    void runCompleted(int code);

    void setDir(QString dir);
    void setExe(QString exe);
    void setOptions (QString opt = QString(""));
    void changeExe ();

    void setNohess(bool value);
    void setShess(bool value);
    void setEst(bool value);
    void setNoest(bool value);
    void setLprof(bool value);
    void setMcdiag(bool value);

    void stdOutput();
    void stdError();
    void outputLine();

    void showRptCharts();
    void showWarnFile();
    void showEchoFile();

    void showOptions();
    void applyOptions();
    void resetOptions();

    void onProcessStarted();
    void onProcessStdOutWrite(QString szOutput);
    void onProcessStdErrWrite(QString szOutput);
    void onProcessTerminate();

    void increaseFont(bool flag = true);
    void decreaseFont(bool flag = true);
    void setFontSize (int fsize);

signals:
    void complete();
    void canceled();

private:
    Ui::Dialog_run *ui;
    Dialog_fileView *warnview;
    Dialog_fileView *echoview;
    chartDialog *charts;

    QVBoxLayout *layout;
    QPlainTextEdit *out;
    QPushButton *done;
    QHBoxLayout *buttons;

    int fixedPosition;
    QProcess *stocksynth;
    QString output;
    bool running;

    QFont dfont;

    Dialog_runoptions *runoptions;
    bool nohess;
    bool shess;
    bool est;
    bool noest;
    bool lprof;
    bool mcdiag;

    void setUiEnabled(bool flag);
    void finishOutput();

};

#endif // DIALOG_RUN_H
