#include "game/ranking_system.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace
{
    constexpr const char *FileName = "ranking.dat";    // Caminho relativo ao CWD do .exe.
    constexpr int FileMagic = 0x424A4B30;              // 'BJK0' como assinatura simples.
}

RankingSystem::RankingSystem()
{
    filePath_ = FileName;                              // Salva ao lado do executavel.
    load();                                            // Tabela vazia se arquivo ainda nao existe.
}

bool RankingSystem::isHighScore(int score) const
{
    if (score <= 0) return false;                      // Nao registra zero.
    if (entries_.size() < MaxEntries) return true;     // Ha vaga livre.
    return score > entries_.back().score;              // Ultima posicao e a menor; basta vencer ela.
}

void RankingSystem::addEntry(const char initials[3], int score)
{
    Entry e;
    for (int i = 0; i < 3; ++i)
    {
        e.initials[i] = (initials[i] >= ' ' && initials[i] <= '~') ? initials[i] : ' ';
    }
    e.initials[3] = '\0';
    e.score = score;

    entries_.push_back(e);
    std::sort(entries_.begin(), entries_.end(),
              [](const Entry& a, const Entry& b) { return a.score > b.score; });
    if (entries_.size() > MaxEntries)
    {
        entries_.resize(MaxEntries);                   // Mantem apenas o topo.
    }
}

bool RankingSystem::save() const
{
    FILE* f = std::fopen(filePath_.c_str(), "wb");
    if (!f) return false;

    int magic = FileMagic;
    std::fwrite(&magic, sizeof(magic), 1, f);          // Cabecalho para detectar arquivo invalido.

    std::uint32_t count = static_cast<std::uint32_t>(entries_.size());
    std::fwrite(&count, sizeof(count), 1, f);

    for (const auto& e : entries_)
    {
        std::fwrite(e.initials, 1, 4, f);              // 3 iniciais + 1 byte de padding.
        std::fwrite(&e.score, sizeof(e.score), 1, f);
    }

    std::fclose(f);
    return true;
}

bool RankingSystem::load()
{
    entries_.clear();

    FILE* f = std::fopen(filePath_.c_str(), "rb");
    if (!f) return false;                              // Sem arquivo ainda: ranking comeca vazio.

    int magic = 0;
    if (std::fread(&magic, sizeof(magic), 1, f) != 1 || magic != FileMagic)
    {
        std::fclose(f);
        return false;                                  // Arquivo corrompido/de outra versao: ignora.
    }

    std::uint32_t count = 0;
    if (std::fread(&count, sizeof(count), 1, f) != 1)
    {
        std::fclose(f);
        return false;
    }
    if (count > MaxEntries) count = MaxEntries;        // Defensivo.

    for (std::uint32_t i = 0; i < count; ++i)
    {
        Entry e;
        if (std::fread(e.initials, 1, 4, f) != 4) break;
        if (std::fread(&e.score, sizeof(e.score), 1, f) != 1) break;
        e.initials[3] = '\0';
        entries_.push_back(e);
    }

    std::fclose(f);
    return true;
}
