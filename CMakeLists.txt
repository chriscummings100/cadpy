cmake_minimum_required(VERSION 3.15...3.26)

set(CMAKE_CXX_STANDARD 20)

project(cadpy LANGUAGES CXX)

if (NOT SKBUILD)
  message(WARNING "\
  This CMake file is meant to be executed using 'scikit-build'. Running
  it directly will almost certainly not produce the desired result. If
  you are a user trying to install this package, please use the command
  below, which will install all necessary build dependencies, compile
  the package in an isolated environment, and then install it.
  =====================================================================
   $ pip install .
  =====================================================================
  If you are a software developer, and this is your own package, then
  it is usually much more efficient to install the build dependencies
  in your environment once and use the following command that avoids
  a costly creation of a new virtual environment at every compilation:
  =====================================================================
   $ pip install nanobind scikit-build-core[pyproject]
   $ pip install --no-build-isolation -ve .
  =====================================================================
  You may optionally add -Ceditable.rebuild=true to auto-rebuild when
  the package is imported. Otherwise, you need to re-run the above
  after editing C++ files.")
endif()

# Try to import all Python components potentially needed by nanobind
find_package(Python 3.8
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)

# Import nanobind through CMake's find_package mechanism
#find_package(nanobind CONFIG REQUIRED)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/nanobind)

add_subdirectory(ext/fmt)

# We are now ready to compile the actual extension module
nanobind_add_module(
  # Name of the extension
  cadpy_ext

  # Target the stable ABI for Python 3.12+, which reduces
  # the number of binary wheels that must be built. This
  # does nothing on older Python versions
  STABLE_ABI

  # Build libnanobind statically and merge it into the
  # extension (which itself remains a shared library)
  #
  # If your project builds multiple extensions, you can
  # replace this flag by NB_SHARED to conserve space by
  # reusing a shared libnanobind across libraries
  NB_STATIC

  # Source code goes here
  src/cadpy_ext.cpp
  src/bsp.cpp
)

target_link_libraries(cadpy_ext PRIVATE fmt::fmt)

# Generate stub file as well
nanobind_add_stub(
  cadpy_ext_stub
  MODULE cadpy_ext
  OUTPUT $<CONFIG>/cadpy_ext.pyi
  PYTHON_PATH $<TARGET_FILE_DIR:cadpy_ext>
  MARKER_FILE $<CONFIG>/py.typed
  DEPENDS cadpy_ext
)

# Install directive for scikit-build-core
install(TARGETS cadpy_ext LIBRARY DESTINATION cadpy)
install(FILES $<TARGET_FILE_DIR:cadpy_ext>/py.typed $<TARGET_FILE_DIR:cadpy_ext>/cadpy_ext.pyi DESTINATION cadpy)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/__init__.py DESTINATION cadpy)
