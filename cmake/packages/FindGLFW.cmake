find_path(GLFW_INCLUDE_DIR GL/glfw3.h)
find_library(GLFW_LIBRARY NAMES ${GLFW_NAMES} libglfw3 glfw3 PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW DEFAULT_MSG GLFW_LIBRARY GLFW_INCLUDE_DIR)
mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARY)
