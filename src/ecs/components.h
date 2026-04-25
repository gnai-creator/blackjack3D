#pragma once                                       // Inclusao unica por unidade de traducao.

#include "raylib.h"                                // Necessario por causa de Vector3 e Color usados abaixo.

namespace blackjack3D                              // Namespace evita colisao com tipos do raylib (ex.: Transform).
{
    // Posicao, rotacao e escala basicas para objetos 3D.
    struct Transform                               // Componente de transformacao espacial de uma entidade.
    {
        Vector3 position{0.0f, 0.0f, 0.0f};        // Posicao no mundo (x,y,z).
        Vector3 rotation{0.0f, 0.0f, 0.0f};        // Rotacao em GRAUS por eixo (X usado para flip da carta).
        Vector3 scale{1.0f, 1.0f, 1.0f};           // Escala por eixo; 1.0 mantem o tamanho natural.
    };

    // Carta de baralho. rank: 1 = As, 11 = J, 12 = Q, 13 = K.
    // suit: 0 = copas, 1 = ouros, 2 = paus, 3 = espadas.
    struct Card                                    // Componente de dados da carta.
    {
        int rank = 1;                              // Valor da carta (1 a 13). 1 = As por convencao.
        int suit = 0;                              // Naipe codificado em 0..3.
        bool faceUp = true;                        // True desenha a face; false desenha o verso.
    };

    // Dados minimos para desenhar uma entidade como um cubo fino.
    struct Renderable                              // Componente visual leve (sem material/mesh).
    {
        Color color = WHITE;                       // Cor de fallback usada quando nao ha textura.
        Vector3 size{1.0f, 0.05f, 1.4f};           // Dimensoes do "cubo" em metros (X largura, Y espessura, Z profundidade).
    };

    // Animacao de translacao + flip da carta saindo do baralho.
    // O sistema de animacao avanca 'elapsed' por GetFrameTime; quando elapsed >= duration,
    // o componente e removido e a transform fica fixa no destino.
    struct CardAnim
    {
        Vector3 startPos{0.0f, 0.0f, 0.0f};        // Posicao inicial (geralmente o topo do baralho).
        Vector3 endPos{0.0f, 0.0f, 0.0f};          // Posicao final (slot na mao).
        float startRotZ = 0.0f;                    // Rotacao Z inicial (em graus). Z = eixo longo da carta deitada.
        float endRotZ = 360.0f;                    // Rotacao Z final; uma volta completa = flip lateral.
        float duration = 1.0f;                     // Tempo total da animacao em segundos.
        float elapsed = 0.0f;                      // Tempo decorrido; comeca em 0.
        bool willBeFaceUp = true;                  // Estado final do faceUp; flip acontece em t=0.5.
        bool flipped = false;                      // Marca se ja trocamos faceUp para evitar repetir.
    };
}
