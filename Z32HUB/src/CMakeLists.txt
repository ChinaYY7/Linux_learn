SET(SRC_LIST main.c usb_hub.c Deal_Error.c Panel.c ICP209_cmd.c)
ADD_EXECUTABLE(Z32HUB.bin ${SRC_LIST})
SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread")
INCLUDE_DIRECTORIES(/usr/local/include/libusb-1.0 ${CMAKE_CURRENT_SOURCE_DIR})
#MESSAGE(STATUS "This is SOURCE dir " ${CMAKE_CURRENT_SOURCE_DIR})
#TARGET_LINK_LIBRARIES(Z32HUB.bin /usr/local/lib/libusb-1.0.so)
TARGET_LINK_LIBRARIES(Z32HUB.bin libusb-1.0.so)