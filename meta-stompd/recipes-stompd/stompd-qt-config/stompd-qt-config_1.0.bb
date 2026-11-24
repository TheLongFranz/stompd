SUMMARY = "Qt EGLFS display configuration for stompd"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://eglfs-kms.json file://qt-eglfs.sh"
S = "${UNPACKDIR}"

do_install() {
    install -Dm 0644 ${S}/eglfs-kms.json ${D}${sysconfdir}/eglfs-kms.json
    install -Dm 0644 ${S}/qt-eglfs.sh ${D}${sysconfdir}/profile.d/qt-eglfs.sh
}