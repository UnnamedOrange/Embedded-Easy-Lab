#pragma once

#include <QMainWindow>

#include "ui_main_window.h"

class main_window : public QMainWindow
{
    Q_OBJECT

private:
    Ui::main_window ui;

public:
    main_window(QWidget* parent = nullptr);
};
