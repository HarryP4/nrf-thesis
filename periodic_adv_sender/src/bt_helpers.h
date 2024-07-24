#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/sys/util.h>
#include <zephyr/net/buf.h>


#define NUM_RSP_SLOTS	  4
#define NUM_SUBEVENTS	  4
#define PACKET_SIZE		  8
#define SUBEVENT_INTERVAL 0x30
#define ADVERTISING_INTERVAL BT_GAP_ADV_FAST_INT_MIN_2
#define PER_ADV_INTERVAL 0x100

extern struct bt_conn *default_conn;
extern struct net_buf_simple bufs[NUM_SUBEVENTS];

// change these to alter the advertising interval
extern const struct bt_le_per_adv_param per_adv_params;
extern const struct bt_le_adv_param* adv_params;

extern struct bt_le_per_adv_subevent_data_params subevent_data_params[NUM_SUBEVENTS];


// Function declarations
void request_cb(struct bt_le_ext_adv *adv, const struct bt_le_per_adv_data_request *request);
bool get_address(struct bt_data *data, void *user_data);
void response_cb(struct bt_le_ext_adv *adv, struct bt_le_per_adv_response_info *info, struct net_buf_simple *buf);
void connected_cb(struct bt_conn *conn, uint8_t err);
void disconnected_cb(struct bt_conn *conn, uint8_t reason);
void init_bufs(uint8_t backing_store[NUM_SUBEVENTS][PACKET_SIZE]);
void update_bufs(uint8_t backing_store[NUM_SUBEVENTS][PACKET_SIZE]);
bool initialise_bt(uint8_t backing_store[NUM_SUBEVENTS][PACKET_SIZE]);