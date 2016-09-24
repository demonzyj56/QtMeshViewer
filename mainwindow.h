#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "openglwindow.h"

class QMenu;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void CreateActions();
    void CreateMenus();

private:
    Ui::MainWindow *ui;
    OpenGLWindow *openglwindow_;

    // Manu
    QMenu *menu_file_;
    QAction *action_open_;
    QAction *action_exit_;
};

#endif // MAINWINDOW_H
