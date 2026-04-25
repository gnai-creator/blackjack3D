#pragma once                            // Inclusao unica por TU.

// Snapshot das acoes pressionadas neste frame.
// Cada campo agrega teclado + gamepad para que o resto do codigo nao precise
// se preocupar com a fonte do input.
struct InputState
{
    // Acoes especificas do jogo (in-round).
    bool hit = false;                   // H ou RB do gamepad: pedir mais uma carta.
    bool stand = false;                 // S ou LB do gamepad: parar de pedir.
    bool reset = false;                 // R ou Y/triangle: reset (debug).
    bool bet = false;                   // Reservado para acao de aposta dedicada.

    // Acoes de menu / fluxo.
    bool confirm = false;               // ENTER/SPACE ou A/cross: confirmar/selecionar.
    bool cancel = false;                // BACKSPACE ou B/circle: voltar/cancelar.
    bool up = false;                    // Setas/D-pad/Stick: navegacao.
    bool down = false;
    bool left = false;
    bool right = false;

    bool exitRequested = false;         // ESC ou Start: sair (depende do contexto, main decide).
};

class InputSystem                       // Wrapper sobre a API de input do raylib (teclado + gamepad).
{
    public:
        InputState poll() const;        // Snapshot agregado de todas as fontes de input.

    private:
        static bool gamepadDown(int btn);  // Helper: true se algum dos primeiros 4 gamepads tem o botao pressed neste frame.
};
