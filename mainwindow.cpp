#include "common.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenu>
#include <QAction>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    openglwindow_ = new OpenGLWindow(this);
    CreateActions();
    CreateMenus();

    setCentralWidget(openglwindow_);
}

MainWindow::~MainWindow()
{
    delete ui;
    SafeDelete(menu_file_);
    SafeDelete(action_open_);
    SafeDelete(action_exit_);
//    if (openglwindow_) {
//        delete openglwindow_;
//        openglwindow_ = Q_NULLPTR;
//    }
}

void MainWindow::CreateActions() {
    action_open_ = new QAction(tr("Open"), this);
    action_open_->setShortcut(QKeySequence::Open);
    action_open_->setStatusTip(tr("Open an existing mesh (Current only m-file is supported)."));
//    connect(action_open_, SIGNAL(triggered(bool)), openglwindow_, SLOT(ReadMesh()));
    action_exit_ = new QAction(tr("Exit"), this);
    action_exit_->setShortcut(QKeySequence::Quit);
    connect(action_exit_, SIGNAL(triggered(bool)), this, SLOT(close()));
}

void MainWindow::CreateMenus() {
    menu_file_ = menuBar()->addMenu(tr("&File"));
    menu_file_->addAction(action_open_);
    menu_file_->addAction(action_exit_);
}
