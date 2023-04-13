# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO powerof3/CLibUtil
    REF 52947fda394cdbaa9bf59ac54d03a7ee417c3a1c
    SHA512 ad8718449b191e5ae2421110df33f3c47852dd4d82bff3460a206cedf8eebb1fcb1de2db397bd5192402e0a1a417667cd4d1ff59ba3d1268ab44f53aa25bc6de
    HEAD_REF master
)

# Install codes
set(CLIBUTIL_SOURCE	${SOURCE_PATH}/include/ClibUtil)
file(INSTALL ${CLIBUTIL_SOURCE} DESTINATION ${CURRENT_PACKAGES_DIR}/include)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
