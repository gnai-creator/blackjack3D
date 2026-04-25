#include "game/blackjack_rules.h"                                   // Header com declaracoes da classe.

int Blackjack3DRules::calculateScore(const std::vector<Entity>& hand, const Registry& registry)
{
    int total = 0;                                                  // Soma corrente da mao.
    int aces = 0;                                                   // Quantos Ases foram contados como 11.

    for (Entity entity : hand)                                      // Itera cada carta da mao por ID.
    {
        const blackjack3D::Card* card = registry.getCard(entity);   // Busca o componente Card no Registry.
        if (!card)                                                  // Entidade pode nao ter Card (defensivo).
        {
            continue;                                               // Ignora silenciosamente entidades invalidas.
        }

        if (card->rank == 1)                                        // rank 1 = As; precisamos lembrar para downgrade.
        {
            ++aces;                                                 // Conta para tentar virar 1 depois, se necessario.
        }

        total += cardValue(*card);                                  // ATENCAO: cardValue() esta DECLARADA mas nao DEFINIDA.
    }

    // cada As comeca valendo 11. Se estourar, trasforma As em 1.

    while (total > 21 && aces > 0)                                  // Enquanto estourado e ainda houver As "alto"...
    {
        total -= 10;                                                // ...rebaixa um As de 11 para 1 (diferenca = 10).
        --aces;                                                     // Marca o As como ja convertido.
    }

    return total;                                                   // Pontuacao final da mao.
}


bool Blackjack3DRules::isBust(const std::vector<Entity>& hand, const Registry& registry)
{
    return calculateScore(hand, registry) > 21;                     // Estoura se passar de 21.
}

int Blackjack3DRules::cardValue(const blackjack3D::Card& card)
{
    if (card.rank == 1)                                             // As: comeca como 11; calculateScore rebaixa para 1 se estourar.
    {
        return 11;
    }

    if (card.rank >= 10)                                            // 10, J(11), Q(12), K(13) valem 10.
    {
        return 10;
    }

    return card.rank;                                               // 2..9 valem o proprio numero.
}
