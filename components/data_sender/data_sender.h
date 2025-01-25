/*
 * data_sender.h
 *
 *  Created on: 21.01.2025
 *      Author: michael
 */

#include "../dlms/dlms.h"

void initMqttClient();
void sendData(measurement_t* measurement);