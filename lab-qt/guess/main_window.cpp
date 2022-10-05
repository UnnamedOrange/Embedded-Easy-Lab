#include "main_window.h"

main_window::main_window(QWidget* parent) : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(this, &main_window::new_game, &server, &guess_server::_on_new_game);
    connect(this, &main_window::guess, &server, &guess_server::_on_guess);
    connect(&server, &guess_server::history_appended, this,
            &main_window::_on_history_appended);
    connect(&server, &guess_server::history_cleared, this,
            &main_window::_on_history_cleared);
}

void main_window::on_button_guess_clicked()
{
    guess_server::guess_t input_guess;
    auto text = ui.lineEdit->text();
    if (text.size() != 4)
    {
        // TODO: Show error message.
        return;
    }
    for (size_t i = 0; i < 4; ++i)
    {
        auto c = text.at(i);
        if (!c.isDigit())
        {
            // TODO: Show error message.
            return;
        }
        input_guess[i] = c.digitValue();
    }
    emit guess(input_guess);
}
void main_window::on_button_quit_clicked() { close(); }
void main_window::on_button_new_clicked() { emit new_game(); }

void main_window::_on_history_appended(const QString& item)
{
    ui.listWidget->addItem(item);
}
void main_window::_on_history_cleared() { ui.listWidget->clear(); }
