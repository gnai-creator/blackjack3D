#include "render/render_system.h"                                  // Header da classe.

#include "raylib.h"                                                // API de alto nivel do raylib.
#include "rlgl.h"                                                  // API de baixo nivel (rlBegin/rlEnd, vertex stream).

namespace
{
    constexpr float CardPixelWidth = 48.0f;                        // Largura util da carta no atlas, em pixels.
    constexpr float CardPixelHeight = 64.0f;                       // Altura util da carta no atlas, em pixels.

    // Atlas tem espacamento NAO uniforme: cartas A..10 com pitch ~52, mas J/Q/K com pitch 56.
    // Para evitar bleeding entre celulas, usamos as posicoes X reais medidas no PNG.
    constexpr float ColumnX[13] = {                                // Posicao X (px) do canto esquerdo de cada rank (A..K).
        6.0f,   58.0f,  109.0f, 161.0f, 213.0f,                    // A, 2, 3, 4, 5
        264.0f, 316.0f, 368.0f, 419.0f, 471.0f,                    // 6, 7, 8, 9, 10
        524.0f, 580.0f, 636.0f                                     // J, Q, K
    };
    constexpr float RowY[5] = {                                    // Posicao Y (px) do topo de cada linha (4 naipes + verso).
        5.0f, 74.0f, 144.0f, 213.0f, 282.0f
    };

    const char *findDeckTexturePath()
    {
        // Permite executar pela raiz do projeto ou pela pasta do binario CMake.
        const char *candidates[] = {                               // Caminhos relativos tentados em ordem.
            "resources/decks.png",                                 // Executando da raiz do projeto.
            "../resources/decks.png",                              // Um nivel abaixo (build/).
            "../../resources/decks.png",                           // Dois niveis abaixo.
            "../../../resources/decks.png"};                       // Tres niveis (build/Debug/x64/...).

        for (const char *path : candidates)                        // Tenta cada caminho ate achar um valido.
        {
            if (FileExists(path))                                  // Helper do raylib (verifica disco).
            {
                return path;                                       // Devolve o primeiro que existir.
            }
        }

        return nullptr;                                            // Nenhum caminho serviu; cai no fallback de cubos.
    }
}

RenderSystem::RenderSystem()
{
    const char *texturePath = findDeckTexturePath();               // Procura o atlas no disco.
    if (texturePath)
    {
        deckTexture = LoadTexture(texturePath);                    // Carrega para a GPU. Requer InitWindow ja chamado.
        hasDeckTexture = deckTexture.id != 0;                      // id == 0 indica falha de upload na GPU.
    }
}

RenderSystem::~RenderSystem()
{
    if (hasDeckTexture)                                            // Evita liberar textura nao inicializada.
    {
        UnloadTexture(deckTexture);                                // Libera memoria de GPU.
    }
}

void RenderSystem::drawScene(const Registry &registry) const
{
    drawTable();                                                   // Desenha primeiro o tampo (atras das cartas).
    drawCards(registry);                                           // Depois as cartas, posicionadas em cima.
}

void RenderSystem::drawTable() const
{
    DrawCube(Vector3{0.0f, -0.08f, 0.0f}, 8.0f, 0.16f, 5.0f, DARKGREEN); // Cubo achatado simulando o tampo verde.
}

void RenderSystem::drawCards(const Registry &registry) const
{
    rlDisableBackfaceCulling();                                    // Cartas giram 180o no flip; precisamos das duas faces.

    for (Entity entity : registry.getEntities())                   // Itera TODAS as entidades (filtra dentro do loop).
    {
        const blackjack3D::Transform *transform = registry.getTransform(entity);
        const blackjack3D::Card *card = registry.getCard(entity);
        const blackjack3D::Renderable *renderable = registry.getRenderable(entity);

        if (!transform || !card || !renderable)
        {
            continue;
        }

        if (hasDeckTexture)
        {
            drawTexturedCard(*transform, *renderable, sourceForCard(*card));
        }
        else
        {
            Color color = card->faceUp ? renderable->color : Color{80, 20, 20, 255};
            DrawCubeV(transform->position, renderable->size, color);
            DrawCubeWiresV(transform->position, renderable->size, WHITE);
        }
    }

    rlEnableBackfaceCulling();                                     // Restaura estado padrao.
}

Rectangle RenderSystem::sourceForCard(const blackjack3D::Card &card) const
{
    if (!card.faceUp)
    {
        // Carta virada: verso na coluna 2, linha 4 do atlas.
        return Rectangle{ColumnX[2], RowY[4], CardPixelWidth, CardPixelHeight};
    }

    // Atlas: linha 0 = ouros, 1 = paus, 2 = copas, 3 = espadas.
    const int suitRows[] = {2, 0, 1, 3};                           // Mapeia suit interno (0=copas..) para a linha real do atlas.
    const int row = suitRows[card.suit % 4];                       // % 4 e defensivo contra valores fora de faixa.
    const int column = card.rank - 1;                              // rank 1..13 -> coluna 0..12.

    return Rectangle{
        ColumnX[column],                                           // X exato da carta (lookup; pitch nao e uniforme no atlas).
        RowY[row],                                                 // Y exato da linha do naipe.
        CardPixelWidth,                                            // 48 px de carta (sem espacamento).
        CardPixelHeight};                                          // 64 px de carta.
}

void RenderSystem::drawTexturedCard(
    const blackjack3D::Transform &transform,
    const blackjack3D::Renderable &renderable,
    Rectangle source) const
{
    const float halfWidth = renderable.size.x * transform.scale.x * 0.5f;  // Meia largura em metros (eixo X).
    const float halfDepth = renderable.size.z * transform.scale.z * 0.5f;  // Meia profundidade (eixo Z).

    const float u0 = source.x / static_cast<float>(deckTexture.width);     // UV [0..1] da regiao no atlas.
    const float v0 = source.y / static_cast<float>(deckTexture.height);
    const float u1 = (source.x + source.width) / static_cast<float>(deckTexture.width);
    const float v1 = (source.y + source.height) / static_cast<float>(deckTexture.height);

    rlPushMatrix();                                                // Empilha matriz para aplicar transform local.
    rlTranslatef(transform.position.x, transform.position.y, transform.position.z); // Move para a posicao da carta.
    rlRotatef(transform.rotation.x, 1.0f, 0.0f, 0.0f);             // Aplica rotacao X (flip da carta durante anim).
    rlRotatef(transform.rotation.y, 0.0f, 1.0f, 0.0f);             // Y (geralmente 0).
    rlRotatef(transform.rotation.z, 0.0f, 0.0f, 1.0f);             // Z (geralmente 0).

    rlSetTexture(deckTexture.id);
    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);
    rlNormal3f(0.0f, 1.0f, 0.0f);

    // Vertices em coordenadas locais (origem = centro da carta).
    rlTexCoord2f(u0, v1);
    rlVertex3f(-halfWidth, 0.0f, +halfDepth);

    rlTexCoord2f(u1, v1);
    rlVertex3f(+halfWidth, 0.0f, +halfDepth);

    rlTexCoord2f(u1, v0);
    rlVertex3f(+halfWidth, 0.0f, -halfDepth);

    rlTexCoord2f(u0, v0);
    rlVertex3f(-halfWidth, 0.0f, -halfDepth);

    rlEnd();
    rlSetTexture(0);

    rlPopMatrix();                                                 // Desfaz a transform para nao vazar.
}
