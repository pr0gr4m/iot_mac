#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupPlot();

    colors = rndColors(COLOR_NUM);

    //QMainWindow::showMaximized();
    sniffer = new ProbeReqSniffer(this);
    connect(sniffer, SIGNAL(MaxDeviceChanged(QString)), this, SLOT(onMaxDeviceChanged(QString)));
    connect(sniffer, SIGNAL(NewDevice(QString)), this, SLOT(onNewDevice(QString)));

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerOut()));

    hoppingTimer = new QTimer();
    connect(hoppingTimer, SIGNAL(timeout()), this, SLOT(hoppingTimeout()));

    graphTimer = new QTimer();
    connect(graphTimer, SIGNAL(timeout()), this, SLOT(graphTimeout()));

    ui->startBtn->setDisabled(true);
    ui->stopBtn->setDisabled(true);
    ui->interfaceLabel->setText(inf);
    ui->SSISpinBox->setValue(-35);

    ui->customPlot->xAxis->setRange(0, 8);
    ui->customPlot->yAxis->setRange(0, 100);

    ui->horizontalScrollBar->setRange(0, 8);
    ui->verticalScrollBar->setRange(0, 100);

    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
    connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(vertScrollBarChanged(int)));
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisChanged(QCPRange)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupPlot(){
    ui->customPlot->xAxis->setLabel("Time(sec)");
    ui->customPlot->yAxis->setLabel("Detected Signals");

    ui->customPlot->axisRect()->setupFullAxesBox();
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    ui->customPlot->legend->setVisible(true);
    ui->customPlot->legend->setFont(QFont("Helvetica", 9));

    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));
}

void MainWindow::onMaxDeviceChanged(QString sta){
    if(sta == ui->MACLineEdit->text())
        return;
//    if(ui->MACLineEdit->text().length() == 17)
//        ui->customPlot->graph(staCountMap.at(ui->MACLineEdit->text().toStdString()).graphNum)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));

    ui->MACLineEdit->setText(sta);
    ui->infoLabel->setText("Max Count Device Changed");
    addSta(sta);

    //ui->customPlot->graph(staCountMap.at(sta.toStdString()).graphNum)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
}

void MainWindow::onNewDevice(QString sta){
    ui->infoLabel->setText("New Device Founded");
    addSta(sta);
}

void MainWindow::addSta(QString sta){
    HWAddress<6> key = sta.toStdString();
    if(staCountMap.find(key) == staCountMap.end()){

        staParam param;

        param.staCount = 1;
        param.graphNum = ui->customPlot->graphCount();
        staCountMap.insert(std::make_pair(key, param));

        ui->customPlot->addGraph();
        ui->customPlot->graph(param.graphNum)->setPen(QPen(colors.at(param.graphNum % COLOR_NUM + 1)));
        ui->customPlot->graph()->setName(sta);
    }
}

void MainWindow::onTimerOut(){
    sniffer->clearMap();
    qDebug() << "Timeout : clear Map";
}

void MainWindow::on_selectBtn_clicked()
{
    ui->selectBtn->setDisabled(true);
    widget = new InterfaceWidget;
    widget->setAttribute(Qt::WA_DeleteOnClose);
    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(onWidgetClosed()));
    widget->show();
}

void MainWindow::onWidgetClosed(){
    QString ret = widget->getInf();

    if((ret != NULL && ret != "")){
        inf = ret;
        ui->interfaceLabel->setText(inf);
        ui->startBtn->setDisabled(false);

        ui->infoLabel->setText("Press Start and toggle device WiFi");
    }else if(inf != "None"){
    }else
        ui->infoLabel->setText("Please select right interface..");
    ui->selectBtn->setDisabled(false);
}

void MainWindow::on_startBtn_clicked()
{
    if(ui->customPlot->graphCount() != 0)
        ui->customPlot->clearGraphs();

    ui->MACLineEdit->setText("");
    ui->infoLabel->setText("Waiting signal...");
    ui->startBtn->setDisabled(true);
    ui->stopBtn->setDisabled(false);
    ui->SSISpinBox->setDisabled(true);
    ui->reFreshSpinBox->setDisabled(true);
    ui->selectBtn->setDisabled(true);

    sniffer->setInf(inf.toStdString());
    sniffer->start();

    if(interval != 0){
        timer->setInterval(interval);
        timer->start();
    }

    if(hopping){
        hoppingTimer->setInterval(300);
        hoppingTimer->start();
    }

    graphTimer->setInterval(500);
    graphTimer->start();
}

