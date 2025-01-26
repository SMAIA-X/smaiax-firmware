/*
 * data_sender.h
 *
 *  Created on: 21.01.2025
 *      Author: michael
 */

#include "../dlms/dlms.h"

void init_mqtt_client();
void send_data(measurement_t* measurement);