#include "ecs/registry.h"
#include "game/blackjack_rules.h"
#include "game/deck_system.h"
#include "game/money_system.h"
#include "game/ranking_system.h"
#include "input/input_system.h"
#include "render/render_system.h"

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace
{

// ===== Constantes de layout =====
constexpr float CardSpacing   = 0.9f;
constexpr float DealerHandZ   = -1.1f;
constexpr float PlayerHandZ   =  1.1f;
constexpr float HandY         =  0.05f;
constexpr float CardAnimDur   =  1.0f;
constexpr float DealStepDelay =  0.3f;     // Defasagem entre as 4 cartas iniciais.
constexpr float ArcHeight     =  1.2f;     // Pico do arco da carta voando do baralho.
constexpr float RotStartT     =  0.2f;     // Rotacao so comeca depois de subir um pouco...
constexpr float RotEndT       =  0.8f;     // ...e termina antes de aterrissar (carta plana ao tocar a mesa).

// ===== Estados de alto nivel do jogo =====
enum class State
{
    Menu,
    Ranking,
    Betting,         // entre rodadas, jogador escolhe a aposta
    Dealing,         // animacao das 4 cartas iniciais
    PlayerTurn,      // hit/stand
    DealerTurn,      // dealer joga automatico
    RoundResult,     // mostra resultado, ENTER continua
    InitialInput,    // 3 iniciais para o ranking
};

enum class RoundOutcome { None, PlayerWin, DealerWin, Push };

struct RoundState
{
    Registry registry;
    std::vector<Entity> playerHand;
    std::vector<Entity> dealerHand;
    bool dealerHoleRevealed = false;
    RoundOutcome outcome = RoundOutcome::None;
};

// ===== Helpers de posicionamento =====
Vector3 handSlotPos(int index, std::size_t count, float z)
{
    const float startX = -static_cast<float>(count - 1) * CardSpacing * 0.5f;
    return Vector3{startX + index * CardSpacing, HandY, z};
}

// ===== Sistema de animacao =====
void startCardAnim(Registry& registry, Entity entity, Vector3 endPos, bool willBeFaceUp,
                   float duration = CardAnimDur, float startDelay = 0.0f)
{
    auto* tf = registry.getTransform(entity);
    if (!tf) return;

    blackjack3D::CardAnim anim;
    anim.startPos = tf->position;
    anim.endPos = endPos;
    anim.startRotZ = 0.0f;
    anim.endRotZ = 360.0f;                           // Volta completa em torno de Z (flip lateral, eixo longo).
    anim.duration = duration;
    anim.elapsed = -startDelay;                      // Negativo = atraso ate comecar a se mover.
    anim.willBeFaceUp = willBeFaceUp;
    anim.flipped = false;

    registry.addCardAnim(entity, anim);
}

void updateAnimations(Registry& registry, float dt)
{
    std::vector<Entity> toRemove;

    for (Entity entity : registry.getEntities())
    {
        auto* anim = registry.getCardAnim(entity);
        if (!anim) continue;

        anim->elapsed += dt;
        float t = anim->elapsed / anim->duration;
        if (t < 0.0f) t = 0.0f;                      // Ainda no atraso: carta fica parada na startPos.
        if (t > 1.0f) t = 1.0f;

        auto* tf = registry.getTransform(entity);
        if (tf)
        {
            tf->position.x = anim->startPos.x + (anim->endPos.x - anim->startPos.x) * t;
            tf->position.z = anim->startPos.z + (anim->endPos.z - anim->startPos.z) * t;
            const float baseY = anim->startPos.y + (anim->endPos.y - anim->startPos.y) * t;
            tf->position.y = baseY + std::sin(t * PI) * ArcHeight;  // Arco parabolico em Y.

            // Rotacao acontece apenas no meio da animacao (entre RotStartT e RotEndT) para
            // garantir que a carta esteja PLANA ao tocar a mesa.
            float rt = (t - RotStartT) / (RotEndT - RotStartT);
            if (rt < 0.0f) rt = 0.0f;
            if (rt > 1.0f) rt = 1.0f;
            tf->rotation.z = anim->startRotZ + (anim->endRotZ - anim->startRotZ) * rt;
        }

        if (!anim->flipped && t >= 0.5f)              // Troca o source da carta no meio da rotacao.
        {
            auto* card = registry.getCard(entity);
            if (card) card->faceUp = anim->willBeFaceUp;
            anim->flipped = true;
        }

        if (t >= 1.0f)
        {
            if (tf)
            {
                tf->position = anim->endPos;          // Garante posicao final exata.
                tf->rotation.z = 0.0f;                // Volta ao plano da mesa.
            }
            toRemove.push_back(entity);
        }
    }

    for (Entity e : toRemove) registry.removeCardAnim(e);
}

bool hasActiveAnimations(const Registry& registry)
{
    for (Entity entity : registry.getEntities())
    {
        if (registry.getCardAnim(entity)) return true;
    }
    return false;
}

// ===== Distribuicao com animacao =====
void dealOneCard(DeckSystem& deck, Registry& registry, std::vector<Entity>& hand,
                 float zHand, bool faceUp, int slotIdx, int totalSlots, float startDelay)
{
    auto entityOpt = deck.drawTop();
    if (!entityOpt) return;
    hand.push_back(*entityOpt);
    const Vector3 endPos = handSlotPos(slotIdx, totalSlots, zHand);
    startCardAnim(registry, *entityOpt, endPos, faceUp, CardAnimDur, startDelay);
}

void hitCard(DeckSystem& deck, Registry& registry, std::vector<Entity>& hand,
             float zHand, bool faceUp)
{
    auto entityOpt = deck.drawTop();
    if (!entityOpt) return;
    hand.push_back(*entityOpt);
    const int newSize = static_cast<int>(hand.size());

    for (int i = 0; i < newSize - 1; ++i)             // Snap das cartas antigas para os novos slots.
    {
        auto* tf = registry.getTransform(hand[i]);
        if (tf) tf->position = handSlotPos(i, newSize, zHand);
    }

    const Vector3 endPos = handSlotPos(newSize - 1, newSize, zHand);
    startCardAnim(registry, *entityOpt, endPos, faceUp);
}

void revealHoleCard(Registry& registry, const std::vector<Entity>& dealerHand)
{
    if (dealerHand.size() < 2) return;
    Entity hole = dealerHand[1];
    auto* tf = registry.getTransform(hole);
    if (!tf) return;

    blackjack3D::CardAnim anim;                       // Anim no lugar (so vira).
    anim.startPos = tf->position;
    anim.endPos = tf->position;
    anim.startRotZ = 0.0f;
    anim.endRotZ = 360.0f;
    anim.duration = 0.6f;
    anim.elapsed = 0.0f;
    anim.willBeFaceUp = true;
    anim.flipped = false;
    registry.addCardAnim(hole, anim);
}

// ===== UI 2D =====
void drawCenteredText(const char* text, int y, int fontSize, Color color)
{
    int w = MeasureText(text, fontSize);
    DrawText(text, (GetScreenWidth() - w) / 2, y, fontSize, color);
}

void drawMenu(int selected)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 140});  // Veu para legibilidade.
    drawCenteredText("BLACKJACK 3D", 80, 56, RAYWHITE);

    const char* options[] = {"JOGAR", "RANKING", "SAIR"};
    for (int i = 0; i < 3; ++i)
    {
        Color c = (i == selected) ? YELLOW : RAYWHITE;
        std::string s = (i == selected ? "> " : "  ") + std::string(options[i]);
        drawCenteredText(s.c_str(), 240 + i * 50, 32, c);
    }
    drawCenteredText("Setas/D-pad: navegar    Enter/(A): selecionar    ESC/(Start): sair", 660, 18, GRAY);
}

