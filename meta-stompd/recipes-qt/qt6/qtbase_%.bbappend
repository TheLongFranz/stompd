# These have to be added to a class-target otherwise they will apply to the entire qt build, not just this app.
PACKAGECONFIG_GL:class-target = "gles2"
PACKAGECONFIG:append:class-target = " eglfs kms gbm"
QT_QPA_DEFAULT_PLATFORM:class-target = "eglfs"