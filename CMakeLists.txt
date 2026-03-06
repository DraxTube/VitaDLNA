cmake_minimum_required(VERSION 3.14)

project(VitaDLNAReceiver VERSION 1.0.0)

include(${VITASDK}/share/vita.cmake REQUIRED)

set(VITA_APP_NAME "VitaDLNA")
set(VITA_TITLEID  "VDLNA0001")
set(VITA_VERSION  "01.00")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

add_executable(${PROJECT_NAME}
    src/main.c
    src/dlna_server.c
    src/ssdp.c
    src/soap_server.c
    src/http_server.c
    src/video_player.c
    src/network.c
    src/utils.c
)

target_link_libraries(${PROJECT_NAME}
    SceLibKernel_stub
    SceSysmodule_stub
    SceNet_stub
    SceNetCtl_stub
    SceDisplay_stub
    SceCtrl_stub
    SceAppMgr_stub
    SceAppUtil_stub
    SceAvPlayer_stub
    SceAudioOut_stub
    SceVshBridge_stub
    m
    c
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})

# Configurazione VPK semplificata SENZA icone
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
)
