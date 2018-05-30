# NGM CMake file

project(ngm)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DENABLE_OBJECT_COUNTER") # <- to enable ObjectsCounter for memory leak seeking

set(LOG_DIR Logger)
set(AWS_SDK_DIR aws-iot-device-sdk)
set(MBEDTSL_DIR mbedTLS)
set(AWS_DIR Aws)
set(CLOUD_MGR_DIR CloudManager)
set(UTILS_DIR Utils)
set(SERIAL_COMM_DIR SerialComm)
set(MSP430_DIR Msp430)
set(GPS_DIR Gps)
set(ACCEL_DIR Accelerometer)
set(RTL_DIR Rtl8363)
set(TEMP_DIR TempSensor)
set(LEDS_DIR Leds)
set(DISCL_DIR DisclosingSensor)
set(LTE_DIR LteModem)
set(NMEA_DIR libnmea)
set(TINYXML_DIR tinyxml)
set(DEVMGR_DIR DeviceMgr)
set(CLI_DIR CLI)
set(SRC_DIR .)

set(CURR_CXX_FLAGS " ")

add_subdirectory(${MBEDTSL_DIR})

file(GLOB SRC_LIST ${SRC_DIR}/*.cpp)
set_source_files_properties(${SRC_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB LOG_LIST ${LOG_DIR}/*.cpp)
set_source_files_properties(${LOG_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB AWS_LIST ${AWS_DIR}/*.cpp)
set_source_files_properties(${AWS_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB CLOUD_MGR_LIST ${CLOUD_MGR_DIR}/*.cpp)
set_source_files_properties(${CLOUD_MGR_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB UTILS_LIST ${UTILS_DIR}/*.cpp)
set_source_files_properties(${UTILS_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB SERIAL_COMM_LIST ${SERIAL_COMM_DIR}/*.cpp)
set_source_files_properties(${SERIAL_COMM_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB AWS_SDK_LIST ${AWS_SDK_DIR}/*.c)

file(GLOB NMEA_LIST ${NMEA_DIR}/*.c)

file(GLOB CPP_AWS_LIST ${AWS_CPP_DIR}/*.c)

file(GLOB MSP430_LIST ${MSP430_DIR}/*.cpp)
set_source_files_properties(${MSP430_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB GPS_LIST ${GPS_DIR}/*.cpp)
set_source_files_properties(${GPS_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB ACCEL_LIST ${ACCEL_DIR}/*.cpp)
set_source_files_properties(${ACCEL_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB RTL_LIST ${RTL_DIR}/*.cpp)
set_source_files_properties(${RTL_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB LEDS_LIST ${LEDS_DIR}/*.cpp)
set_source_files_properties(${LEDS_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB DISCL_LIST ${DISCL_DIR}/*.cpp)
set_source_files_properties(${DISCL_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB LTE_LIST ${LTE_DIR}/*.cpp)
set_source_files_properties(${LTE_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB TEMP_LIST ${TEMP_DIR}/*.cpp)
set_source_files_properties(${TEMP_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB TINYXML_LIST ${TINYXML_DIR}/*.cpp)
set_source_files_properties(${TINYXML_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB DEVMGR_LIST ${DEVMGR_DIR}/*.cpp)
set_source_files_properties(${DEVMGR_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})
file(GLOB DEVMGR_C_LIST ${DEVMGR_DIR}/*.c)

file(GLOB CLI_LIST ${CLI_DIR}/*.cpp)
set_source_files_properties(${CLI_LIST} PROPERTIES COMPILE_FLAGS ${CURR_CXX_FLAGS})

file(GLOB INCLUDES
    ${AWS_SDK_DIR}/include/*.h
    ${AWS_CPP_DIR}/include/*/*/*.h
    ${AWS_CPP_DIR}/include/*/*/*/*.h
    ${AWS_CPP_DIR}/include/*/*/*/*/*.h
    ${AWS_DIR}/*.h
    ${UTILS_DIR}/*.h
    ${SQLITE3_DIR}/include/*.h
    ${SERIAL_COMM_DIR}/*.h
    ${DEVMGR_DIR}/*.h
    ${CLOUD_MGR_DIR}/*.h
    ${MSP430_DIR}/*.h
    ${ACCEL_DIR}/*.h
    ${RTL_DIR}/*.h
    ${TEMP_DIR}/*.h
    ${LEDS_DIR}/*.h
    ${GPS_DIR}/*.h
    ${DISCL_DIR}/*.h
    ${LTE_DIR}/*.h
    ${NMEA_DIR}/*.h
    ${TINYXML_DIR}/*.h
    ${DEVMGR_DIR}/*.h
    ${SRC_DIR}/*.h
    ${LOG_DIR}/*.h
)


INCLUDE_DIRECTORIES(
    ${AWS_SDK_DIR}/include
    ${AWS_CPP_DIR}/include
    ${MBEDTSL_DIR}/include
    ${SQLITE3_DIR}/include
    ${AWS_DIR}
    ${CLOUD_MGR_DIR}
    ${UTILS_DIR}
    ${SERIAL_COMM_DIR}
    ${MSP430_DIR}
    ${GPS_DIR}
    ${LEDS_DIR}
    ${ACCEL_DIR}
    ${RTL_DIR}
    ${TEMP_DIR}
    ${DISCL_DIR}
    ${LTE_DIR}
    ${NMEA_DIR}
    ${TINYXML_DIR}
    ${DEVMGR_DIR}
    ${CLI_DIR}
    ${SRC_DIR}
    ${LOG_DIR}
)

LINK_DIRECTORIES(
    ${AWS_CPP_DIR}/lib
    ${SQLITE3_DIR}/lib
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_IOT_INFO -DENABLE_IOT_WARN -DENABLE_IOT_ERROR -DENABLE_IOT_DEBUG")

add_executable(
    ${PROJECT_NAME}
    ${SRC_LIST}
    ${LOG_LIST}
    ${AWS_LIST}
    ${CLOUD_MGR_LIST}
    ${UTILS_LIST}
    ${SERIAL_COMM_LIST}
    ${AWS_SDK_LIST}
    ${CPP_AWS_LIST}
    ${MSP430_LIST}
    ${GPS_LIST}
    ${ACCEL_LIST}
    ${RTL_LIST}
    ${TEMP_LIST}
    ${LEDS_LIST}
    ${DISCL_LIST}
    ${LTE_LIST}
    ${NMEA_LIST}
    ${TINYXML_LIST}
    ${DEVMGR_LIST}
    ${DEVMGR_C_LIST}
    ${CLI_LIST}
    ${INCLUDES}
)

target_link_libraries(${PROJECT_NAME} mbedtls pthread mosquittopp mosquitto aws-cpp-sdk-s3 aws-cpp-sdk-core sqlite3)
