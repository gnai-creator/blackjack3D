#include "input/input_system.h"

#include "raylib.h"

bool InputSystem::gamepadDown(int btn)
{
    // Aceita ate 4 gamepads conectados (0..3); basta um responder.
    for (int g = 0; g < 4; ++g)
    {
        if (IsGamepadAvailable(g) && IsGamepadButtonPressed(g, btn))
            return true;
    }
    return false;
}

InputState InputSystem::poll() const
{
    InputState in;

    // ----- Acoes do jogo -----
    in.hit   = IsKeyPressed(KEY_H) || gamepadDown(GAMEPAD_BUTTON_RIGHT_TRIGGER_1)            // RB / R1
                                   || gamepadDown(GAMEPAD_BUTTON_RIGHT_FACE_DOWN);           // A / cross (tambem confirm)
    in.stand = IsKeyPressed(KEY_S) || gamepadDown(GAMEPAD_BUTTON_LEFT_TRIGGER_1)             // LB / L1
                                   || gamepadDown(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);          // B / circle (tambem cancel)
    in.reset = IsKeyPressed(KEY_R) || gamepadDown(GAMEPAD_BUTTON_RIGHT_FACE_UP);             // Y / triangle
    in.bet   = IsKeyPressed(KEY_B) || gamepadDown(GAMEPAD_BUTTON_RIGHT_FACE_LEFT);           // X / square

    // ----- Confirm / cancel -----
    in.confirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)
              || gamepadDown(GAMEPAD_BUTTON_RIGHT_FACE_DOWN)                                 // A / cross
              || gamepadDown(GAMEPAD_BUTTON_MIDDLE_RIGHT);                                   // Start
    in.cancel  = IsKeyPressed(KEY_BACKSPACE)
              || gamepadDown(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);                               // B / circle

    // ----- Navegacao (setas, D-pad, e analog stick esquerdo) -----
    in.up    = IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W) || gamepadDown(GAMEPAD_BUTTON_LEFT_FACE_UP);
    in.down  = IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S) || gamepadDown(GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    in.left  = IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A) || gamepadDown(GAMEPAD_BUTTON_LEFT_FACE_LEFT);
    in.right = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || gamepadDown(GAMEPAD_BUTTON_LEFT_FACE_RIGHT);

    in.exitRequested = IsKeyPressed(KEY_ESCAPE) || gamepadDown(GAMEPAD_BUTTON_MIDDLE_LEFT);  // Back/Select.

    return in;
}
