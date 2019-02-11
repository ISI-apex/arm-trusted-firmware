#include <stdbool.h>
#include <platform.h>
#include <mmio.h>

#include "command.h"
#include "link.h"
#include "mailbox.h"
#include "mailbox-link.h"
#include "object.h"
#include <assert.h>
//#include "panic.h"
//#include "INFO.h"

#define MAX_LINKS 8

struct cmd_ctx {
    volatile bool tx_acked;
    uint32_t *reply;
    size_t reply_sz;
    size_t reply_sz_read;
};

struct mbox_link {
    struct object obj;
    unsigned idx_to;
    unsigned idx_from;
    struct mbox *mbox_from;
    struct mbox *mbox_to;
    struct cmd_ctx cmd_ctx;
};

static struct link links[MAX_LINKS] = {0};
static struct mbox_link mlinks[MAX_LINKS] = {0};

static void handle_ack(void *arg)
{
    struct link *link = arg;
    struct mbox_link *mlink = link->priv;
    INFO("handle_ack: %s\r\n", link->name);
    mlink->cmd_ctx.tx_acked = true;
}

static void handle_cmd(void *arg)
{
    struct link *link = arg;
    struct mbox_link *mlink = link->priv;
    struct cmd cmd; // can't use initializer, because GCC inserts a memset for initing .msg
    cmd.link = link;
    assert(sizeof(cmd.msg) == HPSC_MBOX_DATA_SIZE); // o/w zero-fill rest of msg

    INFO("handle_cmd: %s\r\n", link->name);
    // read never fails if sizeof(cmd.msg) > 0
    mbox_read(mlink->mbox_from, cmd.msg, sizeof(cmd.msg));
    if (cmd_enqueue(&cmd))
        INFO("PANIC: handle_cmd: failed to enqueue command");
}

static void handle_reply(void *arg)
{
    struct link *link = arg;
    struct mbox_link *mlink = link->priv;
    INFO("handle_reply: %s\r\n", link->name);
    mlink->cmd_ctx.reply_sz_read = mbox_read(mlink->mbox_from,
                                             mlink->cmd_ctx.reply,
                                             mlink->cmd_ctx.reply_sz);
}

static int mbox_link_disconnect(struct link *link) {
    struct mbox_link *mlink = link->priv;
    int rc;
    INFO("mbox_link_disconnect: %s\r\n", link->name);
    // in case of failure, keep going and fwd code
    rc = mbox_release(mlink->mbox_from);
    rc |= mbox_release(mlink->mbox_to);
    OBJECT_FREE(mlink);
    OBJECT_FREE(link);
    return rc;
}

static int mbox_link_send(struct link *link, int timeout_ms, void *buf,
                          size_t sz)
{
    // TODO: timeout
    struct mbox_link *mlink = link->priv;
    mlink->cmd_ctx.tx_acked = false;
    // INFO("mbox_link_send: %s\r\n", link->name);
    return mbox_send(mlink->mbox_to, buf, sz);
}

static bool mbox_link_is_send_acked(struct link *link)
{
    struct mbox_link *mlink = link->priv;
    return mlink->cmd_ctx.tx_acked;
}

static int mbox_link_poll(struct link *link, int timeout_ms)
{
    // TODO: timeout
    struct mbox_link *mlink = link->priv;
    // INFO("mbox_link_poll: %s: waiting for reply...\r\n", link->name);
    while (!mlink->cmd_ctx.reply_sz_read);
    // INFO("mbox_link_poll: %s: reply received\r\n", link->name);
    return mlink->cmd_ctx.reply_sz_read;
}

/* Todo: replace with sleep */
static int busy_wait()
{
    int i, j;
    for(i = j = 0; i < 1000; i++) {
       j += i * (i - 2);
    }
    return j;
}

