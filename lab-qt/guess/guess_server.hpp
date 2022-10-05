#pragma once

#include <array>
#include <random>
#include <utility>
#include <vector>

#include <QObject>
#include <QString>

class guess_server : public QObject
{
    Q_OBJECT

public:
    using guess_t = std::array<int, 4>;

private:
    guess_t answer;
    std::vector<QString> history;

public:
    explicit guess_server(QObject* parent = nullptr) : QObject(parent)
    {
        _on_new_game();
    }

private:
    /**
     * @brief Returns random numbers.
     */
    static guess_t generate_answer()
    {
        guess_t ret;
        std::random_device rd;
        std::default_random_engine engine(rd());
        std::uniform_int_distribution<int> dist(0, 9);
        for (auto& digit : ret)
            digit = dist(engine);
        return ret;
    }
    static std::pair<int, int> judge(const guess_t& std_guess,
                                     const guess_t& input_guess)
    {
        int A = 0; // Correct digit and position.
        int B = 0; // Correct number but wrong position.
        {
            for (size_t i = 0; i < std_guess.size(); ++i)
                if (std_guess[i] == input_guess[i])
                    A++;
        }
        {
            for (int i = 0; i < 10; i++)
            {
                size_t count_std = 0;
                size_t count_input = 0;
                for (size_t j = 0; j < std_guess.size(); ++j)
                {
                    if (std_guess[j] == i)
                        count_std++;
                    if (input_guess[j] == i)
                        count_input++;
                }
                B += std::min(count_std, count_input);
            }
        }
        return {A, B};
    }

public slots:
    void _on_guess(const guess_t& trial)
    {
        auto result = judge(answer, trial);
        QString result_str = QString::number(result.first) + "A" +
                             QString::number(result.second) + "B";
        history.push_back(result_str);
        emit history_appended(result_str);
    }
    void _on_new_game()
    {
        answer = generate_answer();
        history.clear();
        emit history_cleared();
    }

signals:
    void history_cleared();
    void history_appended(const QString& item);
};
