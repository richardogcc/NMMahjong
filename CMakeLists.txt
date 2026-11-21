cmake_minimum_required(VERSION 3.24)
project(NMMahjong VERSION 0.6.0 LANGUAGES C CXX)
if(WIN32)
  enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(NM_BUILD_TESTS "Compila las pruebas de la lógica del juego" ON)
# Repositorio de GitHub cuyas «releases» consulta el actualizador del juego.
set(NM_UPDATE_REPO "richardogcc/NMMahjong" CACHE STRING "usuario/repositorio de las versiones publicadas")

# --- raylib (zlib/libpng) -------------------------------------------------
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_Declare(raylib
  URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(raylib)
# Los fondos son fotografías en JPG, formato que raylib trae desactivado.
target_compile_definitions(raylib PRIVATE SUPPORT_FILEFORMAT_JPG=1)

# --- Lógica pura (sin dependencias gráficas) ------------------------------
add_library(nmcore STATIC
  src/core/Layout.cpp
  src/core/Board.cpp
  src/core/Release.cpp
  src/core/Season.cpp)
target_include_directories(nmcore PUBLIC src)

# --- Juego ----------------------------------------------------------------
add_executable(NMMahjong MACOSX_BUNDLE WIN32
  src/main.cpp
  src/audio/Audio.cpp
  src/audio/Music.cpp
  src/render/Background.cpp
  src/render/Particles.cpp
  src/render/TileAtlas.cpp
  src/render/TileFaces.cpp
  src/ui/Ui.cpp
  src/game/Save.cpp
  src/game/Strings.cpp
  src/game/StringsEsEn.cpp
  src/game/StringsFrDe.cpp
  src/game/StringsPtZh.cpp
  src/game/Themes.cpp
  src/game/Menu.cpp
  src/game/Panels.cpp
  src/game/Options.cpp
  src/game/Game.cpp
  src/game/GameDraw.cpp
  src/game/GamePlay.cpp
  src/update/Process.cpp
  src/update/Updater.cpp)
target_link_libraries(NMMahjong PRIVATE nmcore raylib)
target_compile_definitions(NMMahjong PRIVATE
  NM_VERSION="${PROJECT_VERSION}"
  NM_UPDATE_REPO="${NM_UPDATE_REPO}")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(nmcore PRIVATE -Wall -Wextra)
  target_compile_options(NMMahjong PRIVATE -Wall -Wextra -Wno-missing-field-initializers)
endif()

if(APPLE)
  target_sources(NMMahjong PRIVATE assets/icon/icon.icns)
  set_source_files_properties(assets/icon/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set_target_properties(NMMahjong PROPERTIES
    MACOSX_BUNDLE_ICON_FILE "icon.icns"
    MACOSX_BUNDLE_BUNDLE_NAME "NM Mahjong"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.richardogcm.nmmahjong"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "© 2026 Richardo G. Cibea M. Todos los derechos reservados.")
  target_link_libraries(NMMahjong PRIVATE "-framework CoreFoundation")
  set(NM_ASSET_DEST "$<TARGET_BUNDLE_CONTENT_DIR:NMMahjong>/Resources/assets")
else()
  set(NM_ASSET_DEST "$<TARGET_FILE_DIR:NMMahjong>/assets")
endif()

if(WIN32)
  target_sources(NMMahjong PRIVATE src/platform/windows.rc)
  set_source_files_properties(src/platform/windows.rc PROPERTIES
    COMPILE_FLAGS "-I\"${CMAKE_SOURCE_DIR}\"")
endif()

if(UNIX AND NOT APPLE)
  # Linux: el runtime de C++ va dentro para depender solo de glibc, X11 y OpenGL.
  target_link_options(NMMahjong PRIVATE -static-libgcc -static-libstdc++ -s)
endif()

if(MINGW)
  # Un único .exe sin DLL de runtime que acompañar.
  target_link_options(NMMahjong PRIVATE -static -static-libgcc -static-libstdc++ -s)
endif()

add_custom_command(TARGET NMMahjong POST_BUILD
  # Se borra antes para que no queden recursos de versiones anteriores.
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${NM_ASSET_DEST}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets" "${NM_ASSET_DEST}"
  COMMENT "Copiando recursos")

# --- Pruebas --------------------------------------------------------------
if(NM_BUILD_TESTS AND NOT CMAKE_CROSSCOMPILING)
  enable_testing()
  add_executable(board_tests tests/board_tests.cpp)
  target_link_libraries(board_tests PRIVATE nmcore)
  add_test(NAME board_tests COMMAND board_tests)
endif()
