#pragma once                                       // Inclusao unica por TU.

// Sistema de dinheiro: saldo, aposta, melhor saldo da sessao.
// Regras: comeca com 15 coins, aposta inicial de 5; ao ganhar, aposta dobra;
// ao perder a rodada, sessao acaba e o melhor saldo vai para o ranking.
class MoneySystem
{
    public:
        static constexpr int InitialBalance = 15;  // Saldo no inicio de cada sessao.
        static constexpr int InitialBet = 5;       // Aposta inicial padrao.

        MoneySystem();                             // Construtor: estado igual a newSession().

        void newSession();                         // Reseta saldo, aposta e melhor saldo.

        int balance() const { return balance_; }   // Saldo atual.
        int bet() const { return bet_; }           // Aposta atual.
        int bestBalance() const { return best_; }  // Maior saldo atingido na sessao (= score).

        void increaseBet();                        // Dobra a aposta (limitada ao saldo).
        void decreaseBet();                        // Reduz a aposta pela metade (minimo 1).
        bool canStartRound() const;                // True se ha saldo suficiente para a aposta atual.

        void onWin();                              // Vitoria: saldo += aposta; aposta dobra; atualiza best.
        void onLose();                             // Derrota: saldo -= aposta. Sinaliza fim de sessao via isGameOver.
        void onPush();                             // Empate: nada muda.

        bool isGameOver() const { return gameOver_; } // True quando uma rodada foi perdida.

    private:
        int balance_;                              // Saldo corrente.
        int bet_;                                  // Aposta corrente.
        int best_;                                 // Melhor saldo atingido na sessao.
        bool gameOver_;                            // Vira true depois de onLose().
};
