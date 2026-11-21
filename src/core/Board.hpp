// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "core/Layout.hpp"

namespace nm {

struct Tile {
  Slot pos;
  int face = 0;
  bool removed = false;
};

// Estado de una partida de mahjong solitario. No depende de gráficos ni de audio.
class Board {
 public:
  // Reparte las fichas de forma que el tablero tenga solución garantizada.
  bool generate(const Layout& layout, std::uint32_t seed);

  const std::vector<Tile>& tiles() const { return tiles_; }
  // Orden de retirada que resuelve el reparto recién generado.
  const std::vector<std::pair<int, int>>& solution() const { return solution_; }

  // Una ficha está libre si no tiene nada encima y tiene un lateral despejado.
  bool isFree(int index) const;
  bool canMatch(int a, int b) const;
  bool removePair(int a, int b);
  std::optional<std::pair<int, int>> undo();
  bool canUndo() const { return !history_.empty(); }

  std::optional<std::pair<int, int>> findHint() const;
  int availableMoves() const;
  int remaining() const;
  bool cleared() const { return remaining() == 0; }

  // Vuelve a repartir las caras de las fichas que quedan. Devuelve false si
  // ni siquiera barajando queda algún movimiento posible.
  bool shuffle(std::uint32_t seed);

 private:
  std::vector<Tile> tiles_;
  std::vector<std::pair<int, int>> history_;
  std::vector<std::pair<int, int>> solution_;
};

}  // namespace nm
