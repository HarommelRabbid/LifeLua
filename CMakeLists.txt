## This file is a quick tutorial on writing CMakeLists for targeting the Vita
cmake_minimum_required(VERSION 3.16)

## This includes the Vita toolchain, must go before project definition
# It is a convenience so you do not have to type
# -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake for cmake. It is
# highly recommended that you include this block for all projects.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

## Define project parameters here
# Name of the project
project(LifeLua)
# This line adds Vita helper macros, must go after project definition in order
# to build Vita specific artifacts (self/vpk).
include("${VITASDK}/share/vita.cmake" REQUIRED)

## Configuration options for this app
# Display name (under bubble in LiveArea)
set(VITA_APP_NAME "LifeLua")
# Unique ID must be exactly 9 characters. Recommended: XXXXYYYYY where X =
# unique string of developer and Y = a unique number for this app
set(VITA_TITLEID  "LIFELUAJT")
# Optional version string to show in LiveArea's more info screen
set(VITA_VERSION  "00.00")

## Flags and includes for building
# Note that we make sure not to overwrite previous flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wl,-q -g -fno-lto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -std=c++11 -fno-exceptions -fno-lto")
# Optional. You can specify more param.sfo flags this way.
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS} -a 0x2800000000000001")

# Add any additional include paths here
include_directories(
  ${VITASDK}/arm-vita-eabi/include/luajit-2.1
)

# Add any additional library paths here
# ${CMAKE_CURRENT_BINARY_DIR} lets you use any library currently being built
link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

## Build and link
# Add all the files needed to compile here
add_executable(${PROJECT_NAME}
  src/lj_lifemain.c
  src/lj_lifetimer.c
  src/lj_lifeio.c
  src/lj_lifeos.c
  src/lj_lifenetwork.c
  src/lj_lifecontrols.c
  src/lj_lifesqlite.c
  src/lj_lifedraw.c
  src/lj_lifeimage.c
  src/lj_lifecolor.c
  src/lj_lifefont.c
  src/lj_lifecamera.c
  src/lj_lifethread.c
  src/lj_lifejson.c
  src/lj_lifestring.c
  src/lj_lifeimgui.c++
  src/lj_lifeaudio.c
  src/lj_lifevideo.c
  src/ext/lutf8lib.c
  src/include/ftpvita.c
  src/include/sha1.c
  src/include/md5.c
  src/include/makeheadbin.c
  src/include/ioapi.c
  src/include/zip.c
  src/include/unzip.c
  src/include/qrcodegen.c
  src/include/cJSON.c
  src/include/sha256.c
  src/include/vitaaudiolib.c
  src/include/libtimer.c
)

# Library to link to (drop the -l prefix). This will mostly be stubs.
target_link_libraries(${PROJECT_NAME}
    quirc
    vita2d
    sqlite
    imgui_vita2d
    SceLibJson_stub
    SceDisplay_stub
    SceGxm_stub
    SceLibKernel_stub
    SceSysmodule_stub
    SceCtrl_stub
    SceCamera_stub
    SceCommonDialog_stub
    SceIme_stub
    SceFios2_stub
    SceVshBridge_stub
    SceShellSvc_stub
    SceMusicExport_stub
    ScePhotoExport_stub
    SceVideoExport_stub
    ScePspnetAdhoc_stub
    SceTouch_stub
    SceMotion_stub
    ScePgf_stub
    ScePvf_stub
    SceAppMgr_stub
    SceAppUtil_stub
    ScePromoterUtil_stub
    SceKernelDmacMgr_stub
    SceHttp_stub
    SceSsl_stub
    SceNet_stub
    SceNetCtl_stub
    SceKernelModulemgr_stub
    SceNotificationUtil_stub
    SceScreenShot_stub
    SceShutterSound_stub
    SceRegistryMgr_stub
    ScePower_stub
    SceAudio_stub
    SceAtrac_stub
    SceAudiodec_stub
    SceAvcodec_stub
    SceVideodec_stub
    SceAvPlayer_stub
    SceHid_stub
    SceUdcd_stub
    SceAVConfig_stub
    SceSqlite_stub
    luajit-5.1
    dl
    taihen_stub
    SceSblSsMgr_stub
    png
    jpeg
    imagequant
    avcodec
    avutil
    avformat
    freetype
    mpg123
    opusfile
    opusurl
    opus
    vorbisfile
    vorbis
    ogg
    c
    m
    z
    #k
    bz2
    pthread
)

## Create Vita files
#vita_create_self(safe_eboot.bin ${PROJECT_NAME})
vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)
# The FILE directive lets you add additional files to the VPK, the syntax is
# FILE src_path dst_path_in_vpk. In this case, we add the LiveArea paths.
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  #FILE sce_sys/icon0.png sce_sys/icon0.png
  #FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  #FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  #FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