static int mbox_link_request(struct link *link,
                             int wtimeout_ms, void *wbuf, size_t wsz,
                             int rtimeout_ms, void *rbuf, size_t rsz)
{
    struct mbox_link *mlink = link->priv;
    int rc;

    // INFO("mbox_link_request: %s\r\n", link->name);
    mlink->cmd_ctx.reply_sz_read = 0;
    mlink->cmd_ctx.reply = rbuf;
    mlink->cmd_ctx.reply_sz = rsz / sizeof(uint32_t);

    rc = mbox_link_send(link, wtimeout_ms, wbuf, wsz);
    if (!rc) {
        WARN("%s: mbox_link_request: send failed\r\n", __func__);
        return -1;
    }
    // TODO: timeout on ACKs as part of rtimeout_ms
    // INFO("mbox_link_request: %s: waiting for ACK...\r\n", link->name);

#if ATF_FIQ
    while (!mlink->cmd_ctx.tx_acked);
#else
    /* Until TRCH server becomes stable, we just avoid infinite wait */
    /* while (!mbox_get_ack(mlink->mbox_to)); */
    int i;
    for (i = 0; i < 10; i++) {
        busy_wait();
        if (mbox_get_ack(mlink->mbox_to))
            break;
    }
    if (!mbox_get_ack(mlink->mbox_to))
        WARN("%s: Didn't get ACK, but ignores for now\n", __func__);
    mbox_clear_ack(mlink->mbox_to);
#endif
    // INFO("mbox_link_request: %s: ACK received\r\n", link->name);

    rc = rsz;
    if (rsz) {
        rc = mbox_link_poll(link, rtimeout_ms);
        if (!rc)
            WARN("%s: mbox_link_request: recv failed\r\n", __func__);
    }
    return rc;
}

struct link *mbox_link_connect(
        const char *name,
        volatile uint32_t *base,
        unsigned idx_from, unsigned idx_to,
        struct irq *rcv_irq, unsigned rcv_int_idx, /* int index within IP block */
        struct irq *ack_irq, unsigned ack_int_idx,
        unsigned server, unsigned client)
{
    struct mbox_link *mlink;
    struct link *link;
    INFO("mbox_link_connect: %s\r\n", name);
    link = OBJECT_ALLOC(links);
    if (!link)
        return NULL;

    mlink = OBJECT_ALLOC(mlinks);
    if (!mlink) {
        INFO("ERROR: mbox_link_connect: failed to allocate mlink state\r\n");
        goto free_link;
    }

    mlink->idx_from = idx_from;
    mlink->idx_to = idx_to;

    union mbox_cb rcv_cb = { .rcv_cb = server ? handle_cmd : handle_reply };
    mlink->mbox_from = mbox_claim(base, idx_from, rcv_irq, rcv_int_idx,
                                  server, client, server, MBOX_INCOMING,
                                  rcv_cb, link);
    if (!mlink->mbox_from) {
        INFO("ERROR: mbox_link_connect: failed to claim mbox_from\r\n");
        goto free_links;
    }

    union mbox_cb ack_cb = { .ack_cb = handle_ack };
    mlink->mbox_to = mbox_claim(base, idx_to, ack_irq, ack_int_idx,
                                server, server, client, MBOX_OUTGOING,
                                ack_cb, link);
    if (!mlink->mbox_to) {
        INFO("ERROR: mbox_link_connect: failed to claim mbox_to\r\n");
        goto free_from;
    }

    mlink->cmd_ctx.tx_acked = false;
    mlink->cmd_ctx.reply = NULL;

    link->priv = mlink;
    link->name = name;
    link->disconnect = mbox_link_disconnect;
    link->send = mbox_link_send;
    link->is_send_acked = mbox_link_is_send_acked;
    link->request = mbox_link_request;
    link->recv = NULL;
    return link;

free_from:
    mbox_release(mlink->mbox_from);
free_links:
    OBJECT_FREE(mlink);
free_link:
    OBJECT_FREE(link);
    return NULL;
}
