// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <string>
#include <vector>

// Lo poco que el actualizador necesita del sistema operativo. Los argumentos
// se pasan como lista, nunca como una línea de órdenes para un intérprete.
namespace nm::proc {

// Ejecuta un programa sin ventana y espera a que termine. Devuelve su código
// de salida, o -1 si no se pudo lanzar.
int run(const std::vector<std::string>& args);
// Lanza un programa que sigue vivo cuando el juego se cierra.
bool spawnDetached(const std::vector<std::string>& args);

std::string tempDir();
std::string executablePath();
long pid();
bool dirWritable(const std::string& dir);

}  // namespace nm::proc
