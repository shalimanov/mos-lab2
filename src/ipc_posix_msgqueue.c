#include "ipc_posix_msgqueue.h"
#include "utils.h"
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static mqd_t mq;
static const char *MQ_NAME = "/ipc_test_mq";
static size_t msg_max_size = 1024;

int ipc_posix_msgqueue_init() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; // Максимальна кількість повідомлень у черзі
    attr.mq_msgsize = msg_max_size; // Максимальний розмір одного повідомлення
    attr.mq_curmsgs = 0;

    mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (mq == (mqd_t) -1) {
        log_error("Не вдалося створити POSIX чергу повідомлень: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int ipc_posix_msgqueue_send(const void *message, size_t size) {
    if (size > msg_max_size) {
        log_error("Розмір повідомлення перевищує допустимий ліміт");
        return -1;
    }

    if (mq_send(mq, (const char *)message, size, 0) == -1) {
        log_error("Не вдалося надіслати повідомлення в POSIX чергу: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int ipc_posix_msgqueue_receive(void *buffer, size_t size) {
    if (mq_receive(mq, (char *)buffer, size, NULL) == -1) {
        log_error("Не вдалося отримати повідомлення з POSIX черги: %s", strerror(errno));
        return -1;
    }
    return 0;
}

size_t ipc_posix_msgqueue_get_capacity() {
    return msg_max_size;
}

void ipc_posix_msgqueue_cleanup() {
    mq_close(mq);
    mq_unlink(MQ_NAME);
}
