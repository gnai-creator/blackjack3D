#pragma once                                       // Inclusao unica por TU.

#include <string>
#include <vector>

// Ranking persistente em arquivo binario simples (ranking.dat ao lado do .exe).
// Mantem ate 10 entradas com 3 iniciais e a pontuacao (em coins).
class RankingSystem
{
    public:
        struct Entry
        {
            char initials[4] = {' ', ' ', ' ', '\0'};  // 3 chars + terminador (apenas conveniencia).
            int score = 0;                             // Coins acumulados na sessao.
        };

        static constexpr std::size_t MaxEntries = 10;

        RankingSystem();                               // Carrega do disco; tabela vazia se arquivo nao existir.

        bool isHighScore(int score) const;             // True se score entra no top 10.
        void addEntry(const char initials[3], int score); // Insere ordenado e mantem ate 10.

        const std::vector<Entry>& entries() const { return entries_; }

        bool save() const;                             // Grava em disco. Retorna false em caso de erro.
        bool load();                                   // Le do disco. Retorna false se arquivo ausente.

    private:
        std::vector<Entry> entries_;
        std::string filePath_;                         // Caminho absoluto/relativo ao binario.
};
