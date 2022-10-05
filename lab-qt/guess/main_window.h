#pragma once

#include <QMainWindow>

#include "guess_server.hpp"

#include "ui_main_window.h"

class main_window : public QMainWindow
{
    Q_OBJECT

private:
    Ui::main_window ui;

private:
    guess_server server;

public:
    main_window(QWidget* parent = nullptr);

private:
    void print(const QString& text);

private slots:
    void on_button_guess_clicked();
    void on_button_new_clicked();
    void on_button_quit_clicked();
    void on_lineEdit_returnPressed();

private slots:
    void _on_history_appended(const QString& item);
    void _on_history_cleared();

signals:
    void new_game();
    void guess(const guess_server::guess_t& guess);
};
