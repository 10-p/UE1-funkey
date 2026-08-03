project(Unreal)

if(USE_SDL)
  set(SRC_FILES "Src/SDLLaunch.cpp")
else()
  set(SRC_FILES "Src/WinLaunch.cpp")
endif()

if(FUNKEY)
  enable_language(C)
endif()

if(VITA)
  set(VITA_APP_NAME "Unreal")
  set(VITA_TITLEID  "UNRL00001")
  set(VITA_MKSFOEX_FLAGS "-d ATTRIBUTE2=12")
  include("${VITASDK}/share/vita.cmake" REQUIRED)
endif()

if(TARGET_IS_WINDOWS)
  list(APPEND SRC_FILES "Src/Res/LaunchRes.rc")
  add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES})
else()
  add_executable(${PROJECT_NAME} ${SRC_FILES})
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Src
)

if(USE_SDL)
  target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} NSDLDrv)
  target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIR})
elseif(TARGET_IS_WINDOWS)
  target_link_libraries(${PROJECT_NAME} Window)
else()
  message(FATAL_ERROR "Non-Windows platforms can only use NSDLDrv.")
endif()

if(BUILD_STATIC)
  set(LINK_PACKAGES ${INSTALL_TARGETS})
  list(REMOVE_ITEM LINK_PACKAGES ${PROJECT_NAME})
  target_link_libraries(${PROJECT_NAME} -Wl,--whole-archive ${LINK_PACKAGES} -Wl,--no-whole-archive)
else()
  target_link_libraries(${PROJECT_NAME} Engine Core)
endif()

if(VITA)
  target_link_libraries(${PROJECT_NAME}
    -Wl,--whole-archive pthread -Wl,--no-whole-archive
    openal
    xmp
    vitaGL
    vitashark
    mathneon
    SceShaccCgExt
    taihen_stub
    SceNet_stub
    SceNetCtl_stub
    SceAppMgr_stub
    SceAppUtil_stub
    SceAudio_stub
    SceAudioIn_stub
    SceCtrl_stub
    SceCommonDialog_stub
    SceDisplay_stub
    SceFios2_stub
    SceGxm_stub
    SceIme_stub
    SceShaccCg_stub
    SceSysmodule_stub
    ScePower_stub
    SceTouch_stub
    SceMotion_stub
    SceHid_stub
    SceVshBridge_stub
    SceSblSsMgr_stub
    SceKernelDmacMgr_stub
  )
elseif(FUNKEY)
  target_link_libraries(${PROJECT_NAME}
    -Wl,--whole-archive pthread -Wl,--no-whole-archive
    ${OPENAL_LIBRARY}
    ${LIBXMP_LIBRARY}
    m
    dl
    asound
  )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE UNREAL_EXPORTS UPACKAGE_NAME=${PROJECT_NAME})

if(VITA)
  vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)
  vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE ${CMAKE_CURRENT_SOURCE_DIR}/Res/sce_sys sce_sys
  )
elseif(FUNKEY)
  set(FUNKEY_DIR "${CMAKE_SOURCE_DIR}/../FunKey")
  set(OPK_STAGING "${CMAKE_BINARY_DIR}/opk-staging")
  set(OPK_OUTPUT "${CMAKE_BINARY_DIR}/unreal_funkey-s.opk")

  add_custom_target(package-opk
    # Clean and create staging directory
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${OPK_STAGING}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OPK_STAGING}/lib"

    # Binary and FunKey metadata
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${PROJECT_NAME}>" "${OPK_STAGING}/Unreal.bin"
    COMMAND ${CMAKE_COMMAND} -E copy "${FUNKEY_DIR}/unreal.funkey-s.desktop" "${OPK_STAGING}/"
    COMMAND ${CMAKE_COMMAND} -E copy "${FUNKEY_DIR}/unreal.png" "${OPK_STAGING}/"
    COMMAND ${CMAKE_COMMAND} -E copy "${FUNKEY_DIR}/readme.txt" "${OPK_STAGING}/"
    COMMAND ${CMAKE_COMMAND} -E copy "${FUNKEY_DIR}/unreal_launcher.sh" "${OPK_STAGING}/"
    COMMAND chmod +x "${OPK_STAGING}/Unreal.bin" "${OPK_STAGING}/unreal_launcher.sh"

    # Bundle shared libraries from funkey-deps
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${FUNKEY_DEPS_PREFIX}/lib" "${OPK_STAGING}/lib"

    # Bundle OpenAL from SDK sysroot
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SYSROOT}/usr/lib/libopenal.so.1" "${OPK_STAGING}/lib/"

    # Create OPK
    COMMAND mksquashfs "${OPK_STAGING}" "${OPK_OUTPUT}"
            -all-root -noappend -no-exports -no-xattrs -no-progress

    DEPENDS ${PROJECT_NAME}
    COMMENT "Packaging FunKey OPK -> ${OPK_OUTPUT}"
  )
endif()
