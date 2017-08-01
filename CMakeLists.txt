cmake_minimum_required(VERSION 2.8.5)

set(PROJECT zsilencer)
project(${PROJECT})

include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

add_executable(${PROJECT} ${sources})
target_compile_options(${PROJECT} PUBLIC -std=c++1y)

find_package(ZLIB REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
include_directories(${SDL2_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIRS})

target_link_libraries(${PROJECT} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${ZLIB_LIBRARY})

install(TARGETS "${PROJECT}" RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(DIRECTORY "data/" DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT}")
install(FILES "res/icon_16.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/16x16/apps" RENAME "zsilencer.png")
install(FILES "res/icon_32.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps" RENAME "zsilencer.png")
install(FILES "res/icon_64.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps" RENAME "zsilencer.png")
install(FILES "res/icon_128.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps" RENAME "zsilencer.png")
install(FILES "zsilencer.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")

set(CPACK_PACKAGE_NAME "zSilencer")
set(CPACK_PACKAGE_VERSION "00022")

include(CPack)
