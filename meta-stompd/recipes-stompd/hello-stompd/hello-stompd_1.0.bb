SUMMARY = "Qt Quick hello world for stompd"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESEXTRAPATHS:prepend := "${THISDIR}/../../..:"
SRC_URI = "file://src file://hello-stompd.service"
S = "${UNPACKDIR}/src"

SYSTEMD_SERVICE:${PN} = "hello-stompd.service"

inherit qt6-cmake systemd useradd

DEPENDS += "qtbase qtdeclarative qtdeclarative-native"
RDEPENDS:${PN} += "qtbase-plugins qtdeclarative-qmlplugins ttf-dejavu-sans stompd-qt-config"

USERADD_PACKAGES = "${PN}"
GROUPADD_PARAM:${PN} = "-r input; -r render"
USERADD_PARAM:${PN} = "--system --no-create-home --shell /sbin/nologin --groups audio,video,render,input stompd"

do_install:append() {
    install -Dm 0644 ${UNPACKDIR}/hello-stompd.service ${D}${systemd_system_unitdir}/hello-stompd.service
}
