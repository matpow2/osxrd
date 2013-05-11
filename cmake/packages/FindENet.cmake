find_path(ENET_INCLUDE_DIR enet/enet.h)
find_library(ENET_LIBRARY NAMES enet PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ENET DEFAULT_MSG ENET_LIBRARY ENET_INCLUDE_DIR)
mark_as_advanced(ENET_INCLUDE_DIR ENET_LIBRARY)
