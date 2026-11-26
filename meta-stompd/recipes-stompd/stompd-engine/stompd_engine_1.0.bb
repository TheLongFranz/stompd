SUMMARY = "stompd-engine for running the audio service"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESEXTRAPATHS:prepend := "${THISDIR}/../../..:"
SRC_URI = "file://engine file://stompd_engine.service"
S = "${UNPACKDIR}/engine"

SYSTEMD_SERVICE:${PN} = "stompd_engine.service"

inherit systemd useradd

DEPENDS += ""
RDEPENDS:${PN} += ""

USERADD_PACKAGES = "${PN}"
GROUPADD_PARAM:${PN} = "--system stompd"
USERADD_PARAM:${PN} = "--system --no-create-home --shell /bin/false --gid stompd --groups audio stompd-engine"

do_install:append() {
    install -Dm 0644 ${UNPACKDIR}/stompd-engine.service ${D}${systemd_system_unitdir}/stompd_engine.service
}
