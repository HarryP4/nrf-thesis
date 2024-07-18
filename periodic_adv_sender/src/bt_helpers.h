#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/sys/util.h>
#include <zephyr/net/buf.h>

#define NUM_RSP_SLOTS	  4
#define NUM_SUBEVENTS	  4
#define PACKET_SIZE		  5
#define SUBEVENT_INTERVAL 0x30

struct bt_conn *default_conn;
struct net_buf_simple bufs[NUM_SUBEVENTS];


// change these to alter the advertising interval
const struct bt_le_per_adv_param per_adv_params = {
	.interval_min = 0xFF,
	.interval_max = 0xFF,
	.options = 0,
	.num_subevents = NUM_SUBEVENTS,
	.subevent_interval = SUBEVENT_INTERVAL,
	.response_slot_delay = 0x5,
	.response_slot_spacing = 0x50,
	.num_response_slots = NUM_RSP_SLOTS,
};

struct bt_le_per_adv_subevent_data_params subevent_data_params[NUM_SUBEVENTS];

// Function declarations
void request_cb(struct bt_le_ext_adv *adv, const struct bt_le_per_adv_data_request *request);
bool get_address(struct bt_data *data, void *user_data);
void response_cb(struct bt_le_ext_adv *adv, struct bt_le_per_adv_response_info *info, struct net_buf_simple *buf);
void connected_cb(struct bt_conn *conn, uint8_t err);
void disconnected_cb(struct bt_conn *conn, uint8_t reason);
void init_bufs(uint8_t backing_store[NUM_SUBEVENTS][PACKET_SIZE]);
bool initialise_bt(uint8_t backing_store[NUM_SUBEVENTS][PACKET_SIZE]);