#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTimer>
#include <QVector>



QSerialPort* mySerialPort = new QSerialPort;
QString dataOut ;

int left_encoder =0;
int right_encoder = 0 ;
int pre_left_encoder = 0;
int pre_right_encoder = 0;
//QVector<double> time_X = {0}, VR_data ={0} ,VL_data{0};
QVector <double> X_data = {0.00}, Y_data = {0.00},Phi_data = {0.00};
int status_robot = 0;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    this->setWindowTitle("Serial Control");

    // set comport in laptop appear on combox_Port_Name
    foreach(auto &portInfo, QSerialPortInfo::availablePorts()){
        ui->combox_Port_Name->addItem(portInfo.portName());
    }
    // add Baudrate_value into combox_Baudrate

    QStringList Baudrate_list = {"115200","9600","4800","2400","1200"};
    ui->combox_Baudrate->addItems(Baudrate_list);
    // add databits into comBox_Data_Bits
    QStringList databits_list = {"8","7","6"};
    ui->comBox_Data_Bits->addItems(databits_list);
    // add stopbits into combox_Stop_Bits
    QStringList stopbits_list = {"One","Two"};
    ui->combox_Stop_Bits->addItems(stopbits_list);
    // add Paritybits into combox_Stopbit
    QStringList Parity_list ={"None","Odd","Even"};
    ui->combox_Parity_bits->addItems(Parity_list);
    //enable some button
    ui->btnconnect->setEnabled(true);
    ui->btn_Close->setEnabled(false);
    ui->lbl_Status_Comport->setText("OFF");
    ui->lbl_Status_Comport->setStyleSheet("color: black;background-color: grey;");
    //config graph
    ui->Plot_chart->addGraph();
//    ui->Plot_chart->addGraph();

    ui->Plot_chart->graph(0)->setScatterStyle(QCPScatterStyle::ssDot);
    ui->Plot_chart->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->Plot_chart->graph(0)->setPen(QPen(Qt::blue));

//    ui->Plot_chart->addGraph();
//    ui->Plot_chart->graph(1)->setScatterStyle(QCPScatterStyle::ssDot);
//    ui->Plot_chart->graph(1)->setLineStyle(QCPGraph::lsLine);
//    ui->Plot_chart->graph(1)->setPen(QPen(Qt::red));

    ui->Plot_chart->xAxis->setLabel("X(m)");
    ui->Plot_chart->yAxis->setLabel("Y(m)");
    ui->Plot_chart->xAxis->setRange(0,1000);
    ui->Plot_chart->yAxis->setRange(-100,100);
    ui->Plot_chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);


//    qDebug()<<titleWindow;
}

MainWindow::~MainWindow()
{

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event){
    QMessageBox closemsgBox(QMessageBox::Question,"Question","Do You Want To Exit???",QMessageBox::Yes| QMessageBox::No);
    closemsgBox.setDefaultButton(QMessageBox::No);
    int ret = closemsgBox.exec();
    if(ret ==QMessageBox::No){
        event->ignore();
    }
}


void MainWindow::on_btnconnect_clicked() {
//    ui->list_data->addItem("iam TXN");

//    if(mySerialPort == nullptr){
//        mySerialPort->close();
//        delete mySerialPort;
//    }
    // config serial port with data took from comboxs
    mySerialPort = new QSerialPort(this);
    mySerialPort->setPortName(ui->combox_Port_Name->currentText());
    mySerialPort->setBaudRate(ui->combox_Baudrate->currentText().toInt());
    mySerialPort->setDataBits(static_cast<QSerialPort::DataBits>(ui->comBox_Data_Bits->currentText().toInt()));
    mySerialPort->setStopBits(static_cast<QSerialPort::StopBits>(ui->combox_Stop_Bits->currentText().toInt()));
    mySerialPort->setParity(static_cast<QSerialPort::Parity>(ui->combox_Parity_bits->currentText().toInt()));

    mySerialPort->open(QIODevice::ReadWrite);
    connect(mySerialPort, SIGNAL(readyRead()), this, SLOT(Serial_DataReceived()));
    if(mySerialPort->isOpen()){
        ui->btnconnect->setEnabled(false);
        ui->btn_Close->setEnabled(true);
        ui->lbl_Status_Comport->setText("ON");
        ui->lbl_Status_Comport->setStyleSheet("color: black;background-color: green;");
    }
    else{
       ui->btnconnect->setEnabled(false);
       ui->btn_Close->setEnabled(true);
       ui->lbl_Status_Comport->setText("OFF");
       ui->lbl_Status_Comport->setStyleSheet("color: black;background-color: grey;");
       QMessageBox::warning(this,"Warning","Port is not conected!!");
    }

}

void MainWindow::Mytimer_timeout(){

}

void MainWindow::on_btn_Close_clicked()
{
    if(mySerialPort->isOpen()){
        mySerialPort->close();
        ui->btnconnect->setEnabled(true);
        ui->btn_Close->setEnabled(false);
        ui->lbl_Status_Comport->setText("OFF");
        ui->lbl_Status_Comport->setStyleSheet("color: black;background-color: grey;");
    }
}


void MainWindow::on_btnSend_Data_clicked()
{
    if(mySerialPort->isOpen()){
        dataOut = ui->txtEdit_Send_Data->toPlainText() + "\n";
        mySerialPort->write(dataOut.toUtf8());
        ui->listWidget_data_display->addItem("DATA SEND:");
        ui->listWidget_data_display->addItem(dataOut.trimmed()); //cut off '\n'

    }else{
        QMessageBox::warning(this,"Warning","Port is not conected!!");
    }
}

