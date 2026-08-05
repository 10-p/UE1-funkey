project(SoftDrv CXX)

set(SRC_FILES
  "Src/DrawPoly.cpp"
  "Src/DrawSurf.cpp"
  "Src/DrawTile.cpp"
  "Src/Hit.cpp"
  "Src/Line2D.cpp"
  "Src/SoftDrv.cpp"
)

add_library(${PROJECT_NAME} ${LIB_TYPE} ${SRC_FILES})

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Src
)

target_link_libraries(${PROJECT_NAME} Render Engine Core)

target_compile_definitions(${PROJECT_NAME} PRIVATE SOFTDRV_EXPORTS UPACKAGE_NAME=${PROJECT_NAME})

# The SizeX/SizeY-indexed scratch tables are STATIC arrays, so their bounds cost .bss at every
# resolution, not just at the ceiling: roughly 108 bytes per unit (84 on the Y-indexed Setup/MMXSetup/
# SpanIndex, 24 on the X-indexed UCoordTable/Photon/FogPhoton). 2048 costs ~221 KB, 4096 ~442 KB,
# 256 ~28 KB -- worth trimming on a memory-constrained target.
#
# Default 2048 keeps the X bound this tree already shipped. Do not trim it below a width the target
# can actually open: DrawPoly fail-softs on Y, but nothing in this fork clamps X. See the rationale
# in Src/SoftDrvPrivate.h.
set(SOFTDRV_MAX_SCREEN "2048" CACHE STRING "Max software-renderer viewport dimension (static table bound)")
target_compile_definitions(${PROJECT_NAME} PRIVATE SOFTDRV_MAX_SCREEN=${SOFTDRV_MAX_SCREEN})
