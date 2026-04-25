#pragma once                                       // Inclusao unica por TU.

#include "ecs/entity.h"                            // Alias Entity (uint32_t).
#include "ecs/registry.h"                          // Registry para consultar componentes Card.

#include <vector>                                  // std::vector usado nas maos.

class Blackjack3DRules                             // Regras puras do blackjack expostas como funcoes estaticas.
{
    public:

        // Calcula a pontuacao da mao tratando Ases como 11 ou 1 conforme convenha.
        static int calculateScore(const std::vector<Entity>& hand, const Registry& registry);

        // Conveniencia: retorna true se a pontuacao passa de 21.
        static bool isBust(const std::vector<Entity>& hand, const Registry& registry);

    private:

        // Valor numerico de uma unica carta (As=11 por padrao; figuras=10).
        static int cardValue(const blackjack3D::Card& card);
};
