// kserver.c - educational kernel TCP server example
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include <linux/net.h>
#include <linux/in.h>
#include <net/sock.h>

#define SERVER_PORT 8080
#define RX_MAX      1024

static struct task_struct *server_thread;

static int krecv(struct socket *sock, char *buf, size_t len)
{
    struct msghdr msg = {0};
    struct kvec iov;
    int ret;

    iov.iov_base = buf;
    iov.iov_len  = len;

    ret = kernel_recvmsg(sock, &msg, &iov, 1, len, 0);
    return ret;
}

static int ksend(struct socket *sock, const char *buf, size_t len)
{
    struct msghdr msg = {0};
    struct kvec iov;
    int ret;

    iov.iov_base = (char *)buf;
    iov.iov_len  = len;

    ret = kernel_sendmsg(sock, &msg, &iov, 1, len);
    return ret;
}

static int server_fn(void *data)
{
    struct socket *listen_sock = NULL;
    struct socket *client_sock = NULL;
    struct sockaddr_in addr;
    char rx[RX_MAX];
    int ret;

    pr_info("kserver: thread started\n");

    /* 1) Create kernel socket */
    ret = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &listen_sock);
    if (ret < 0) {
        pr_err("kserver: sock_create_kern failed: %d\n", ret);
        return ret;
    }

    /* 2) Bind to 127.0.0.1:8080 */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);   // 127.0.0.1
    addr.sin_port        = htons(SERVER_PORT);

    ret = kernel_bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        pr_err("kserver: bind failed: %d\n", ret);
        goto out;
    }

    /* 3) Listen */
    ret = kernel_listen(listen_sock, 1);
    if (ret < 0) {
        pr_err("kserver: listen failed: %d\n", ret);
        goto out;
    }

    pr_info("kserver: listening on 127.0.0.1:%d\n", SERVER_PORT);

    /* 4) Accept one client (blocking) */
    ret = kernel_accept(listen_sock, &client_sock, 0);
    if (ret < 0) {
        pr_err("kserver: accept failed: %d\n", ret);
        goto out;
    }

    pr_info("kserver: client accepted\n");

    /* 5) Receive */
    memset(rx, 0, sizeof(rx));
    ret = krecv(client_sock, rx, sizeof(rx) - 1);
    if (ret < 0) {
        pr_err("kserver: recv failed: %d\n", ret);
        goto out_client;
    }

    pr_info("kserver: received %d bytes: '%s'\n", ret, rx);

    /* 6) Send reply */
    {
        const char *reply = "Hello from kernel server";
        ret = ksend(client_sock, reply, strlen(reply));
        if (ret < 0) {
            pr_err("kserver: send failed: %d\n", ret);
            goto out_client;
        }
        pr_info("kserver: sent %d bytes reply\n", ret);
    }

out_client:
    if (client_sock)
        sock_release(client_sock);

out:
    if (listen_sock)
        sock_release(listen_sock);

    pr_info("kserver: thread exiting\n");
    return 0;
}

static int __init kserver_init(void)
{
    server_thread = kthread_run(server_fn, NULL, "kserver_thread");
    if (IS_ERR(server_thread)) {
        pr_err("kserver: kthread_run failed\n");
        return PTR_ERR(server_thread);
    }
    pr_info("kserver: module loaded\n");
    return 0;
}

static void __exit kserver_exit(void)
{
    if (server_thread) {
        /* best-effort stop; accept() may be blocking */
        kthread_stop(server_thread);
        server_thread = NULL;
    }
    pr_info("kserver: module unloaded\n");
}

module_init(kserver_init);
module_exit(kserver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba-");
MODULE_DESCRIPTION("Educational kernel TCP server example");
