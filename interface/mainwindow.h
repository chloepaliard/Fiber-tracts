#pragma once

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "widgetopengl.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    /** Quit the application */
    void action_quit();
    /** Set the Wireframe mode for the meshes */

    void action_slider();
    void action_color_mode();
    
    // On appelle la fonction de scene qui enlève les outliers
    void action_outliers();

    void keyPressEvent(QKeyEvent *event);

private:

    /** Layout for the Window */
    Ui::MainWindow *ui;
    /** The OpenGL Widget */
    WidgetOpenGL *glWidget;
};

#endif // MAINWINDOW_H
