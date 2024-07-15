################# Add global include #################
list(APPEND ADD_INCLUDE
"${CMAKE_CURRENT_SOURCE_DIR}/hal_drv/inc"
"${CMAKE_CURRENT_SOURCE_DIR}/std_drv/inc"
"${CMAKE_CURRENT_SOURCE_DIR}/regs"
"${CMAKE_CURRENT_SOURCE_DIR}/startup"
"${CMAKE_CURRENT_SOURCE_DIR}"
)
#######################################################

################# Add private include #################
list(APPEND ADD_PRIVATE_INCLUDE
"${CMAKE_CURRENT_SOURCE_DIR}/hal_drv/default_config"
)
#######################################################

############## Add current dir source files ###########
file(GLOB_RECURSE sources
"${CMAKE_CURRENT_SOURCE_DIR}/std_drv/src/*.c"
"${CMAKE_CURRENT_SOURCE_DIR}/hal_drv/src/*.c"
"${CMAKE_CURRENT_SOURCE_DIR}/startup/interrupt.c"
"${CMAKE_CURRENT_SOURCE_DIR}/startup/system_bl702l.c"
"${CMAKE_CURRENT_SOURCE_DIR}/startup/GCC/entry.S"
"${CMAKE_CURRENT_SOURCE_DIR}/startup/GCC/start_load.c"
)
list(APPEND ADD_SRCS  ${sources})
# aux_source_directory(src ADD_SRCS)
list(REMOVE_ITEM ADD_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/std_drv/src/bl702l_snflash.c"
"${CMAKE_CURRENT_SOURCE_DIR}/std_drv/src/bl702l_romdriver.c"
)
#######################################################

########### Add required/dependent components #########
list(APPEND ADD_REQUIREMENTS common)
#######################################################

############ Add static libs ##########################
#list(APPEND ADD_STATIC_LIB "lib/libtest.a")
#######################################################

############ Add dynamic libs #########################
# list(APPEND ADD_DYNAMIC_LIB "lib/arch/v831/libmaix_nn.so"
#                             "lib/arch/v831/libmaix_cam.so"
# )
#######################################################

############ Add global compile option ################
#add components denpend on this component
if(CONFIG_ROMAPI)
list(APPEND ADD_DEFINITIONS -DBFLB_USE_ROM_DRIVER)
list(REMOVE_ITEM ADD_SRCS 
"${CMAKE_CURRENT_SOURCE_DIR}/std_drv/src/bl702l_hbn.c"
"${CMAKE_CURRENT_SOURCE_DIR}/std_drv/src/bl702l_l1c.c"
"${CMAKE_CURRENT_SOURCE_DIR}/std_drv/src/bl702l_glb_gpio.c"
"${CMAKE_CURRENT_SOURCE_DIR}/std_drv/src/bl702l_glb.c"
"${CMAKE_CURRENT_SOURCE_DIR}/std_drv/src/bl702l_ef_ctrl.c"
"${CMAKE_CURRENT_SOURCE_DIR}/std_drv/src/bl702l_clock.c"
)
else()
list(REMOVE_ITEM ADD_SRCS 
"${CMAKE_CURRENT_SOURCE_DIR}/std_drv/src/bl702l_romapi.c"
"${CMAKE_CURRENT_SOURCE_DIR}/std_drv/src/bl702l_romapi_patch.c"
)
endif()
if(CONFIG_HALAPI)
list(APPEND ADD_DEFINITIONS -DBFLB_USE_HAL_DRIVER)
endif()
list(APPEND ADD_DEFINITIONS -DARCH_RISCV)
if("${SUPPORT_FAST_WAKEUP}" STREQUAL "y")
list(APPEND ADD_DEFINITIONS -DFAST_WAKEUP)
endif()
#######################################################

############ Add private compile option ################
#add compile option for this component that won't affect other modules
# list(APPEND ADD_PRIVATE_DEFINITIONS -DAAAAA=1)
#######################################################

generate_library()
