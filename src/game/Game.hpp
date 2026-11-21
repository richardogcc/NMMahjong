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
enum class BoardKind { Level = 0, Random = 1, Daily = 2 };
enum class PlayStyle { Normal = 0, Timed = 1, Zen = 2 };

// Qué partida se pide: un nivel fijo, un tablero aleatorio, el reto del día
// o continuar la que quedó guardada.
struct GameRequest {
  BoardKind kind = BoardKind::Level;
  int level = 0;
  bool resume = false;
};

// Una partida: reglas, animaciones y marcador.
class Game {
 public:
  void start(const GameRequest& request, Ctx& ctx);
  void update(Ctx& ctx);
  // Dibuja la partida y atiende los botones del marcador.
  GameAction draw(Ctx& ctx);

  GameRequest sameRequest() const { return {kind_, level_, false}; }
  GameRequest nextRequest() const { return {kind_, level_ + (kind_ == BoardKind::Level ? 1 : 0), false}; }
  // Qué ambiente (fondo y música) acompaña a esta partida.
  int themeIndex() const { return themeIndex_; }
  // Pausa o reanuda; no hace nada si la partida ya ha terminado.
  void togglePause();
  // Guarda la partida a medias para poder continuarla (al salir al menú o cerrar).
  void suspend() const;
  // Juega sola una pareja válida (para las capturas de pantalla).
  bool autoMove(Ctx& ctx);

 private:
  enum class State { Playing, Paused, Won, Stuck, TimeUp };

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

  // Game.cpp: montaje, geometría y guardado
  bool resume(Ctx& ctx);
  void resetRound(Ctx& ctx);
  const Layout& layout() const;
  void layoutBoard();
  void dealAnimation(float baseDelay);
  Vector2 faceCenter(const Slot& slot) const;
  int pick(Vector2 mouse) const;
  bool finished() const { return state_ == State::Won || state_ == State::TimeUp; }

  // GamePlay.cpp: reglas, entrada y progreso
  void moveCursor(int dx, int dy);
  void handleClick(int index, Ctx& ctx);
  void match(int a, int b, Ctx& ctx);
  void afterBoardChange(Ctx& ctx);
  void useHint(Ctx& ctx);
  void useUndo(Ctx& ctx);
  void useShuffle(Ctx& ctx);
  void win(Ctx& ctx);
  void unlock(const char* id, Ctx& ctx);
  int hintsLeft() const { return hintLimit_ < 0 ? -1 : hintLimit_ - hintsUsed_; }
  int shufflesLeft() const { return shuffleLimit_ < 0 ? -1 : shuffleLimit_ - shufflesUsed_; }

  // GameDraw.cpp
  void drawBoard(const Ctx& ctx) const;
  void drawGhosts(const Ctx& ctx) const;
  GameAction drawHud(Ctx& ctx);
  GameAction drawWon(Ctx& ctx);
  GameAction drawStuck(Ctx& ctx);
  GameAction drawPaused(Ctx& ctx);
  GameAction drawTimeUp(Ctx& ctx);
  void drawToast(const Ctx& ctx) const;
  const char* title() const;

  Board board_;
  std::vector<TileView> views_;
  std::vector<int> order_;  // índices ordenados de atrás hacia delante
  std::vector<Ghost> ghosts_;
  std::vector<Popup> popups_;
  Particles particles_;

  BoardKind kind_ = BoardKind::Level;
  int level_ = 0;
  std::uint32_t layoutSeed_ = 0;  // de qué semilla salió el tablero aleatorio o diario
  Layout generated_;              // tablero aleatorio o diario
  int themeIndex_ = 0;
  Difficulty difficulty_ = Difficulty::Easy;
  PlayStyle style_ = PlayStyle::Normal;
  int hintLimit_ = -1, shuffleLimit_ = -1;  // -1: sin límite

  State state_ = State::Playing;
  bool deadEnd_ = false;  // ni barajando queda salida
  float stateTime_ = 0.0f;
  float introTime_ = 0.0f;
  float confettiTimer_ = 0.0f;

  float tileW_ = 64.0f, tileH_ = 84.0f, depth_ = 9.0f;
  Vector2 origin_ = {};
  int minX_ = 0, minY_ = 0;

  int selected_ = -1, hover_ = -1;
  int cursor_ = -1;          // ficha señalada con teclado o mando
  bool cursorActive_ = false;
  int hintA_ = -1, hintB_ = -1;
  float hintTimer_ = 0.0f;

  float elapsed_ = 0.0f, timeLeft_ = 0.0f;
  int score_ = 0, combo_ = 0, stars_ = 0, moves_ = 0;
  float comboTimer_ = 0.0f;
  int hintsUsed_ = 0, shufflesUsed_ = 0;

  std::string toast_;  // logro recién desbloqueado
  float toastTime_ = 0.0f;
};

}  // namespace nm