void drawRanking(const RankingSystem& ranking)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 160});
    drawCenteredText("RANKING - TOP 10", 60, 40, RAYWHITE);
    int y = 130;
    int rank = 1;
    for (const auto& e : ranking.entries())
    {
        char line[64];
        std::snprintf(line, sizeof(line), "%2d.   %s    %5d coins", rank, e.initials, e.score);
        drawCenteredText(line, y, 28, RAYWHITE);
        y += 36;
        ++rank;
    }
    if (ranking.entries().empty())
    {
        drawCenteredText("(sem registros ainda)", 200, 24, GRAY);
    }
    drawCenteredText("ESC/(B)/(Start): voltar", 660, 18, GRAY);
}

void drawBettingHUD(const MoneySystem& money)
{
    DrawText(TextFormat("Saldo: %d", money.balance()),    20, 20, 24, RAYWHITE);
    DrawText(TextFormat("Aposta: %d", money.bet()),       20, 50, 24, YELLOW);
    DrawText(TextFormat("Recorde: %d", money.bestBalance()), 20, 80, 22, GRAY);
    drawCenteredText("Cima/Baixo (D-pad): aposta x2 ou /2    Enter/(A): distribuir    ESC/(Start): cash out", 670, 20, RAYWHITE);
}

void drawGameHUD(const RoundState& round, const MoneySystem& money,
                 bool inPlayerTurn, const char* statusMsg)
{
    DrawText(TextFormat("Saldo: %d", money.balance()), 20, 20, 24, RAYWHITE);
    DrawText(TextFormat("Aposta: %d", money.bet()),   20, 50, 24, YELLOW);
    int playerScore = Blackjack3DRules::calculateScore(round.playerHand, round.registry);
    DrawText(TextFormat("Jogador: %d", playerScore), 20, 80, 22, RAYWHITE);
    if (round.dealerHoleRevealed)
    {
        int ds = Blackjack3DRules::calculateScore(round.dealerHand, round.registry);
        DrawText(TextFormat("Dealer: %d", ds), 20, 110, 22, RAYWHITE);
    }
    else
    {
        DrawText("Dealer: ?", 20, 110, 22, RAYWHITE);
    }

    if (inPlayerTurn)
    {
        DrawText("H/(RB) hit    S/(LB) stand    ESC/(Start) menu", 20, GetScreenHeight() - 36, 20, RAYWHITE);
    }
    if (statusMsg && *statusMsg)
    {
        drawCenteredText(statusMsg, 280, 36, YELLOW);
        drawCenteredText("Enter/(A): continuar", 330, 22, RAYWHITE);
    }
}

