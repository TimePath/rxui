# FindRapidJSON.cmake
#
# Finds the gtk+-3.0 library
#
# This will define the following variables
#
#    GTK3_FOUND
#    GTK3_INCLUDE_DIRS
#    GTK3_VERSION
#
# and the following imported targets
#
#     GTK3::gtk
#

find_package(PkgConfig)
pkg_check_modules(PC_GTK3 QUIET gtk+-3.0)

find_path(GTK3_INCLUDE_DIR
        NAMES gtk/gtk.h
        PATHS ${PC_GTK3_INCLUDE_DIRS}
        )

set(GTK3_VERSION ${PC_GTK3_VERSION})

mark_as_advanced(GTK3_FOUND GTK3_INCLUDE_DIR GTK3_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GTK3
        REQUIRED_VARS GTK3_INCLUDE_DIR
        VERSION_VAR GTK3_VERSION
        )

if (GTK3_FOUND)
    set(GTK3_INCLUDE_DIRS ${GTK3_INCLUDE_DIR})
endif ()

if (GTK3_FOUND AND NOT TARGET GTK3::gtk)
    add_library(GTK3::gtk INTERFACE IMPORTED)
    set_target_properties(GTK3::gtk PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES
            "${PC_GTK3_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES
            "${PC_GTK3_LIBRARIES}"
            )
endif ()
