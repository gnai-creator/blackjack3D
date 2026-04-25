#include "game/deck_system.h"

#include <algorithm>

namespace
{
    // Posicao base do baralho visual (canto superior-esquerdo da mesa).
    constexpr float DeckBaseX = -3.0f;
    constexpr float DeckBaseY = 0.05f;
    constexpr float DeckBaseZ = -1.8f;
    constexpr float DeckStackStep = 0.0015f;       // Pequeno offset Y por carta para criar ilusao de pilha.
}

DeckSystem::DeckSystem(Registry& registryRef) : registry(registryRef), rng(std::random_device{}())
{
}

void DeckSystem::createDeck()
{
    deck.clear();

    int index = 0;                                                 // Posicao na pilha (cresce conforme empilha).
    for (int suit = 0; suit < 4; ++suit)
    {
        for (int rank = 1; rank <= 13; ++rank)
        {
            Entity cardEntity = registry.createEntity();

            registry.addCard(cardEntity, blackjack3D::Card{rank, suit, false});  // Comeca de costas (e' o baralho).
            registry.addTransform(cardEntity, blackjack3D::Transform{
                Vector3{DeckBaseX, DeckBaseY + index * DeckStackStep, DeckBaseZ}, // Empilhada visualmente.
                Vector3{0.0f, 0.0f, 0.0f},
                Vector3{1.0f, 1.0f, 1.0f}
            });
            registry.addRenderable(cardEntity, blackjack3D::Renderable{
                colorForSuit(suit),
                Vector3{0.78f, 0.01f, 1.04f}                       // Aspecto 48:64 da textura.
            });

            deck.push_back(cardEntity);
            ++index;
        }
    }
}

void DeckSystem::shuffle()
{
    std::shuffle(deck.begin(), deck.end(), rng);
}

std::optional<Entity> DeckSystem::drawTop()
{
    if (deck.empty())
    {
        return std::nullopt;
    }

    Entity entity = deck.back();
    deck.pop_back();
    return entity;
}

Color DeckSystem::colorForSuit(int suit)
{
    return (suit == 0 || suit == 1) ? RED: BLACK;
}
