/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include "bt_helpers.h"


const struct bt_le_per_adv_param per_adv_params = {
	.interval_min = PER_ADV_INTERVAL,
	.interval_max = PER_ADV_INTERVAL,
	.options = 0, //(BT_LE_PER_ADV_OPT_USE_TX_POWER | BT_LE_PER_ADV_OPT_INCLUDE_ADI),
	.num_subevents = NUM_SUBEVENTS,
	.subevent_interval = SUBEVENT_INTERVAL,
	.response_slot_delay = 0x5,
	.response_slot_spacing = 0x50,
	.num_response_slots = NUM_RSP_SLOTS,
};

const struct bt_le_adv_param* adv_params = BT_LE_EXT_ADV_NCONN_NAME; //BT_LE_ADV_PARAM((BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_USE_NAME), ADVERTISING_INTERVAL, ADVERTISING_INTERVAL, NULL);

struct bt_conn *default_conn;
struct net_buf_simple bufs[NUM_SUBEVENTS];
struct bt_le_per_adv_subevent_data_params subevent_data_params[NUM_SUBEVENTS];

BUILD_ASSERT(ARRAY_SIZE(bufs) == ARRAY_SIZE(subevent_data_params));

// Function to handle the request for data from the receiver.
void request_cb(struct bt_le_ext_adv *adv, const struct bt_le_per_adv_data_request *request) {
	int err;
	uint8_t to_send;

	/* Continuously send the data in bufs and listen to all response slots */

	to_send = MIN(request->count, ARRAY_SIZE(subevent_data_params));
	for (size_t i = 0; i < to_send; i++) {
		subevent_data_params[i].subevent =
			(request->start + i) % per_adv_params.num_subevents;
		subevent_data_params[i].response_slot_start = 0;
		subevent_data_params[i].response_slot_count = NUM_RSP_SLOTS;
		subevent_data_params[i].data = &bufs[i];
	}

	err = bt_le_per_adv_set_subevent_data(adv, to_send, subevent_data_params);
	if (err) {
		printk("Failed to set subevent data (err %d)\n", err);
	}
}

bool get_address(struct bt_data *data, void *user_data) {
	bt_addr_le_t *addr = user_data;

	if (data->type == BT_DATA_LE_BT_DEVICE_ADDRESS) {
		memcpy(addr->a.val, data->data, sizeof(addr->a.val));
		addr->type = data->data[sizeof(addr->a)];

		return false;
	}

	return true;
}

void response_cb(struct bt_le_ext_adv *adv, struct bt_le_per_adv_response_info *info,
			struct net_buf_simple *buf)
{
	int err;
	bt_addr_le_t peer;
	char addr_str[BT_ADDR_LE_STR_LEN];
	struct bt_conn_le_create_synced_param synced_param;
	struct bt_le_conn_param conn_param;

	if (!buf) {
		return;
	}

	if (default_conn) {
		/* Do not initiate new connections while already connected */
		return;
	}

	bt_addr_le_copy(&peer, &bt_addr_le_none);
	bt_data_parse(buf, get_address, &peer);
	if (bt_addr_le_eq(&peer, &bt_addr_le_none)) {
		/* No address found */
		return;
	}

	bt_addr_le_to_str(&peer, addr_str, sizeof(addr_str));
	printk("Connecting to %s in subevent %d\n", addr_str, info->subevent);

	synced_param.peer = &peer;
	synced_param.subevent = info->subevent;

	/* Choose same interval as PAwR advertiser to avoid scheduling conflicts */
	conn_param.interval_min = SUBEVENT_INTERVAL;
	conn_param.interval_max = SUBEVENT_INTERVAL;

	/* Default values */
	conn_param.latency = 0;
	conn_param.timeout = 400;

	err = bt_conn_le_create_synced(adv, &synced_param, &conn_param, &default_conn);
	if (err) {
		printk("Failed to initiate connection (err %d)", err);
	}
	//bt_le_adv_stop();
}

const struct bt_le_ext_adv_cb adv_cb = {
	.pawr_data_request = request_cb,
	.pawr_response = response_cb,
};

void connected_cb(struct bt_conn *conn, uint8_t err)
{
	printk("Connected (err 0x%02X)\n", err);

	__ASSERT(conn == default_conn, "Unexpected connected callback");

	if (err) {
		bt_conn_unref(default_conn);
		default_conn = NULL;
	}
}

void disconnected_cb(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02X)\n", reason);

	__ASSERT(conn == default_conn, "Unexpected disconnected callback");

	bt_conn_unref(default_conn);
	default_conn = NULL;
}

BT_CONN_CB_DEFINE(conn_cb) = {
	.connected = connected_cb,
	.disconnected = disconnected_cb,
};

void init_bufs(uint8_t backing_store[NUM_SUBEVENTS][PACKET_SIZE])
{
	/* Set up data to send */
	for (size_t i = 0; i < NUM_SUBEVENTS; i++) {
		net_buf_simple_init_with_data(&bufs[i], &backing_store[i],
					      PACKET_SIZE);
	}
}

// resets the buffer and reinitialises it with new backing store data
void update_bufs(uint8_t backing_store[NUM_SUBEVENTS][PACKET_SIZE]) {
    for (size_t i = 0; i < NUM_SUBEVENTS; i++) {
        net_buf_simple_reset(&bufs[i]);
        init_bufs(backing_store);
    }
}

// Function to initialise the Bluetooth subsystem.
bool initialise_bt(uint8_t backing_store[NUM_SUBEVENTS][PACKET_SIZE]) {
	int err;
	struct bt_le_ext_adv *pawr_adv;

	init_bufs(backing_store);

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 1;
	}

	/* Create a non-connectable non-scannable advertising set */
	err = bt_le_ext_adv_create(adv_params, &adv_cb, &pawr_adv);
	if (err) {
		printk("Failed to create advertising set (err %d)\n", err);
		return 1;
	}

	/* Set periodic advertising parameters */
	err = bt_le_per_adv_set_param(pawr_adv, &per_adv_params);
	if (err) {
		printk("Failed to set periodic advertising parameters (err %d)\n", err);
		return 1;
	}

	/* Enable Periodic Advertising */
	err = bt_le_per_adv_start(pawr_adv);
	if (err) {
		printk("Failed to enable periodic advertising (err %d)\n", err);
		return 1;
	}

	printk("Start Periodic Advertising\n");
	err = bt_le_ext_adv_start(pawr_adv, BT_LE_EXT_ADV_START_DEFAULT);
	if (err) {
		printk("Failed to start extended advertising (err %d)\n", err);
		return 1;
	}
	return 0;
}