#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    //Setup window layout
    ui->setupUi(this);

    this->resize(1920*2/3,1280*2/3);

    //Create openGL context
    QGLFormat qglFormat;
    //qglFormat.setVersion(1,2);
    qglFormat.setVersion(4,5);
    qglFormat.setSampleBuffers(true);
    //Create OpenGL Widget renderer
    glWidget=new WidgetOpenGL(qglFormat);

    //Add the OpenGL Widget into the layout
    ui->layout_scene->addWidget(glWidget);

    //Connect slot and signals
    connect(ui->quit,SIGNAL(clicked()),this,SLOT(action_quit()));
    connect(ui->slider,SIGNAL(sliderMoved(int)), this, SLOT(action_slider()));
    connect(ui->ColorMode,SIGNAL(clicked()), this, SLOT(action_color_mode()));
    connect(ui->Outliers,SIGNAL(clicked()), this, SLOT(action_outliers()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::action_quit()
{
    close();
}


void MainWindow::action_slider()
{
    int const value = ui->slider->value();
    glWidget->get_scene().slider_set_value((float)value/100);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int current=event->key();
    if (current==Qt::Key_Right)
    {
        string textToPrint="Right Key Pressed";
        ui->informations->setText(QString::fromStdString(textToPrint));
    }
    if (current==Qt::Key_Left)
    {
        string textToPrint="Left Key Pressed";
        ui->informations->setText(QString::fromStdString(textToPrint));
    }
}

void MainWindow::action_color_mode()
{
    bool const state = ui->ColorMode->isChecked();
    glWidget->get_scene().changeColorState(state);
}

// On appelle la fonction de scene qui enlève les outliers
void MainWindow::action_outliers()
{
  bool const state = ui->Outliers->isChecked();
  glWidget->get_scene().getRidOfOutliers(state,2);
}
