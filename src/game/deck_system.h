#pragma once                                       // Inclusao unica por TU.

#include "ecs/entity.h"                            // Alias Entity.
#include "ecs/registry.h"                          // Registry referenciado por &.

#include <optional>                                // std::optional<Entity> usado em drawTop.
#include <random>                                  // std::mt19937 usado para embaralhar.
#include <vector>                                  // Vetor que guarda as cartas restantes.

class DeckSystem                                   // Sistema responsavel por criar, embaralhar e distribuir cartas.
{
    public:

        explicit DeckSystem(Registry& registry);   // explicit evita conversao implicita; guarda referencia ao Registry.

        void createDeck();                         // Cria as 52 entidades-carta e popula o vetor 'deck'.
        void shuffle();                            // Embaralha o vetor 'deck' com mt19937.
        std::optional<Entity> drawTop();           // Saca a carta do topo (back do vetor); nullopt se vazio.


    private:
        static Color colorForSuit(int suit);       // Mapeia naipe -> cor (vermelho/preto). Util como fallback visual.

        Registry& registry;                        // Referencia para criar entidades; nao possui ownership.
        std::vector<Entity> deck;                  // Pilha de cartas restantes (topo no fim do vetor).
        std::mt19937 rng;                          // Mersenne Twister; bom equilibrio entre velocidade e qualidade.
};