void drawInitialInput(const char initials[3], int cursor, int score)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 160});
    drawCenteredText("NOVO RECORDE!", 100, 48, YELLOW);
    drawCenteredText(TextFormat("Pontuacao: %d", score), 170, 28, RAYWHITE);
    drawCenteredText("Digite suas iniciais:", 230, 24, RAYWHITE);

    const int spacing = 80;
    const int totalW = spacing * 3;
    const int startX = (GetScreenWidth() - totalW) / 2;
    for (int i = 0; i < 3; ++i)
    {
        char buf[2] = { initials[i] ? initials[i] : '_', '\0' };
        Color c = (i == cursor) ? YELLOW : RAYWHITE;
        DrawText(buf, startX + i * spacing + 20, 290, 64, c);
        if (i == cursor)
        {
            DrawText("_", startX + i * spacing + 20, 320, 64, YELLOW);  // Underline do cursor.
        }
    }
    drawCenteredText("A-Z ou Cima/Baixo (D-pad): letra    Esq/Dir (D-pad): cursor    Enter/(A): confirmar", 460, 18, GRAY);
}

// ===== Inicia uma nova rodada (cartas frescas, animacao escalonada) =====
void startNewRound(RoundState& round, DeckSystem& deck)
{
    round.registry.clear();                          // Limpa cartas da rodada anterior.
    round.playerHand.clear();
    round.dealerHand.clear();
    round.dealerHoleRevealed = false;
    round.outcome = RoundOutcome::None;

    deck.createDeck();                               // Cria 52 cartas empilhadas no canto sup-esq.
    deck.shuffle();

    dealOneCard(deck, round.registry, round.playerHand, PlayerHandZ, true,  0, 2, 0 * DealStepDelay);
    dealOneCard(deck, round.registry, round.dealerHand, DealerHandZ, true,  0, 2, 1 * DealStepDelay);
    dealOneCard(deck, round.registry, round.playerHand, PlayerHandZ, true,  1, 2, 2 * DealStepDelay);
    dealOneCard(deck, round.registry, round.dealerHand, DealerHandZ, false, 1, 2, 3 * DealStepDelay); // hole.
}

}  // anonymous namespace


