# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(tls)

# target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "cert/client_cert.pem" TEXT)
# target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "cert/client_key.pem" TEXT)
# target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "go_cert/go_server.pem" TEXT)

# target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "cert/ca.crt" TEXT)
# target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "cert/ca.key" TEXT)

target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "cert/ca_server_copy.crt" TEXT)