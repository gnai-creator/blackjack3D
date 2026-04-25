#pragma once                                       // Inclusao unica por TU.

#include "ecs/registry.h"                          // Registry passado a drawScene.

#include "raylib.h"                                // Texture2D, Rectangle.

class RenderSystem                                 // Sistema de desenho 3D (mesa + cartas).
{
    public:
        RenderSystem();                            // Carrega textura do baralho se existir.
        ~RenderSystem();                           // Libera a textura (se carregada).

        RenderSystem(const RenderSystem&) = delete;            // Proibe copia: textura tem ownership unico.
        RenderSystem& operator = (const RenderSystem&) = delete; // Idem para atribuicao por copia.

        void drawScene(const Registry& registry) const;        // Ponto de entrada chamado dentro de BeginMode3D.

    private:
        Rectangle sourceForCard(const blackjack3D::Card& card) const;          // Calcula a regiao da textura a desenhar.
        void drawTexturedCard(const blackjack3D::Transform& transform,         // Desenha a carta como quad texturizado.
                              const blackjack3D::Renderable& renderable,
                              Rectangle source) const;
        void drawTable() const;                                                // Desenha o tampo verde da mesa.
        void drawCards(const Registry& registry) const;                        // Itera o registry e desenha cada carta.

        Texture2D deckTexture{};                   // Atlas com todas as cartas (53x70 px por celula).
        bool hasDeckTexture = false;               // Sinaliza se o construtor conseguiu carregar a textura.

};
