/*
 * server.c -- сервер с неблокирующими(уж точно) сокетами
 * делает обмен строками между двумя клиентами
 */
#include "server.h"

/* хочу завершить выполнение программы на ^C и освободить память */
static volatile int keepRunning = 1;
void intHandler(int dummy) {
    keepRunning = 0;
}

int main(int argc, char *argv[])
{
    /* десриптор сокета, который мы слушаем */
    int listener;

    /* адрес клиента */
    char buf[BACKLOG][BUFSIZE];
    int i, j;
    int fds_size;
    char *fp[BACKLOG];

    struct pollfd *fds;
    /* чтобы я мог по ^C выйти из цикла */
    struct sigaction sa;
    sa.sa_handler = &intHandler;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    fds = NULL;
    fds_size = 0;
    /* не указан порт */
    if (argc != 2) {
        fprintf(stderr, "%s port\n", argv[0]);
        return 1;
    }


    listener = create_connection(argv[1]);

    /* добавляем его к master set */
    fds = malloc(sizeof(struct pollfd));

    if (fds == NULL) {
        fprintf(stderr, "сouldn't malloc!\n");
        exit(1);
    }

    fds_size++;
    fds[0].fd = listener;
    fds[0].events = POLLIN;

    while(keepRunning){
        poll(fds, fds_size, -1);
        for(i = 0; i < fds_size; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == listener) {
                    fds_size = newcon(fds, i, fds_size);
                    fds[fds_size - 1].events = POLLIN;
                    fp[fds_size - 1]= NULL;
                    break;
                } else {
                    fp[i] = buf[i];
                    polen(fds[i], fp[i], i);
                    printf("%s\n", buf[i]);
                    fds[i].events = POLLOUT;
                }
            } if(fds[i].revents & POLLOUT){
                fp[i] = strret(buf[i]);
                servmsg(fds[i], fp[i],i);
                close(fds[i].fd);
                fp[i] = NULL;
                memset(buf[i], 0, BUFSIZE);
                for(j = i; j < fds_size - 1; j++)
                    fds[j] = fds[j+1];
                fds_size--;
                resize(&fds, fds_size);
            }
        }
    }
    printf("\nконечная\n");
    free(fds);
    return 0;

}


int create_connection(char *port)
{
    struct addrinfo hints, *p, *ai;
    int rv;
    int yes;
    int listener;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    /* заполняем информацию */
    if ((rv = getaddrinfo(NULL, port, &hints, &ai)) != 0) {
        fprintf(stderr, "server: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        /* проверяем, получили ли мы сокет */
        if (listener < 0)
            continue;
        /* мы хотим использовать именно этот адресс и порт */
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        break;
    }

    /* если у нас p == NULL, то нифига не вышло */
    if (p == NULL) {
        fprintf(stderr, "server: can't bind\n");
        exit(2);
    }

    freeaddrinfo(ai);
    if (listen(listener, BACKLOG) == -1) {
        perror("server: listen ");
        exit(3);
    }
    return listener;
}

/* иногда дескрипторы выкидываются на мороз, и они нам больше не нужны */
int resize(struct pollfd **fds, int size)
{
    struct pollfd *tmp;
    tmp = realloc(*fds, size * sizeof(struct pollfd));
    if (tmp != NULL) {
        *fds = tmp;
    } else {
        fprintf(stderr,"couldn't realloc\n");
        exit(1);
    }
    return 0;
}


/* делаем accept соединения */
int newcon(struct pollfd *fds, int i, int fds_size)
{
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
    fds_size++;
    resize(&fds, fds_size);
    fds[fds_size - 1].fd = accept(fds[i].fd,
                                  (struct sockaddr *)&remoteaddr,
                                  &addrlen);
    if (fds[fds_size-1].fd == -1) {
        perror("server, accept");
    } else {
        if (fcntl(fds[fds_size -1].fd, F_SETFL, O_NONBLOCK) == -1) {
            perror("server, fcntl:");
        }
        printf("new connection \n");
    }
    return fds_size;

}

/* POLLIN  */
void polen(struct pollfd fds, char *fp, int i)
{
    int len;
    while(1) {
        len = recv(fds.fd, fp, BUFSIZE,0);
        if (len == -1 && errno == EAGAIN)
            break;
        if(len == 0)
            break;
        fp+=len -1;
    }

}

/* выбираем что послать */
char *strret(char *str)
{
    if(!strcmp(str, "ping")){
        return "pong";
    } else {
        return "агагагага";
    }
}

/* посылаем сообщение */
void servmsg(struct pollfd fds, char *fp, int i)
{
    int len;
    while(1) {
        len = send(fds.fd, fp, strlen(fp),0);
        if (len == -1 && errno == EAGAIN)
            break;
        if(len == 0)
            break;
        fp+=len;
    }
}
