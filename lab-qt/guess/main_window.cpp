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

void main_window::print(const QString& text)
{
    ui.listWidget->addItem(text);
    ui.listWidget->scrollToBottom();
}

void main_window::on_button_guess_clicked()
{
    do
    {
        guess_server::guess_t input_guess;
        auto text = ui.lineEdit->text();
        if (text.size() != 4)
        {
            print("Invalid input: a 4-digit number expected.");
            break;
        }
        for (size_t i = 0; i < 4; ++i)
        {
            auto c = text.at(i);
            if (!c.isDigit())
            {
                print("Invalid input: all characters must be digits.");
                break;
            }
            input_guess[i] = c.digitValue();
        }
        emit guess(input_guess);

        ui.lineEdit->clear();
    } while (false);

    ui.lineEdit->setFocus();
}
void main_window::on_button_quit_clicked() { close(); }
void main_window::on_button_new_clicked()
{
    emit new_game();
    ui.lineEdit->clear();
    ui.lineEdit->setFocus();
}
void main_window::on_lineEdit_returnPressed() { on_button_guess_clicked(); }

void main_window::_on_history_appended(const QString& item) { print(item); }
void main_window::_on_history_cleared() { ui.listWidget->clear(); }
