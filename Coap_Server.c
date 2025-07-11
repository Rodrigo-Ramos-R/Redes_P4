/*
 * Copyright 2025 NXP
 * 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <openthread/coap.h>
#include <openthread/cli.h>
#include "LED.h"
#include "Temp_sensor.h"
#include <stdio.h>
#include <stdlib.h>

#include "Coap_Server.h"
#include <ctype.h>
#include <string.h>

otInstance *instance_g;


void handle_led_request(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    char payload[10];
    int length = otMessageRead(aMessage, otMessageGetOffset(aMessage), payload, sizeof(payload) - 1);
    payload[length] = '\0';

    if (payload[0] == '1')
    {
        // Turn LED on
        otCliOutputFormat("Payload Received: %s\r\n", payload);
        otCliOutputFormat("LED On \r\n");
        LED_ON();
    }
    else if (payload[0] == '0')
    {
        // Turn LED off
        otCliOutputFormat("Payload Received: %s\r\n", payload);
        otCliOutputFormat("LED Off \r\n");
        LED_OFF();
    }

    // Send response
    otMessage *response = otCoapNewMessage(instance_g, NULL);
    if (response != NULL)
    {
        otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
        otCoapSendResponse(instance_g, response, aMessageInfo);
    }
    else
    {
        otCliOutputFormat("Error: Failed to allocate CoAP response message.\r\n");
    }
}


void handle_sensor_request(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    static double temp_value = 0;
    temp_value = Get_Temperature();

    otMessage *response;

    if (otCoapMessageGetCode(aMessage) == OT_COAP_CODE_GET)  
    {
        response = otCoapNewMessage(instance_g, NULL);
        otCliOutputFormat("GET\r\n");

        if (response != NULL)
        {
            otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CONTENT);
            
            otCoapMessageSetPayloadMarker(response);
           
            char sensorData[50] = {"0"};
            
            snprintf(sensorData, sizeof(sensorData), "%d", (int)temp_value);
            otCliOutputFormat("payload: %s\r\n", sensorData);

            otMessageAppend(response, sensorData, strlen(sensorData));

            otCoapSendResponse(instance_g, response, aMessageInfo);
        }
    }
}

#define MAX_NAME_LENGTH 32
static char current_name[MAX_NAME_LENGTH] = "rodrigo";

void handle_nombre_request(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    otMessage *response;
    otCoapCode code = otCoapMessageGetCode(aMessage);

    switch (code)
    {
        case OT_COAP_CODE_GET:
            otCliOutputFormat("GET nombre\r\n");

            response = otCoapNewMessage(instance_g, NULL);
            if (response != NULL)
            {
                otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CONTENT);
                otCoapMessageSetPayloadMarker(response);

                otCliOutputFormat("payload: %s\r\n", current_name);
                otMessageAppend(response, current_name, strlen(current_name));
                otCoapSendResponse(instance_g, response, aMessageInfo);
            }
            break;

        case OT_COAP_CODE_PUT:
        {
            otCliOutputFormat("PUT nombre\r\n");
            char new_name[MAX_NAME_LENGTH];
            int len = otMessageRead(aMessage, otMessageGetOffset(aMessage), new_name, sizeof(new_name) - 1);
            new_name[len] = '\0';

            strncpy(current_name, new_name, MAX_NAME_LENGTH);
            current_name[MAX_NAME_LENGTH - 1] = '\0'; // Ensure null-termination

            otCliOutputFormat("Updated name to: %s\r\n", current_name);

            response = otCoapNewMessage(instance_g, NULL);
            if (response != NULL)
            {
                otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
                otCoapSendResponse(instance_g, response, aMessageInfo);
            }
            break;
        }

        case OT_COAP_CODE_DELETE:
            otCliOutputFormat("DELETE nombre\r\n");

            current_name[0] = '\0'; // Clear the name

            response = otCoapNewMessage(instance_g, NULL);
            if (response != NULL)
            {
                otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_DELETED);
                otCoapSendResponse(instance_g, response, aMessageInfo);
            }
            break;

        default:
            otCliOutputFormat("Unsupported CoAP code: %d\r\n", code);
            break;
    }
}





void init_coap_server(otInstance *aInstance)
{
    I2C2_InitPins();
    LED_INIT();
    Temp_Sensor_start();

    instance_g = aInstance;
    
    static otCoapResource coapResource_led;
    static otCoapResource coapResource_sensor;
    static otCoapResource coapResource_nombre;
	
    coapResource_led.mUriPath = "led";
    coapResource_led.mHandler = handle_led_request;
    coapResource_led.mContext = NULL;
    coapResource_led.mNext = NULL;

    otCoapAddResource(aInstance, &coapResource_led);

    coapResource_sensor.mUriPath = "sensor";
    coapResource_sensor.mHandler = handle_sensor_request;
    coapResource_sensor.mContext = NULL;
    coapResource_sensor.mNext = NULL;

    otCoapAddResource(aInstance, &coapResource_sensor);
	
	coapResource_nombre.mUriPath = "nombre";
	coapResource_nombre.mHandler = handle_nombre_request;
	coapResource_nombre.mContext = NULL;
	coapResource_nombre.mNext = NULL;

	otCoapAddResource(aInstance, &coapResource_nombre);
}