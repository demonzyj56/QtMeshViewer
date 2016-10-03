#include "common.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    openglwindow_ = new OpenGLWindow(this);
    setGeometry(300, 150, 800, 600);
    CreateActions();
    CreateMenus();
    CreateStatusBar();
    CreateOptionGroup();

    QVBoxLayout *layout_options_ = new QVBoxLayout;
    layout_options_->addWidget(groupbox_options_);
    layout_options_->addStretch();
    QHBoxLayout *layout_main_ = new QHBoxLayout;
    layout_main_->addWidget(openglwindow_, 1);
    layout_main_->addLayout(layout_options_);
    this->centralWidget()->setLayout(layout_main_);

}

MainWindow::~MainWindow()
{
    delete ui;
//    SafeDelete(menu_file_);
//    SafeDelete(action_open_);
//    SafeDelete(action_exit_);
}

void MainWindow::CreateActions() {
    action_open_ = new QAction(tr("Open"), this);
    action_open_->setShortcut(QKeySequence::Open);
    action_open_->setStatusTip(tr("Open an existing mesh (Current only m-file is supported)."));
    connect(action_open_, SIGNAL(triggered(bool)), openglwindow_, SLOT(ReadMesh()));
    action_exit_ = new QAction(tr("Exit"), this);
    action_exit_->setShortcut(QKeySequence::Quit);
    connect(action_exit_, SIGNAL(triggered(bool)), this, SLOT(close()));
}

void MainWindow::CreateMenus() {
    menu_file_ = menuBar()->addMenu(tr("&File"));
    menu_file_->addAction(action_open_);
    menu_file_->addAction(action_exit_);
}

void MainWindow::CreateStatusBar() {
    label_meshinfo_ = new QLabel();
    statusBar()->addWidget(label_meshinfo_);
    connect(openglwindow_, SIGNAL(operatorInfo(QString)), label_meshinfo_, SLOT(setText(QString)));
}

void MainWindow::CreateOptionGroup() {
    check_point_ = new QCheckBox(tr("Point"), this);
    connect(check_point_, SIGNAL(clicked(bool)), openglwindow_, SLOT(SetDrawPoints(bool)));
    check_point_->setChecked(true);
    check_edge_ = new QCheckBox(tr("Edge"), this);
    connect(check_edge_, SIGNAL(clicked(bool)), openglwindow_, SLOT(SetDrawEdges(bool)));
    check_edge_->setChecked(true);
    check_face_ = new QCheckBox(tr("Face"), this);
    connect(check_face_, SIGNAL(clicked(bool)), openglwindow_, SLOT(SetDrawFaces(bool)));
    check_face_->setChecked(true);
    check_axes_ = new QCheckBox(tr("Axes"), this);
    connect(check_axes_, SIGNAL(clicked(bool)), openglwindow_, SLOT(SetDrawAxes(bool)));
    check_axes_->setChecked(true);
    check_aabb_ = new QCheckBox(tr("AABB"), this);
    connect(check_aabb_, SIGNAL(clicked(bool)), openglwindow_, SLOT(SetDrawBoundingBox(bool)));
    check_aabb_->setChecked(false);
    check_light_ = new QCheckBox(tr("Lighting"), this);
    connect(check_light_, SIGNAL(clicked(bool)), openglwindow_, SLOT(SetDrawLighting(bool)));
    check_light_->setChecked(true);

    groupbox_options_ = new QGroupBox(tr("Options"), this);
    QVBoxLayout *options_layout_ = new QVBoxLayout(groupbox_options_);
    options_layout_->addWidget(check_point_);
    options_layout_->addWidget(check_edge_);
    options_layout_->addWidget(check_face_);
    options_layout_->addWidget(check_axes_);
    options_layout_->addWidget(check_aabb_);
    options_layout_->addWidget(check_light_);
}
