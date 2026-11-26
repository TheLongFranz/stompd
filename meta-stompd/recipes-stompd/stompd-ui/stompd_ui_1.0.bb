SUMMARY = "stompd user interface"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESEXTRAPATHS:prepend := "${THISDIR}/../../..:"
SRC_URI = "file://ui file://stompd_ui.service"
S = "${UNPACKDIR}/ui"

SYSTEMD_SERVICE:${PN} = "stompd_ui.service"

inherit qt6-cmake systemd useradd

DEPENDS += "qtbase qtdeclarative qtdeclarative-native"
RDEPENDS:${PN} += "qtbase-plugins qtdeclarative-qmlplugins ttf-dejavu-sans stompd-qt-config"

USERADD_PACKAGES = "${PN}"
GROUPADD_PARAM:${PN} = "--system stompd"
USERADD_PARAM:${PN} = "--system --no-create-home --shell /bin/false --gid stompd --groups video stompd-ui"

do_install:append() {
    install -Dm 0644 ${UNPACKDIR}/stompd_ui.service ${D}${systemd_system_unitdir}/stompd_ui.service
}
