#include "game/money_system.h"

MoneySystem::MoneySystem()
{
    newSession();                                  // Estado inicial = sessao nova.
}

void MoneySystem::newSession()
{
    balance_ = InitialBalance;                     // 15 coins.
    bet_ = InitialBet;                             // 5 coins.
    best_ = InitialBalance;                        // O proprio saldo inicial.
    gameOver_ = false;
}

void MoneySystem::increaseBet()
{
    int next = bet_ * 2;                           // Dobra...
    if (next > balance_) next = balance_;          // ...limitado ao saldo.
    if (next < 1) next = 1;
    bet_ = next;
}

void MoneySystem::decreaseBet()
{
    int next = bet_ / 2;                           // Metade...
    if (next < 1) next = 1;                        // ...minimo 1 coin.
    bet_ = next;
}

bool MoneySystem::canStartRound() const
{
    return !gameOver_ && bet_ > 0 && bet_ <= balance_;
}

void MoneySystem::onWin()
{
    balance_ += bet_;                              // Recebe o valor da aposta como premio (1:1).
    if (balance_ > best_) best_ = balance_;        // Atualiza recorde da sessao.

    int next = bet_ * 2;                           // Aposta dobra automaticamente para a proxima rodada.
    if (next > balance_) next = balance_;
    bet_ = next;
}

void MoneySystem::onLose()
{
    balance_ -= bet_;                              // Perde a aposta.
    if (balance_ < 0) balance_ = 0;
    gameOver_ = true;                              // Qualquer derrota encerra a sessao.
}

void MoneySystem::onPush()
{
    // Empate: ninguem ganha nem perde; aposta tambem nao muda.
}
