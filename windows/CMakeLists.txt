cmake_minimum_required(VERSION 3.22.3)

# this is the windows port
project(umoria_windows LANGUAGES C)

# TODO: get this working with visual studio too (only tested w/mingw)
# this probably requires finding (vcpkg-provided) pdcurses libs or something

# derive some paths from the current directory
set(UMORIA_ROOT_DIR   "${CMAKE_CURRENT_LIST_DIR}/..")
set(UMORIA_SOURCE_DIR "${UMORIA_ROOT_DIR}/source")
set(UMORIA_FILES_DIR  "${UMORIA_ROOT_DIR}/files")
set(UMORIA_DOC_DIR    "${UMORIA_ROOT_DIR}/doc")
set(UMORIA_DIST_DIR   "${CMAKE_CURRENT_LIST_DIR}/dist")

# target for the umoria.exe binary
add_executable(moria
  ${UMORIA_SOURCE_DIR}/create.c
  ${UMORIA_SOURCE_DIR}/creature.c
  ${UMORIA_SOURCE_DIR}/death.c
  ${UMORIA_SOURCE_DIR}/desc.c
  ${UMORIA_SOURCE_DIR}/dungeon.c
  ${UMORIA_SOURCE_DIR}/eat.c
  ${UMORIA_SOURCE_DIR}/files.c
  ${UMORIA_SOURCE_DIR}/generate.c
  ${UMORIA_SOURCE_DIR}/help.c
  ${UMORIA_SOURCE_DIR}/io.c
  ${UMORIA_SOURCE_DIR}/magic.c
  ${UMORIA_SOURCE_DIR}/main.c
  ${UMORIA_SOURCE_DIR}/misc1.c
  ${UMORIA_SOURCE_DIR}/misc2.c
  ${UMORIA_SOURCE_DIR}/misc3.c
  ${UMORIA_SOURCE_DIR}/misc4.c
  ${UMORIA_SOURCE_DIR}/monsters.c
  ${UMORIA_SOURCE_DIR}/moria1.c
  ${UMORIA_SOURCE_DIR}/moria2.c
  ${UMORIA_SOURCE_DIR}/moria3.c
  ${UMORIA_SOURCE_DIR}/moria4.c
  ${UMORIA_SOURCE_DIR}/player.c
  ${UMORIA_SOURCE_DIR}/potions.c
  ${UMORIA_SOURCE_DIR}/prayer.c
  ${UMORIA_SOURCE_DIR}/recall.c
  ${UMORIA_SOURCE_DIR}/rnd.c
  ${UMORIA_SOURCE_DIR}/save.c
  ${UMORIA_SOURCE_DIR}/scrolls.c
  ${UMORIA_SOURCE_DIR}/sets.c
  ${UMORIA_SOURCE_DIR}/signals.c
  ${UMORIA_SOURCE_DIR}/spells.c
  ${UMORIA_SOURCE_DIR}/staffs.c
  ${UMORIA_SOURCE_DIR}/store1.c
  ${UMORIA_SOURCE_DIR}/store2.c
  ${UMORIA_SOURCE_DIR}/tables.c
  ${UMORIA_SOURCE_DIR}/treasure.c
  ${UMORIA_SOURCE_DIR}/variable.c
  ${UMORIA_SOURCE_DIR}/wands.c
  ${UMORIA_SOURCE_DIR}/wizard.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cp437utf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/ms_misc.c
  ${UMORIA_SOURCE_DIR}/config.h
  ${UMORIA_SOURCE_DIR}/constant.h
  ${UMORIA_SOURCE_DIR}/externs.h
  ${UMORIA_SOURCE_DIR}/types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cp437utf.h
)
target_include_directories(moria
  PUBLIC "${UMORIA_SOURCE_DIR}"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)
