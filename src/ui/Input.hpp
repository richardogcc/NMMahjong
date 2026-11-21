// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include "raylib.h"

namespace nm {

// Órdenes de teclado y mando de este fotograma, ya unificadas.
struct Nav {
  bool up = false, down = false, left = false, right = false;
  bool accept = false;   // Enter, espacio o botón A
  bool back = false;     // Escape o botón B
  bool hint = false;     // H o botón X
  bool undo = false;     // Z o botón Y
  bool shuffle = false;  // B o gatillo derecho
  bool pause = false;    // Escape, P o Start
  Vector2 stick = {};    // palanca izquierda, para mover el puntero
  bool any() const { return up || down || left || right || accept; }
};

inline Nav readNav() {
  Nav n;
  n.up = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
  n.down = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S);
  n.left = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A);
  n.right = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D);
  n.accept = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_KP_ENTER);
  n.back = IsKeyPressed(KEY_ESCAPE);
  n.hint = IsKeyPressed(KEY_H);
  n.undo = IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_U);
  n.shuffle = IsKeyPressed(KEY_B);
  n.pause = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P);
  if (IsGamepadAvailable(0)) {
    n.up = n.up || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP);
    n.down = n.down || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    n.left = n.left || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
    n.right = n.right || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
    n.accept = n.accept || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    n.back = n.back || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
    n.hint = n.hint || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
    n.undo = n.undo || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
    n.shuffle = n.shuffle || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
    n.pause = n.pause || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);
    n.stick = {GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X), GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)};
    if (n.stick.x * n.stick.x + n.stick.y * n.stick.y < 0.06f) n.stick = {};
  }
  return n;
}

}  // namespace nm