void MainWindow::on_btnClear_Send_Data_clicked()
{
    ui->txtEdit_Send_Data->clear();
}
void MainWindow::Serial_DataReceived()
{
// If you don't want it to be colorful, just use this code below. :VV;
//    ui->listWidget_data_display->addItem("DATA RECIEVE:");
//    ui->listWidget_data_display->addItem(mySerialPort->readAll());
//   !cmd:RUN#x:0.00#y:0.00#phi:45.00#\n
    QString data_receive = mySerialPort->readAll();

    int split_indexOfCmdStart = data_receive.indexOf(":");
    int split_indexOfCmdEnd = data_receive.indexOf("#",split_indexOfCmdStart);

    int split_indexOfXStart = data_receive.indexOf(":",split_indexOfCmdEnd+1);
    int split_indexOfXEnd = data_receive.indexOf("#",split_indexOfXStart+1);

    int split_indexOfYStart = data_receive.indexOf(":",split_indexOfXEnd+1);
    int split_indexOfYEnd = data_receive.indexOf("#",split_indexOfYStart+1);

    int split_indexOfPhiStart = data_receive.indexOf(":",split_indexOfYEnd+1);
    int split_indexOfPhiEnd = data_receive.indexOf("#",split_indexOfPhiStart+1);


    if (split_indexOfCmdStart != -1 && split_indexOfCmdEnd != -1 && split_indexOfXStart != -1 && split_indexOfXStart != -1
            && split_indexOfYStart != -1 && split_indexOfYEnd != -1 && split_indexOfPhiStart != -1 && split_indexOfPhiEnd!= -1 ) {
        // Lấy phần tử con chuỗi từ sau ký tự ':' đến trước ký tự '#'
        QString cmd_value_string = data_receive.mid(split_indexOfCmdStart + 1,split_indexOfCmdEnd - split_indexOfCmdStart - 1);
        QString X_value_string = data_receive.mid(split_indexOfXStart + 1,split_indexOfXEnd - split_indexOfXStart - 1);
        QString Y_value_string = data_receive.mid(split_indexOfYStart + 1,split_indexOfYEnd - split_indexOfYStart - 1);
        QString Phi_value_string = data_receive.mid(split_indexOfPhiStart + 1,split_indexOfPhiEnd - split_indexOfPhiStart - 1);



//        double cmd_value = cmd_value_string.toDouble();
        if(cmd_value_string == "RUN"){
            status_robot =1 ;
        }
        else{
            status_robot = 0;
        }
        double X_value = X_value_string.toDouble();
        double Y_value = Y_value_string.toDouble();
        double Phi_value = Phi_value_string.toDouble();
        // In ra giá trị đã đọc được
        qDebug()<<"status robot: = "<<status_robot;
        qDebug()<<"X: "<<X_value;
        qDebug()<<"Y:  "<<Y_value;
        qDebug()<<"Phi:  "<<Phi_value;
        X_data.append(X_value);
        Y_data.append(Y_value);
        Phi_data.append(Phi_value);

    }
    else {
        qDebug() << "Không tìm thấy ký tự phân tách trong chuỗi.";
    }
    // add to lsist



    QListWidgetItem *item_recieve1 = new QListWidgetItem("DATA RECIEVE:");
    item_recieve1->setForeground(QBrush(Qt::darkGreen)); // Change color here
    ui->listWidget_data_display->addItem(item_recieve1);

    QListWidgetItem *item_recieve2 = new QListWidgetItem(data_receive.trimmed());
    item_recieve2->setForeground(QBrush(Qt::darkGreen)); // Change color here
    ui->listWidget_data_display->addItem(item_recieve2);

    QListWidgetItem *item_recieve3 = new QListWidgetItem("cmd:"+QString::number(status_robot));
    item_recieve3->setForeground(QBrush(Qt::darkRed)); // Change color here
    ui->listWidget_data_display->addItem(item_recieve3);

    QListWidgetItem *item_recieve4 = new QListWidgetItem("X:"+QString::number(X_data.last()));
    item_recieve4->setForeground(QBrush(Qt::darkRed)); // Change color here
    ui->listWidget_data_display->addItem(item_recieve4);

    QListWidgetItem *item_recieve5 = new QListWidgetItem("Y:"+QString::number(Y_data.last()));
    item_recieve5->setForeground(QBrush(Qt::darkRed)); // Change color here
    ui->listWidget_data_display->addItem(item_recieve5);

    QListWidgetItem *item_recieve6 = new QListWidgetItem("Phi: "+QString::number(Phi_data.last()));
    item_recieve6->setForeground(QBrush(Qt::darkRed)); // Change color here
    ui->listWidget_data_display->addItem(item_recieve6);
    // plot graph


    ui->Plot_chart->graph(0)->setData(X_data,Y_data);
//    ui->Plot_chart->graph(1)->setData(time_X,VR_data);
    ui->Plot_chart->rescaleAxes();
    ui->Plot_chart->replot();
    ui->Plot_chart->update();


}


void MainWindow::on_btnClearData_Display_clicked()
{
   ui->listWidget_data_display->clear();
}


void MainWindow::on_btnClearData_plot_clicked()
{
    X_data.clear();
    Y_data.clear();
    Phi_data.clear();
    ui->Plot_chart->graph(0)->data()->clear();
//    ui->Plot_chart->graph(1)->data()->clear();
//    ui->Plot_chart->rescaleAxes();
    ui->Plot_chart->replot();
    ui->Plot_chart->update();
}

