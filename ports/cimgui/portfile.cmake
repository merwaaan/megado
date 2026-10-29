vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO cimgui/cimgui
    REF b6563e1b115a08e4e6651576420600df225e0cae
    SHA512 662687ce85a038e629e84fb67d096138da2614e16947ce2638627b22e5d95af86b73bcdb6cac98fcb56beb71c9eb8eafbd483cf34b30659748b36b0a8d26676a
    HEAD_REF docking_inter
)

vcpkg_from_github(
    OUT_SOURCE_PATH IMGUI_SOURCE_PATH
    REPO ocornut/imgui
    REF 6ded5230d043aa32c755e65c910c2af5002fb9f9
    SHA512 50ed04eaf96c052e13f179b560507a3e6719564d59233169ded22244002f33f8c8c1ca4ddb6d44c9a831482367c256582d4ff6df8ef5c7bcf36412821df9a83e
    HEAD_REF docking
)

file(REMOVE_RECURSE "${SOURCE_PATH}/imgui")
file(COPY "${IMGUI_SOURCE_PATH}/" DESTINATION "${SOURCE_PATH}/imgui")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE" "${SOURCE_PATH}/imgui/LICENSE.txt")
