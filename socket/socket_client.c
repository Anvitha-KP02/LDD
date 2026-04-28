#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include <linux/net.h>
#include <linux/in.h>
#include <net/sock.h>

#define SERVER_PORT 8080
#define RX_MAX      1024

static struct task_struct *client_thread;

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

static int client_fn(void *data)
{
    struct socket *sock = NULL;
    struct sockaddr_in addr;
    char rx[RX_MAX];
    int ret;

    pr_info("kclient: thread started\n");

    /* 1) Create socket */
    ret = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
    if (ret < 0) {
        pr_err("kclient: sock_create_kern failed: %d\n", ret);
        return ret;
    }

    /* 2) Connect to 127.0.0.1:8080 */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port        = htons(SERVER_PORT);

    ret = kernel_connect(sock, (struct sockaddr *)&addr, sizeof(addr), 0);
    if (ret < 0) {
        pr_err("kclient: connect failed: %d\n", ret);
        goto out;
    }

    pr_info("kclient: connected\n");

    /* 3) Send message */
    {
        const char *msg = "Hello from kernel client";
        ret = ksend(sock, msg, strlen(msg));
        if (ret < 0) {
            pr_err("kclient: send failed: %d\n", ret);
            goto out;
        }
        pr_info("kclient: sent %d bytes\n", ret);
    }

    /* 4) Receive reply */
    memset(rx, 0, sizeof(rx));
    ret = krecv(sock, rx, sizeof(rx) - 1);
    if (ret < 0) {
        pr_err("kclient: recv failed: %d\n", ret);
        goto out;
    }

    pr_info("kclient: received %d bytes: '%s'\n", ret, rx);

out:
    if (sock)
        sock_release(sock);

    pr_info("kclient: thread exiting\n");
    return 0;
}

static int __init kclient_init(void)
{
    client_thread = kthread_run(client_fn, NULL, "kclient_thread");
    if (IS_ERR(client_thread)) {
        pr_err("kclient: kthread_run failed\n");
        return PTR_ERR(client_thread);
    }
    pr_info("kclient: module loaded\n");
    return 0;
}

static void __exit kclient_exit(void)
{
    if (client_thread) {
        kthread_stop(client_thread);
        client_thread = NULL;
    }
    pr_info("kclient: module unloaded\n");
}

module_init(kclient_init);
module_exit(kclient_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("Educational kernel TCP client example");
