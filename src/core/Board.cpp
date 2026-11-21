// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "core/Board.hpp"

#include <algorithm>
#include <cstdlib>
#include <map>
#include <random>

namespace nm {
namespace {

using Pair = std::pair<int, int>;

// Mezcla propia: std::shuffle no da el mismo resultado en todas las
// plataformas y el reto diario tiene que ser igual para todos.
template <typename T>
void shuffleVec(std::vector<T>& v, std::mt19937& rng) {
  for (std::size_t i = v.size(); i > 1; --i) std::swap(v[i - 1], v[rng() % i]);
}

bool freeIn(const std::vector<Tile>& tiles, int index) {
  const Tile& t = tiles[index];
  if (t.removed) return false;
  bool left = false, right = false;
  for (int i = 0; i < static_cast<int>(tiles.size()); ++i) {
    const Tile& o = tiles[i];
    if (i == index || o.removed) continue;
    const int dx = o.pos.x - t.pos.x;
    const int dy = o.pos.y - t.pos.y;
    if (std::abs(dy) >= 2) continue;
    if (o.pos.z > t.pos.z && std::abs(dx) < 2) return false;
    if (o.pos.z == t.pos.z) {
      if (dx == -2) left = true;
      if (dx == 2) right = true;
    }
  }
  return !(left && right);
}

// Parejas de caras para `count` fichas. El juego completo son 72 parejas.
std::vector<Pair> facePairs(int count, std::mt19937& rng) {
  std::vector<Pair> pairs;
  for (int f = 0; f < 34; ++f) {
    pairs.push_back({f, f});
    pairs.push_back({f, f});
  }
  pairs.push_back({34, 35});
  pairs.push_back({36, 37});
  pairs.push_back({38, 39});
  pairs.push_back({40, 41});
  shuffleVec(pairs, rng);
  std::vector<Pair> out;
  for (int i = 0; i < count / 2; ++i) out.push_back(pairs[i % pairs.size()]);
  return out;
}

// Asigna las parejas "pelando" el tablero: en cada paso elige dos fichas
// libres, les da la misma pareja y las retira. El orden de retirada es, por
// construcción, una solución válida.
bool assignSolvable(std::vector<Tile>& tiles, const std::vector<int>& indices, std::vector<Pair> pairs,
                    std::mt19937& rng, std::vector<Pair>* order, bool spread = false) {
  std::vector<Tile> work = tiles;
  std::vector<int> left = indices;
  std::vector<Pair> removal;
  shuffleVec(pairs, rng);
  for (const Pair& faces : pairs) {
    std::vector<int> open;
    for (int i : left)
      if (freeIn(work, i)) open.push_back(i);
    if (open.size() < 2) return false;
    shuffleVec(open, rng);
    const int a = open[0];
    int b = open[1];
    if (spread) {
      // Difícil: la pareja se busca lejos, entre las tres fichas libres más distantes.
      auto distance = [&](int i) {
        const Slot& p = work[a].pos;
        const Slot& q = work[i].pos;
        return std::abs(p.x - q.x) + std::abs(p.y - q.y) + 3 * std::abs(p.z - q.z);
      };
      std::sort(open.begin() + 1, open.end(), [&](int i, int j) {
        return distance(i) != distance(j) ? distance(i) > distance(j) : i < j;
      });
      b = open[1 + rng() % std::min<std::size_t>(3, open.size() - 1)];
    }
    work[a].face = faces.first;
    work[b].face = faces.second;
    work[a].removed = work[b].removed = true;
    removal.push_back({a, b});
    std::erase(left, a);
    std::erase(left, b);
  }
  for (int i : indices) tiles[i].face = work[i].face;
  if (order) *order = removal;
  return true;
}

}  // namespace

bool Board::generate(const Layout& layout, std::uint32_t seed, Difficulty difficulty) {
  tiles_.clear();
  history_.clear();
  solution_.clear();
  if (layout.slots.empty() || layout.slots.size() % 2 != 0) return false;
  for (const Slot& s : layout.slots) tiles_.push_back({s, 0, false});

  std::mt19937 rng(seed);
  std::vector<int> all(tiles_.size());
  for (int i = 0; i < static_cast<int>(all.size()); ++i) all[i] = i;
  const std::vector<Pair> pairs = facePairs(static_cast<int>(tiles_.size()), rng);
  for (int attempt = 0; attempt < 500; ++attempt)
    if (assignSolvable(tiles_, all, pairs, rng, &solution_, difficulty == Difficulty::Hard)) return true;
  return false;
}

bool Board::restore(const Layout& layout, const std::vector<int>& faces, const std::vector<int>& removed,
                    const std::vector<Pair>& history) {
  const std::size_t n = layout.slots.size();
  if (n == 0 || faces.size() != n || removed.size() != n) return false;
  std::vector<Tile> tiles;
  std::map<int, int> groups;
  for (std::size_t i = 0; i < n; ++i) {
    if (faces[i] < 0 || faces[i] >= kFaceCount) return false;
    tiles.push_back({layout.slots[i], faces[i], removed[i] != 0});
    if (!tiles.back().removed) ++groups[matchGroup(faces[i])];
  }
  for (const auto& [group, count] : groups)
    if (count % 2 != 0) return false;  // quedaría una ficha sin pareja
  for (const Pair& p : history) {
    const bool inRange = p.first >= 0 && p.second >= 0 && p.first < static_cast<int>(n) && p.second < static_cast<int>(n);
    if (!inRange || p.first == p.second || !tiles[p.first].removed || !tiles[p.second].removed) return false;
  }
  tiles_ = std::move(tiles);
  history_ = history;
  solution_.clear();
  return true;
}

bool Board::isFree(int index) const {
  if (index < 0 || index >= static_cast<int>(tiles_.size())) return false;
  return freeIn(tiles_, index);
}

bool Board::canMatch(int a, int b) const {
  if (a == b || !isFree(a) || !isFree(b)) return false;
  return matchGroup(tiles_[a].face) == matchGroup(tiles_[b].face);
}

bool Board::removePair(int a, int b) {
  if (!canMatch(a, b)) return false;
  tiles_[a].removed = tiles_[b].removed = true;
  history_.push_back({a, b});
  return true;
}

std::optional<Pair> Board::undo() {
  if (history_.empty()) return std::nullopt;
  const Pair last = history_.back();
  history_.pop_back();
  tiles_[last.first].removed = tiles_[last.second].removed = false;
  return last;
}

std::optional<Pair> Board::findHint() const {
  std::map<int, int> seen;  // grupo -> primera ficha libre vista
  for (int i = 0; i < static_cast<int>(tiles_.size()); ++i) {
    if (!freeIn(tiles_, i)) continue;
    const int group = matchGroup(tiles_[i].face);
    if (auto it = seen.find(group); it != seen.end()) return Pair{it->second, i};
    seen[group] = i;
  }
  return std::nullopt;
}

int Board::availableMoves() const {
  std::map<int, int> counts;
  for (int i = 0; i < static_cast<int>(tiles_.size()); ++i)
    if (freeIn(tiles_, i)) ++counts[matchGroup(tiles_[i].face)];
  int moves = 0;
  for (const auto& [group, n] : counts) moves += n * (n - 1) / 2;
  return moves;
}

int Board::remaining() const {
  return static_cast<int>(
      std::count_if(tiles_.begin(), tiles_.end(), [](const Tile& t) { return !t.removed; }));
}

bool Board::shuffle(std::uint32_t seed) {
  std::mt19937 rng(seed);
  std::vector<int> left;
  std::map<int, std::vector<int>> byGroup;
  for (int i = 0; i < static_cast<int>(tiles_.size()); ++i) {
    if (tiles_[i].removed) continue;
    left.push_back(i);
    byGroup[matchGroup(tiles_[i].face)].push_back(tiles_[i].face);
  }
  std::vector<Pair> pairs;
  for (auto& [group, faces] : byGroup)
    for (std::size_t i = 0; i + 1 < faces.size(); i += 2) pairs.push_back({faces[i], faces[i + 1]});
  if (pairs.size() * 2 != left.size()) return false;

  history_.clear();  // deshacer tras barajar devolvería fichas con caras cambiadas
  for (int attempt = 0; attempt < 300; ++attempt)
    if (assignSolvable(tiles_, left, pairs, rng, nullptr)) return true;

  // No hay orden de retirada completo: se reparte al azar y se comprueba si
  // al menos queda algún movimiento.
  std::vector<int> faces;
  for (const Pair& p : pairs) {
    faces.push_back(p.first);
    faces.push_back(p.second);
  }
  for (int attempt = 0; attempt < 50; ++attempt) {
    shuffleVec(faces, rng);
    for (std::size_t i = 0; i < left.size(); ++i) tiles_[left[i]].face = faces[i];
    if (availableMoves() > 0) return true;
  }
  return false;
}

}  // namespace nm
