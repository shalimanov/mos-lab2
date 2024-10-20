#include "ipc_msgqueue.h"
#include "utils.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MSG_KEY 1234

static int msg_id = -1;

struct message_buffer {
    long msg_type;
    char msg_text[1024];
};

int ipc_msgqueue_init() {
    msg_id = msgget(MSG_KEY, 0666 | IPC_CREAT);
    if (msg_id == -1) {
        log_error("Не вдалося створити чергу повідомлень");
        return -1;
    }
    return 0;
}

int ipc_msgqueue_send(const void *message, size_t size) {
    struct message_buffer msg;
    msg.msg_type = 1; // Встановлюємо тип повідомлення
    if (size > sizeof(msg.msg_text)) {
        log_error("Розмір повідомлення перевищує допустимий ліміт");
        return -1;
    }
    memcpy(msg.msg_text, message, size);

    // Неблокуюче надсилання
    if (msgsnd(msg_id, &msg, size, IPC_NOWAIT) == -1) {
        if (errno == EAGAIN) {
            log_error("Черга переповнена, спроба звільнити місце...");
            // Зчитуємо і видаляємо повідомлення, щоб звільнити місце
            struct message_buffer temp_msg;
            while (msgrcv(msg_id, &temp_msg, sizeof(temp_msg.msg_text), 1, IPC_NOWAIT) != -1);
            // Спробуємо знову надіслати повідомлення
            if (msgsnd(msg_id, &msg, size, IPC_NOWAIT) == -1) {
                log_error("Не вдалося надіслати повідомлення навіть після очищення");
                return -1;
            }
        } else {
            log_error("Не вдалося надіслати повідомлення в чергу");
            return -1;
        }
    }
    return 0;
}

int ipc_msgqueue_receive(void *buffer, size_t size) {
    struct message_buffer msg;
    if (msgrcv(msg_id, &msg, sizeof(msg.msg_text), 1, IPC_NOWAIT) == -1) {
        if (errno != ENOMSG) {
            log_error("Не вдалося отримати повідомлення з черги");
        }
        return -1;
    }
    if (size > sizeof(msg.msg_text)) {
        size = sizeof(msg.msg_text);
    }
    memcpy(buffer, msg.msg_text, size);
    return 0;
}

size_t ipc_msgqueue_get_capacity() {
    return sizeof(((struct message_buffer *)0)->msg_text);
}

void ipc_msgqueue_cleanup() {
    if (msg_id != -1) {
        // Очищуємо чергу від повідомлень перед видаленням
        struct message_buffer msg;
        while (msgrcv(msg_id, &msg, sizeof(msg.msg_text), 1, IPC_NOWAIT) != -1);
        msgctl(msg_id, IPC_RMID, NULL);
        msg_id = -1;
    }
}
