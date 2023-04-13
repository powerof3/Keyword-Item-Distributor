# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO powerof3/CLibUtil
    REF 50fe06a58a293ba0ba708bc2d2d5d2ae3edabce6
    SHA512 b232292a22ac54e97da295446e1dc99fce6c2c4d42739fbcdc260d295d5dcf3fab735042325db56558725463ef25143bdeea37b0baa11af024c8be3b06715b22
    HEAD_REF master
)

# Install codes
set(CLIBUTIL_SOURCE	${SOURCE_PATH}/include/ClibUtil)
file(INSTALL ${CLIBUTIL_SOURCE} DESTINATION ${CURRENT_PACKAGES_DIR}/include)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
