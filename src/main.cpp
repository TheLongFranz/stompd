// STL Stuff
#include<print>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <time.h>

// Third-Party Stuff
#include <QGuiApplication>
#include <QQmlApplicationEngine>


int set_realtime_priority(int priority) {
    struct sched_param param;

    /* Validate priority range */
    if (priority < 1 || priority > 99) {
        fprintf(stderr, "Priority must be between 1 and 99\n");
        return -1;
    }

    param.sched_priority = priority;

    /* Set the scheduling policy to SCHED_FIFO */
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        fprintf(stderr, "sched_setscheduler failed: %s\n", strerror(errno));
        return -1;
    }

    /* Lock all memory to prevent page faults during RT execution */
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        fprintf(stderr, "mlockall failed: %s\n", strerror(errno));
        /* Non-fatal, continue anyway */
    }

    printf("Set to SCHED_FIFO priority %d\n", priority);
    return 0;
}

int main(int argc, char *argv[])
{
    if (set_realtime_priority(80) != 0) {
        /* Fall back to normal operation if RT setup fails */
        fprintf(stderr, "Warning: Could not set RT priority. Running without RT.\n");
    }

    std::println("Hello, World!");
    
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);

    // Qt.quit() in QML is wired to QCoreApplication::quit() by the engine.
    engine.loadFromModule("HelloQt", "Main");

    return app.exec();
}