# set(PDCURSES_ROOT_DIR "${UMORIA_ROOT_DIR}/../PDCurses")
# set(PDCURSES_LIBS
#   "${PDCURSES_ROOT_DIR}/sdl2/pdcurses.a"
#   mingw32
#   SDL2main
#   SDL2
#   -mwindows
#   SDL2_ttf
#   mingw32
#   SDL2main
#   SDL2
#   -mwindows
# )
# set(PDCURSES_INCLUDE_DIR "${PDCURSES_ROOT_DIR}")
# target_link_libraries(moria
#   "${PDCURSES_LIBS}"
# )
# target_include_directories(moria PRIVATE ${PDCURSES_INCLUDE_DIR})
target_link_libraries(moria
  pdcurses_wincon
  winmm
)
# assemble the binary path+name programmatically so we can get it ASAP
# yes, this is probably hacky and fragile
get_target_property(UMORIA_BINARY_NAME moria NAME)
set(UMORIA_BINARY_NAME "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_EXECUTABLE_PREFIX}${UMORIA_BINARY_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
# strip release build binary
add_custom_command(
  TARGET moria POST_BUILD
  DEPENDS "${UMORIA_BINARY_NAME}"
  COMMAND $<$<CONFIG:release>:${CMAKE_STRIP}>
  ARGS "${UMORIA_BINARY_NAME}"
)
# copy files needed for running into the binary dir as a post-build step
# this is provided as a convenience for launching the game from an IDE
file(GLOB UMORIA_RUN_FILES
  "${CMAKE_CURRENT_LIST_DIR}/MORIA.CNF"
  "${UMORIA_FILES_DIR}/*"
)
foreach(UMORIA_RUN_FILE ${UMORIA_RUN_FILES})
  add_custom_command(
    TARGET moria POST_BUILD
    COMMENT "Copying ${UMORIA_RUN_FILE} to ${CMAKE_CURRENT_BINARY_DIR}..."
    COMMAND "${CMAKE_COMMAND}" -E copy "${UMORIA_RUN_FILE}" "${CMAKE_CURRENT_BINARY_DIR}"
    VERBATIM
  )
endforeach()

# follow-on logic for copying everything into dist folder for release purposes
# I know CMake has an "install" feature, but:
# - its IDE integration is terrible
# - the files shouldn't change much anyway
# - computers are fast enough now that it's easy to just clean rebuild as needed

# stuff that goes directly in dist/
set(UMORIA_DIST_FILES
#  $<TARGET_FILE:moria>
  "${UMORIA_BINARY_NAME}"
  ${UMORIA_RUN_FILES}
  "${CMAKE_CURRENT_LIST_DIR}/readme.txt"
)
# stuff that goes in dist/docs/
file(GLOB UMORIA_DOC_FILES
  "${UMORIA_DOC_DIR}/*"
)
# define custom copy commands for each file
# build depends list as we go
set(UMORIA_DIST_DEPENDS "")
# files in dist/
foreach(UMORIA_DIST_FILE ${UMORIA_DIST_FILES})
  # get stripped filename
  get_filename_component(UMORIA_TEMP_NAME "${UMORIA_DIST_FILE}" NAME)
  # prepend destination path
  set(UMORIA_TEMP_DEST "${UMORIA_DIST_DIR}/${UMORIA_TEMP_NAME}")
  add_custom_command(
    OUTPUT  "${UMORIA_TEMP_DEST}"
    DEPENDS "${UMORIA_DIST_FILE}"
    COMMENT "Copying dist     file ${UMORIA_TEMP_NAME}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${UMORIA_DIST_FILE}" "${UMORIA_DIST_DIR}"
    VERBATIM
  )
  # append output file to depends list
  set(UMORIA_DIST_DEPENDS ${UMORIA_DIST_DEPENDS} ${UMORIA_TEMP_DEST})
endforeach()
# files in dist/doc/
foreach(UMORIA_DOC_FILE ${UMORIA_DOC_FILES})
  # get stripped filename
  get_filename_component(UMORIA_TEMP_NAME "${UMORIA_DOC_FILE}" NAME)
  # prepend destination path
  set(UMORIA_TEMP_DEST "${UMORIA_DIST_DIR}/doc/${UMORIA_TEMP_NAME}")
  add_custom_command(
    OUTPUT  "${UMORIA_TEMP_DEST}"
    DEPENDS "${UMORIA_DOC_FILE}"
    COMMENT "Copying dist/doc file ${UMORIA_TEMP_NAME}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${UMORIA_DOC_FILE}" "${UMORIA_DIST_DIR}/doc"
    VERBATIM
  )
  # append output file to depends list
  set(UMORIA_DIST_DEPENDS ${UMORIA_DIST_DEPENDS} ${UMORIA_TEMP_DEST})
endforeach()
# make a target that depends on all of the custom commands
add_custom_target(dist ALL
  DEPENDS ${UMORIA_DIST_DEPENDS}
)
add_dependencies(dist moria)
# TODO: strip binary?
