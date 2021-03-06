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
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QStringList>

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
    layout_options_->addWidget(groupbox_others_);
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
    check_normalize_ = new QCheckBox(tr("Normalize"), this);
    connect(check_normalize_, SIGNAL(clicked(bool)), openglwindow_, SLOT(SetNormalized(bool)));
    check_normalize_->setChecked(false);
    combobox_projection_ = new QComboBox(this);
    combobox_projection_->addItem("Perspective Projection");
    combobox_projection_->addItem("Orthogonal Projection");
    connect(combobox_projection_, SIGNAL(activated(int)), openglwindow_, SLOT(SetProjectionMode(int)));
    combobox_shade_ = new QComboBox(this);
    combobox_shade_->addItem("Smooth Shading");
    combobox_shade_->addItem("Flat Shading");
    connect(combobox_shade_, SIGNAL(activated(int)), openglwindow_, SLOT(SetShadeMode(int)));
    label_roll_speed_ = new QLabel(tr("Roll Speed"), this);
    spinbox_roll_speed_ = new QDoubleSpinBox(this);
    spinbox_roll_speed_->setRange(0.001, 100.);
    spinbox_roll_speed_->setSingleStep(0.003);
    spinbox_roll_speed_->setDecimals(3);
    connect(spinbox_roll_speed_, SIGNAL(valueChanged(double)), openglwindow_, SLOT(SetRollSpeed(double)));
    static QStringList mats_name;
    mats_name << "emerald" << "jade" << "obsidian" << "pearl" << "ruby" << "turquoise" << "brass"
              << "bronze" << "chrome" << "copper" << "gold" << "silver";
    label_material_ = new QLabel(tr("Materials"), this);
    combobox_material_ = new QComboBox(this);
    combobox_material_->addItems(mats_name);
    connect(combobox_material_, SIGNAL(activated(QString)), openglwindow_, SLOT(SetMaterial(QString)));
    label_light_intensity_ = new QLabel(tr("Light Intensity"), this);
    spinbox_light_intensity_ = new QDoubleSpinBox(this);
    spinbox_light_intensity_->setRange(0., 1.);
    spinbox_light_intensity_->setSingleStep(0.05);
    spinbox_light_intensity_->setValue(1.0);
    connect(spinbox_light_intensity_, SIGNAL(valueChanged(double)), openglwindow_, SLOT(SetLightIntensity(double)));

    groupbox_options_ = new QGroupBox(tr("Options"), this);
    QVBoxLayout *options_layout_ = new QVBoxLayout(groupbox_options_);
    options_layout_->addWidget(check_point_);
    options_layout_->addWidget(check_edge_);
    options_layout_->addWidget(check_face_);
    options_layout_->addWidget(check_axes_);
    options_layout_->addWidget(check_aabb_);
    options_layout_->addWidget(check_light_);
    options_layout_->addWidget(check_normalize_);
    options_layout_->addWidget(combobox_projection_);
    options_layout_->addWidget(combobox_shade_);

    groupbox_others_ = new QGroupBox(tr("Others"), this);
    QVBoxLayout *others_layout_ = new QVBoxLayout(groupbox_others_);
    others_layout_->addWidget(label_roll_speed_);
    others_layout_->addWidget(spinbox_roll_speed_);
    others_layout_->addWidget(label_light_intensity_);
    others_layout_->addWidget(spinbox_light_intensity_);
    others_layout_->addWidget(label_material_);
    others_layout_->addWidget(combobox_material_);
}