void MainWindow::on_stopBtn_clicked()
{
    sniffer->setStop(true);
    ui->infoLabel->setText("Stop snifiing.");

    timer->stop();
    hoppingTimer->stop();
    graphTimer->stop();

    graphTime = 0;

    staCountMap.clear();

    ui->stopBtn->setDisabled(true);
    ui->startBtn->setDisabled(false);
    ui->SSISpinBox->setDisabled(false);
    ui->reFreshSpinBox->setDisabled(false);
    ui->selectBtn->setDisabled(false);

    ui->customPlot->replot();
}

void MainWindow::setChannel(int val){
    this->channel = val;
}

void MainWindow::on_SSISpinBox_editingFinished()
{
    this->sniffer->setSSI_MIN(ui->SSISpinBox->value());
}

void MainWindow::on_exitBtn_clicked()
{
    exit(0);
}

void MainWindow::on_reFreshSpinBox_editingFinished()
{
    interval = ui->reFreshSpinBox->value() * 1000;
}

void MainWindow::on_hoppingCheck_clicked(bool checked)
{
    if(checked == true)
        this->hopping = true;
    else
        this->hopping = false;
}

void MainWindow::hoppingTimeout(){
    if(hopping){
        channel = (channel+4) % 13 + 1;
        std::system(("iwconfig " + inf.toStdString() + " channel " + std::to_string(channel)).c_str());
        ui->ChannelSpinBox->setValue(channel);
    }else
        hoppingTimer->stop();
}

void MainWindow::on_STARadioBtn_toggled(bool checked)
{
    if(checked)
        this->sniffer->setTarget(STATION);
}

void MainWindow::on_APRadioBtn_toggled(bool checked)
{
    if(checked)
        this->sniffer->setTarget(AP);
}

void MainWindow::graphTimeout(){
    graphTime+=0.5;

    for(auto it = staCountMap.begin(); it != staCountMap.end(); ++it){
        (*it).second.staCount = sniffer->getStaCount((*it).first);
        ui->customPlot->graph((*it).second.graphNum)->addData(graphTime, (*it).second.staCount);
    }

    ui->customPlot->yAxis->setRange(0, sniffer->getMaxCount() * 2);
    ui->customPlot->xAxis->setRange(0, graphTime * 1.5, Qt::AlignLeft);
    ui->customPlot->replot();
}


inline QVector<QColor> MainWindow::rndColors(int count){
    QVector<QColor> colors;
    float currentHue = 0.0;
    for (int i = 0; i < count; i++){
        colors.push_back( QColor::fromHslF(currentHue, 1.0, 0.5) );
        currentHue += 0.618033988749895f;
        currentHue = std::fmod(currentHue, 1.0f);
    }
    return colors;
}

void MainWindow::horzScrollBarChanged(int value)
{
  if (qAbs(ui->customPlot->xAxis->range().center()-value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
  {
    ui->customPlot->xAxis->setRange(value/100.0, ui->customPlot->xAxis->range().size(), Qt::AlignCenter);
    ui->customPlot->replot();
  }
}

void MainWindow::vertScrollBarChanged(int value)
{
  if (qAbs(ui->customPlot->yAxis->range().center()+value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
  {
    ui->customPlot->yAxis->setRange(-value/100.0, ui->customPlot->yAxis->range().size(), Qt::AlignCenter);
    ui->customPlot->replot();
  }
}

void MainWindow::xAxisChanged(QCPRange range)
{
  ui->horizontalScrollBar->setValue(qRound(range.center()*100.0)); // adjust position of scroll bar slider
  ui->horizontalScrollBar->setPageStep(qRound(range.size()*100.0)); // adjust size of scroll bar slider
}

void MainWindow::yAxisChanged(QCPRange range)
{
  ui->verticalScrollBar->setValue(qRound(-range.center()*100.0)); // adjust position of scroll bar slider
  ui->verticalScrollBar->setPageStep(qRound(range.size()*100.0)); // adjust size of scroll bar slider
}


void MainWindow::on_ChannelSpinBox_editingFinished()
{
    this->setChannel(ui->ChannelSpinBox->value());
    std::system(("iwconfig " + inf.toStdString() + " channel " + std::to_string(channel)).c_str());
}

int MainWindow::getChannel(){
    return this->channel;
}
