// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/Board.hpp"
#include "game/Ctx.hpp"
#include "render/Particles.hpp"

namespace nm {

enum class GameAction { None, Menu, Next, Restart };

// Una partida: reglas, animaciones y marcador de un nivel.
class Game {
 public:
  void start(int levelIndex, Ctx& ctx);
  void update(Ctx& ctx);
  // Dibuja la partida y atiende los botones del marcador.
  GameAction draw(Ctx& ctx);

  // -1 en el modo aleatorio.
  int level() const { return random_ ? -1 : level_; }
  // Qué ambiente (fondo y música) acompaña a esta partida.
  int themeIndex() const { return themeIndex_; }
  // Pausa o reanuda; no hace nada si la partida ya ha terminado.
  void togglePause();
  // Juega sola una pareja válida (para las capturas de pantalla).
  bool autoMove(Ctx& ctx);

 private:
  enum class State { Playing, Paused, Won, Stuck };

  struct TileView {
    float delay = 0.0f;   // cuándo empieza a caer en la animación de entrada
    float appear = 0.0f;  // 0 fuera, 1 colocada
    float lift = 0.0f;    // se eleva al pasar el ratón o al seleccionarla
    float shake = 0.0f;   // tiembla si está bloqueada
    float pop = 0.0f;     // rebote al volver con «deshacer»
  };
  struct Ghost {  // ficha retirada que vuela hacia su pareja
    int face;
    Vector2 from, to;
    float t;
    bool burst;
  };
  struct Popup {
    Vector2 pos;
    std::string text;
    float t;
    Color color;
  };

  // Game.cpp
  void layoutBoard();
  void dealAnimation(float baseDelay);
  Vector2 faceCenter(const Slot& slot) const;
  int pick(Vector2 mouse) const;
  void handleClick(int index, Ctx& ctx);
  void match(int a, int b, Ctx& ctx);
  void afterBoardChange(Ctx& ctx);
  void useHint(Ctx& ctx);
  void useUndo(Ctx& ctx);
  void useShuffle(Ctx& ctx);
  void win(Ctx& ctx);

  // GameDraw.cpp
  void drawBoard(const Ctx& ctx) const;
  void drawGhosts(const Ctx& ctx) const;
  GameAction drawHud(Ctx& ctx);
  GameAction drawWon(Ctx& ctx);
  GameAction drawStuck(Ctx& ctx);
  GameAction drawPaused(Ctx& ctx);
  const char* title() const;

  Board board_;
  std::vector<TileView> views_;
  std::vector<int> order_;  // índices ordenados de atrás hacia delante
  std::vector<Ghost> ghosts_;
  std::vector<Popup> popups_;
  Particles particles_;

  int level_ = 0;
  bool random_ = false;      // tablero inventado en lugar de un nivel fijo
  int themeIndex_ = 0;
  Layout randomLayout_;
  State state_ = State::Playing;
  bool deadEnd_ = false;  // ni barajando queda salida
  float stateTime_ = 0.0f;
  float introTime_ = 0.0f;
  float confettiTimer_ = 0.0f;

  float tileW_ = 64.0f, tileH_ = 84.0f, depth_ = 9.0f;
  Vector2 origin_ = {};
  int minX_ = 0, minY_ = 0;

  int selected_ = -1, hover_ = -1;
  int hintA_ = -1, hintB_ = -1;
  float hintTimer_ = 0.0f;

  float elapsed_ = 0.0f;
  int score_ = 0, combo_ = 0, stars_ = 0, moves_ = 0;
  float comboTimer_ = 0.0f;
  int hintsUsed_ = 0, shufflesUsed_ = 0;
};

}  // namespace nm
