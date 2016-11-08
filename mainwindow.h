#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "openglwindow.h"

class QMenu;
class QAction;
class QLabel;
class QGroupBox;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;

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
    void CreateStatusBar();
    void CreateOptionGroup();

private:
    Ui::MainWindow *ui;
    OpenGLWindow *openglwindow_;

    // Menu
    QMenu *menu_file_;
    QAction *action_open_;
    QAction *action_exit_;
    QLabel  *label_meshinfo_;

    // Options
    QGroupBox *groupbox_options_;
    QCheckBox *check_point_;
    QCheckBox *check_edge_;
    QCheckBox *check_face_;
    QCheckBox *check_axes_;
    QCheckBox *check_aabb_;
    QCheckBox *check_light_;
    QCheckBox *check_normalize_;
    QComboBox *combobox_projection_;
    QComboBox *combobox_shade_;

    // Other options.
    QGroupBox *groupbox_others_;
    QLabel *label_roll_speed_;
    QDoubleSpinBox *spinbox_roll_speed_; // rolling speed for minbotton
};

#endif // MAINWINDOW_H
