cmake_minimum_required(VERSION 3.15)
project(VulkanProject LANGUAGES CXX)

# Configuración general
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluir directorios de librerías locales y código fuente
include_directories(${CMAKE_SOURCE_DIR}/libs)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Buscar Vulkan en el sistema
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Recoger todos los .cpp en src y subcarpetas
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Crear ejecutable
add_executable(${PROJECT_NAME} ${SOURCES})

# Enlazar Vulkan y GLFW
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw)

# --- Compilación automática de shaders ---
file(GLOB SHADERS ${CMAKE_SOURCE_DIR}/src/shaders/*.*)
set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(GLOB SHADERS ${CMAKE_SOURCE_DIR}/src/shaders/*.*)
foreach(SHADER ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPV_FILE ${SHADER_OUTPUT_DIR}/${FILE_NAME}.spv)

    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
        COMMAND glslc ${SHADER} -o ${SPV_FILE}
        DEPENDS ${SHADER}
    )

    # Asociar cada shader compilado al ejecutable
    add_custom_target(${FILE_NAME}_spv DEPENDS ${SPV_FILE})
    add_dependencies(${PROJECT_NAME} ${FILE_NAME}_spv)
endforeach()

# Copiar shaders al mismo directorio que el ejecutable
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHADER_OUTPUT_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)