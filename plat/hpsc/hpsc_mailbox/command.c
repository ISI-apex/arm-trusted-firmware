#include <stdint.h>
#include <delay_timer.h>
#include <debug.h>
#include "mailbox.h"
#include "server.h"

#include "command.h"

#define CMD_QUEUE_LEN 4
#define REPLY_SIZE CMD_MSG_LEN

static size_t cmdq_head = 0;
static size_t cmdq_tail = 0;
static struct cmd cmdq[CMD_QUEUE_LEN];

static cmd_handler_t *cmd_handler = NULL;

void cmd_handler_register(cmd_handler_t cb)
{
    cmd_handler = cb;
}

void cmd_handler_unregister()
{
    cmd_handler = NULL;
}

int cmd_enqueue(struct cmd *cmd)
{
    size_t i;

    if (cmdq_head + 1 % CMD_QUEUE_LEN == cmdq_tail) {
        VERBOSE("cannot enqueue command: queue full\r\n");
        return 1;
    }
    cmdq_head = (cmdq_head + 1) % CMD_QUEUE_LEN;

    // cmdq[cmdq_head] = *cmd; // can't because GCC inserts a memcpy
    cmdq[cmdq_head].link = cmd->link;
    for (i = 0; i < CMD_MSG_LEN; ++i)
        cmdq[cmdq_head].msg[i] = cmd->msg[i];

    VERBOSE("enqueue command (tail %lu head %lu): cmd %u arg %u...\r\n",
           cmdq_tail, cmdq_head, cmdq[cmdq_head].msg[0], cmdq[cmdq_head].msg[1]);

    // TODO: SEV (to prevent race between queue check and WFE in main loop)

    return 0;
}

int cmd_dequeue(struct cmd *cmd)
{
    size_t i;

    if (cmdq_head == cmdq_tail)
        return 1;

    cmdq_tail = (cmdq_tail + 1) % CMD_QUEUE_LEN;

    // *cmd = cmdq[cmdq_tail].cmd; // can't because GCC inserts a memcpy
    cmd->link = cmdq[cmdq_tail].link;
    for (i = 0; i < CMD_MSG_LEN; ++i)
        cmd->msg[i] = cmdq[cmdq_tail].msg[i];
    VERBOSE("dequeue command (tail %lu head %lu): cmd %u arg %u...\r\n",
           cmdq_tail, cmdq_head, cmdq[cmdq_tail].msg[0], cmdq[cmdq_tail].msg[1]);
    return 0;
}

bool cmd_pending()
{
    return !(cmdq_head == cmdq_tail);
}

void cmd_handle(struct cmd *cmd)
{
    uint32_t reply[REPLY_SIZE];
    int reply_len;
    size_t rc;
    unsigned sleep_ms_rem = CMD_TIMEOUT_MS_REPLY;

    VERBOSE("CMD handle cmd %x arg %x...\r\n", cmd->msg[0], cmd->msg[1]);

    if (!cmd_handler) {
        WARN("CMD: no handler registered\r\n");
        return;
    }

    reply_len = cmd_handler(cmd, &reply[0], REPLY_SIZE - 1); // 1 word for header

    if (reply_len < 0) {
        WARN("ERROR: failed to process request: server error\r\n");
        return;
    }

    if (!reply_len) {
        WARN("server did not produce a reply for the request\r\n");
        return;
    }

    rc = cmd->link->send(cmd->link, CMD_TIMEOUT_MS_SEND, reply,
                         reply_len * sizeof(uint32_t));
    if (!rc) {
        WARN("%s: failed to send reply\r\n", cmd->link->name);
    } else {
        INFO("%s: waiting for ACK for our reply\r\n", cmd->link->name);
        do {
            if (cmd->link->is_send_acked(cmd->link)) {
                INFO("%s: ACK for our reply received\r\n", cmd->link->name);
                break;
            }
            if (sleep_ms_rem) {
                mdelay(CMD_TIMEOUT_MS_RECV);
                sleep_ms_rem -= sleep_ms_rem < CMD_TIMEOUT_MS_RECV ?
                    sleep_ms_rem : CMD_TIMEOUT_MS_RECV;
            } else {
                WARN("%s: timed out waiting for ACK\r\n", cmd->link->name);
                break;
            }
        } while (1);
    }
}