int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    SetExitKey(KEY_NULL);
    InitWindow(1280, 720, "Blackjack3D");
    SetTargetFPS(60);

    Camera3D camera{};
    camera.position = Vector3{0.0f, 5.5f, 6.0f};
    camera.target = Vector3{0.0f, 0.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    InputSystem  inputSystem;
    RenderSystem renderSystem;
    MoneySystem  money;
    RankingSystem ranking;

    RoundState round;
    DeckSystem deck(round.registry);

    // Cria um baralho visual inicial para o menu (puramente decorativo).
    deck.createDeck();

    State state = State::Menu;
    int  menuSelected = 0;
    constexpr int MenuOptionsCount = 3;

    char inputInitials[3] = {'A', 'A', 'A'};
    int  initialsCursor = 0;
    int  pendingScore = 0;

    const char* statusMsg = "";
    bool shouldExit = false;

    while (!shouldExit && !WindowShouldClose())
    {
        const float dt = GetFrameTime();
        const InputState input = inputSystem.poll();

        updateAnimations(round.registry, dt);        // Avanca animacoes em todos os estados.

        // ===== Update por estado =====
        switch (state)
        {
            case State::Menu:
                if (input.down)
                    menuSelected = (menuSelected + 1) % MenuOptionsCount;
                if (input.up)
                    menuSelected = (menuSelected + MenuOptionsCount - 1) % MenuOptionsCount;
                if (input.confirm)
                {
                    if (menuSelected == 0)
                    {
                        money.newSession();
                        round.registry.clear();
                        round.playerHand.clear();
                        round.dealerHand.clear();
                        round.dealerHoleRevealed = false;
                        deck.createDeck();           // Baralho visivel para a tela de aposta.
                        state = State::Betting;
                        statusMsg = "";
                    }
                    else if (menuSelected == 1)
                    {
                        state = State::Ranking;
                    }
                    else
                    {
                        shouldExit = true;
                    }
                }
                if (input.exitRequested) shouldExit = true;
                break;

            case State::Ranking:
                if (input.exitRequested || input.confirm || input.cancel)
                    state = State::Menu;
                break;

            case State::Betting:
                if (input.up)   money.increaseBet();
                if (input.down) money.decreaseBet();
                if (input.confirm)
                {
                    if (money.canStartRound())
                    {
                        startNewRound(round, deck);
                        state = State::Dealing;
                    }
                }
                if (input.exitRequested)
                {
                    // Cash out: registra recorde se for top10.
                    if (ranking.isHighScore(money.bestBalance()))
                    {
                        pendingScore = money.bestBalance();
                        std::fill(std::begin(inputInitials), std::end(inputInitials), 'A');
                        initialsCursor = 0;
                        state = State::InitialInput;
                    }
                    else
                    {
                        state = State::Menu;
                    }
                }
                break;

            case State::Dealing:
                if (!hasActiveAnimations(round.registry))
                {
                    state = State::PlayerTurn;       // Animacoes terminaram: jogador joga.
                }
                break;

            case State::PlayerTurn:
                if (!hasActiveAnimations(round.registry))
                {
                    // Verifica se ja estourou (apos hit anterior terminar a anim).
                    if (Blackjack3DRules::isBust(round.playerHand, round.registry))
                    {
                        revealHoleCard(round.registry, round.dealerHand);
                        round.dealerHoleRevealed = true;
                        round.outcome = RoundOutcome::DealerWin;
                        statusMsg = "Voce estourou";
                        state = State::RoundResult;
                        break;
                    }

                    if (input.hit)
                    {
                        hitCard(deck, round.registry, round.playerHand, PlayerHandZ, true);
                    }
                    else if (input.stand)
                    {
                        revealHoleCard(round.registry, round.dealerHand);
                        round.dealerHoleRevealed = true;
                        state = State::DealerTurn;
                    }
                    else if (input.exitRequested)
                    {
                        // Sair no meio da rodada = cash out do que ja acumulou (sem aplicar a aposta corrente).
                        if (ranking.isHighScore(money.bestBalance()))
                        {
                            pendingScore = money.bestBalance();
                            std::fill(std::begin(inputInitials), std::end(inputInitials), 'A');
                            initialsCursor = 0;
                            state = State::InitialInput;
                        }
                        else
                        {
                            state = State::Menu;
                        }
                    }
                }
                break;

            case State::DealerTurn:
                if (!hasActiveAnimations(round.registry))
                {
                    int dealerScore = Blackjack3DRules::calculateScore(round.dealerHand, round.registry);
                    if (dealerScore < 17)
                    {
                        hitCard(deck, round.registry, round.dealerHand, DealerHandZ, true);
                    }
                    else
                    {
                        int playerScore = Blackjack3DRules::calculateScore(round.playerHand, round.registry);
                        if (dealerScore > 21)
                        {
                            round.outcome = RoundOutcome::PlayerWin;
                            statusMsg = "Dealer estourou. Voce venceu!";
                        }
                        else if (playerScore > dealerScore)
                        {
                            round.outcome = RoundOutcome::PlayerWin;
                            statusMsg = "Voce venceu!";
                        }
                        else if (playerScore < dealerScore)
                        {
                            round.outcome = RoundOutcome::DealerWin;
                            statusMsg = "Dealer venceu";
                        }
                        else
                        {
                            round.outcome = RoundOutcome::Push;
                            statusMsg = "Empate";
                        }
                        state = State::RoundResult;
                    }
                }
                break;

            case State::RoundResult:
                if (input.confirm)
                {
                    if (round.outcome == RoundOutcome::PlayerWin)
                    {
                        money.onWin();
                        statusMsg = "";
                        state = State::Betting;
                    }
                    else if (round.outcome == RoundOutcome::DealerWin)
                    {
                        money.onLose();
                        statusMsg = "";
                        if (ranking.isHighScore(money.bestBalance()))
                        {
                            pendingScore = money.bestBalance();
                            std::fill(std::begin(inputInitials), std::end(inputInitials), 'A');
                            initialsCursor = 0;
                            state = State::InitialInput;
                        }
                        else
                        {
                            state = State::Menu;
                        }
                    }
                    else
                    {
                        money.onPush();
                        statusMsg = "";
                        state = State::Betting;
                    }
                }
                if (input.exitRequested)
                {
                    statusMsg = "";
                    state = State::Menu;
                }
                break;

            case State::InitialInput:
            {
                // Letras pelo teclado (digitacao direta).
                int ch = GetCharPressed();
                while (ch > 0)
                {
                    if (ch >= 'a' && ch <= 'z') ch -= 32;       // Upper-case.
                    if (ch >= 'A' && ch <= 'Z')
                    {
                        inputInitials[initialsCursor] = static_cast<char>(ch);
                        if (initialsCursor < 2) ++initialsCursor;
                    }
                    ch = GetCharPressed();
                }

                // Cima/baixo (D-pad ou setas) cicla a letra atual (estilo arcade).
                if (input.up)
                {
                    char c = inputInitials[initialsCursor];
                    if (c < 'A' || c > 'Z') c = 'A';
                    inputInitials[initialsCursor] = (c == 'Z' ? 'A' : static_cast<char>(c + 1));
                }
                if (input.down)
                {
                    char c = inputInitials[initialsCursor];
                    if (c < 'A' || c > 'Z') c = 'A';
                    inputInitials[initialsCursor] = (c == 'A' ? 'Z' : static_cast<char>(c - 1));
                }

                if (input.left  && initialsCursor > 0) --initialsCursor;
                if (input.right && initialsCursor < 2) ++initialsCursor;
                if (input.cancel)
                {
                    if (initialsCursor > 0) --initialsCursor;
                    inputInitials[initialsCursor] = 'A';
                }
                if (input.confirm)
                {
                    ranking.addEntry(inputInitials, pendingScore);
                    ranking.save();
                    state = State::Menu;
                }
                if (input.exitRequested) state = State::Menu;
                break;
            }
        }

        // ===== Render =====
        BeginDrawing();
        ClearBackground(Color{28, 32, 36, 255});

        BeginMode3D(camera);
        renderSystem.drawScene(round.registry);
        EndMode3D();

        switch (state)
        {
            case State::Menu:        drawMenu(menuSelected);                                                 break;
            case State::Ranking:     drawRanking(ranking);                                                   break;
            case State::Betting:     drawBettingHUD(money);                                                  break;
            case State::Dealing:
            case State::PlayerTurn:
            case State::DealerTurn:
            case State::RoundResult: drawGameHUD(round, money, state == State::PlayerTurn, statusMsg);       break;
            case State::InitialInput: drawInitialInput(inputInitials, initialsCursor, pendingScore);         break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
