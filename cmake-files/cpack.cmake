### CPack Convenience Macro

MACRO(setup_cpack)
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OMNIDOME is a arbitrary surface projection mapping tool built by CR8TR. It allows you to map any physical room and project virtual content back on to it.")

  set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
  set(CPACK_PACKAGE_VENDOR "CR8TR GmbH")
  set(CPACK_PACKAGE_VERSION "${VERSION_STRING}")
  set(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
  set(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
  set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")

  IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # If mac os x
    set(CPACK_BINARY_DRAGNDROP ON)
    set(CPACK_DMG_VOLUME_NAME ${CMAKE_PROJECT_NAME})
  ELSE()
    SET(CPACK_GENERATOR "DEB")
  ENDIF()

  SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <m@cr8tr.org>") #required

  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
  SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

  include(CPack)
ENDMACRO(setup_cpack)
