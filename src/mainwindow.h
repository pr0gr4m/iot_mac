#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
 * Author : Famous
 * Class for Main Window of program
 * Three Threads : 1. main Thread(GUI)
 *                 2. sniffer Thread
 *                 3. timer Thread
 *
 * Click the Start Btn then start sniffing
 * Clist the Stop Btn then Stop sniffing
 * every 9s timer will clear The STA_MAC_Map
 */

#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include <unordered_map>
#include "probereqsniffer.h"
#include "interfacewidget.h"
#include "qcustomplot.h"

using std::unordered_map;
using std::string;

#define COLOR_NUM 10

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setupPlot();
    void addSta(QString);
    void clear();
    inline QVector<QColor> rndColors(int);
    void setChannel(int);
    int getChannel();

private slots:
    void on_startBtn_clicked();
    void on_stopBtn_clicked();
    void on_selectBtn_clicked();
    void on_SSISpinBox_editingFinished();
    void on_exitBtn_clicked();
    void on_reFreshSpinBox_editingFinished();
    void on_hoppingCheck_clicked(bool checked);
    void on_STARadioBtn_toggled(bool checked);
    void on_APRadioBtn_toggled(bool checked);

    void onWidgetClosed();
    void onMaxDeviceChanged(QString);
    void onNewDevice(QString);
    void onTimerOut();
    void hoppingTimeout();
    void graphTimeout();

    void horzScrollBarChanged(int value);
    void vertScrollBarChanged(int value);
    void xAxisChanged(QCPRange range);
    void yAxisChanged(QCPRange range);

    void on_ChannelSpinBox_editingFinished();

private:
    typedef struct staParam{
        double staCount;
        int graphNum;
    } staParam;

    typedef unordered_map<HWAddress<6>, staParam> staCountMapType;

    Ui::MainWindow *ui;
    ProbeReqSniffer *sniffer;
    QTimer *timer;
    QTimer *hoppingTimer;
    QTimer *graphTimer;
    InterfaceWidget *widget;
    QString inf = "None";
    int interval = 0;
    int channel = 0;
    bool hopping = false;
    double graphTime = 0;

    staCountMapType staCountMap;

    QVector<QColor> colors;

};

#endif // MAINWINDOW_H
