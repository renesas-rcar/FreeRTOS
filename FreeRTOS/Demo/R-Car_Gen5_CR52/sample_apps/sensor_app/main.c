/*
 *
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "FreeRTOS.h"
#include "task.h"
#include "interrupts.h"
#include "state-manager/r_state_manager.h"
#include "pfc/r_pfc_api.h"
#include "scmi/inc/sensor.h"
#include "scmi/inc/protocol.h"

#define sensorApp_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )
#define SENSOR_LOG(format, ...) \
    {\
        printf("SENSOR [%s:%d] ", __func__, __LINE__);\
        printf(format "\r\n", ##__VA_ARGS__);\
    }

static void prvSensorAppTask(void *pvParameters);
static void sensorAppExample(void);

static void prvSetupHardware(void)
{
    portDISABLE_INTERRUPTS();
    Irq_Setup();
    (void)pfcInitModules(getModuleConfigs());
}

void main()
{

    prvSetupHardware();

    xTaskCreate(prvSensorAppTask, "SensorApp", 512U,
                NULL, sensorApp_TASK_PRIORITY, NULL);

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    for( ;; )
    {
    }
    /* Don't expect to reach here. */
    return;
}

static void prvSensorAppTask(void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    (void) pvParameters;

    SENSOR_LOG("SensorApp FreeRTOS starting...\n");
    sensorAppExample();

    for( ;; )
    {
        SENSOR_LOG("prvSensorAppTask...\n");
        vTaskDelay(10000);
    }
}

static void sensorAppExample(void)
{
    int ret;
    int domain_id;
    int tc_number = 0;
    uint32_t rates[2] = {0};

    /* Get SCMI protocols information */
    SENSOR_LOG("******* SCMI protocols information starting*******\r\n");
    ret = R_StateManager_SCMI_Info_Show();
    if (ret != 0) {
        SENSOR_LOG("Error: Failed to show SCMI information.");
        return;
    }
    SENSOR_LOG("******* SCMI protocols information  end!*******\r\n\r\n");

    /* PROTOCOL_VERSION */
    uint32_t protocol_version = 0;
    SENSOR_LOG("*******TC%d: SCMI Sensor protocol version starting*******", ++tc_number);
    ret = scmi_sensor_protocol_version_get(&protocol_version);
    if (ret != 0) {
        SENSOR_LOG("Error: Failed to get SCMI Sensor protocol version.");
    } else{
        if (protocol_version != 0x30001){
            SENSOR_LOG("Error: SCMI Sensor protocol version is incorrect: 0x%x", protocol_version);
            SENSOR_LOG("Test case is FAIL");
        } else{
            SENSOR_LOG("SCMI Sensor protocol version: 0x%x", protocol_version);
            SENSOR_LOG("Test case is PASS");
        }
    }
    SENSOR_LOG("*******TC%d: SCMI Sensor protocol version end!*******\r\n\r\n", tc_number);

    /* PROTOCOL_ATTRIBUTES */
    struct scmi_sensor_protocol_attributes protocol_attr = {0};
    SENSOR_LOG("*******TC%d: SCMI Sensor protocol attributes starting*******", ++tc_number);
    ret = scmi_sensor_protocol_attributes_get(&protocol_attr);
    if (ret != 0) {
        SENSOR_LOG("Error: Failed to get SCMI Sensor protocol attributes.");
        SENSOR_LOG("Test case is FAIL");
    } else{
        SENSOR_LOG("SCMI Sensor protocol attributes, num_sensors: 0x%x", protocol_attr.num_sensors);
        SENSOR_LOG("SCMI Sensor protocol attributes, max_async: 0x%x", protocol_attr.max_async);
        SENSOR_LOG("SCMI Sensor protocol attributes, shared_mem_addr: 0x%x", protocol_attr.shared_mem_addr);
        SENSOR_LOG("SCMI Sensor protocol attributes, shared_mem_len: 0x%x", protocol_attr.shared_mem_len);
        SENSOR_LOG("Test case is PASS");
    }
    SENSOR_LOG("*******TC%d: SCMI Sensor protocol attributes end!*******\r\n\r\n", tc_number);

    /* PROTOCOL_MESSAGE_ATTRIBUTES */
    uint32_t msg_id[] = {0x0, 0x1, 0x2, 0x3, 0x5, 0x6, 0x9, 0xA};
    uint32_t msg_attr = 0;
    uint8_t error_flag = 0;
    SENSOR_LOG("*******TC%d: SCMI Sensor message attributes starting*******", ++tc_number);
    for (uint8_t i = 0; i < (sizeof(msg_id)/4); i++){
        msg_attr = 0;
        ret = scmi_sensor_protocol_message_attributes_get(msg_id[i], &msg_attr);
        if (ret != 0) {
            SENSOR_LOG("Error: Failed to get SCMI Sensor message attributes");
            error_flag++;
        } else{
            if (msg_attr != 0){
                error_flag++;
            } else{
                SENSOR_LOG("SCMI Sensor message attributes of message id 0x%x: 0x%x", msg_id[i], msg_attr);
            }
        }
    }
    if (error_flag != 0){
        SENSOR_LOG("Test case is FAIL");
    } else{
        SENSOR_LOG("Test case is PASS");
    }
    SENSOR_LOG("*******TC%d: SCMI Sensor message attributes end!*******\r\n\r\n", tc_number);

    SENSOR_LOG("*******TC%d: SCMI Sensor message attributes not supported starting*******", ++tc_number);
    uint32_t msg_id_invalid[] = {0x4, 0x7, 0x8, 0xB, 0xC, 0xD, 0x10, 0x11, 0xFFFF};
    for (uint8_t i = 0; i < (sizeof(msg_id_invalid)/4); i++){
        msg_attr = 0;
        ret = scmi_sensor_protocol_message_attributes_get(msg_id_invalid[i], &msg_attr);
        if (ret == 0) {
            SENSOR_LOG("Error: Successfully retrieved SCMI Sensor message id 0x%x attributes despite of not being supported: 0x%x", msg_id_invalid[i], msg_attr);
            error_flag++;
        } else{
            SENSOR_LOG("Not support: SCMI Sensor message attributes of message id 0x%x: 0x%x. Return value is %d", msg_id_invalid[i], msg_attr, ret);
        }
    }
    if (error_flag != 0){
        SENSOR_LOG("Test case is FAIL");
    } else{
        SENSOR_LOG("Test case is PASS");
    }
    SENSOR_LOG("*******TC%d: SCMI Sensor message attributes not supported end!*******\r\n\r\n", tc_number);

    /* SENSOR_CONFIG_GET */
    uint32_t sensor_id = 0;
    struct scmi_sensor_config out_config = {0};
    error_flag = 0;
    SENSOR_LOG("*******TC%d: SCMI Sensor config getting starting*******", ++tc_number);
    for (uint8_t i = 0; i < protocol_attr.num_sensors; i ++){
        ret = scmi_sensor_config_get(i, &out_config);
        if (ret != SCMI_SUCCESS) {
            SENSOR_LOG("Error: Failed to get SCMI Sensor config getting.");
            error_flag++;
        } else{
            SENSOR_LOG("SCMI Sensor getting configuration of sensor %d: enabled = %d ", i, out_config.enabled);
            SENSOR_LOG("SCMI Sensor getting configuration of sensor %d: timestamped = %d ", i, out_config.timestamped);
            SENSOR_LOG("SCMI Sensor getting configuration of sensor %d: update_interval.sec = %d ", i, out_config.update_interval.sec);
            SENSOR_LOG("SCMI Sensor getting configuration of sensor %d: update_interval.exponent = %d ", i, out_config.update_interval.exponent);
        }
    }
    if (error_flag != 0){
        SENSOR_LOG("Test case is FAIL");
    } else{
        SENSOR_LOG("Test case is PASS");
    }
    SENSOR_LOG("*******TC%d: SCMI Sensor getting end!*******\r\n\r\n", tc_number);

    /* SENSOR_CONFIG_SET */
    sensor_id = 0;
    uint32_t sensor_config = 1;
    error_flag = 0;
    struct scmi_sensor_config sensor_config_get = {0};
    // sensor_config = SENSOR_CONFIG_SET_STATE_MASK | SENSOR_CONFIG_SET_TIMESTAMP_MASK | ( 0x23000000 & SENSOR_CONFIG_SET_SEC_MASK);
    SENSOR_LOG("*******TC%d: SCMI Sensor config setting starting*******", ++tc_number);
    for (uint8_t i = 0; i < protocol_attr.num_sensors; i ++){
        sensor_config_get.enabled = 0;
        sensor_config_get.timestamped = 0;
        sensor_config_get.update_interval.sec = 0;
        sensor_config_get.update_interval.exponent = 0;
        sensor_id = i;
        ret = scmi_sensor_config_set(sensor_id, sensor_config);
        if (ret != SCMI_SUCCESS) {
            SENSOR_LOG("Error: Failed to set SCMI Sensor config. return error is %d", ret);
            error_flag++;
        } else{
            SENSOR_LOG("SCMI Sensor %d setting: 0x%x ",i, ret);
            ret = scmi_sensor_config_get(sensor_id, &sensor_config_get);
            if (ret != SCMI_SUCCESS){
                SENSOR_LOG("Error: Failed to retrieve SCMI Sensor config after setting.");
                error_flag++;
            } else{
                SENSOR_LOG("SCMI Sensor %d configuration is: 0x%x ",i, sensor_config_get);
            }
        }
    }
    if (error_flag != 0){
        SENSOR_LOG("Test case is FAIL");
    } else{
        SENSOR_LOG("Test case is PASS");
    }
    SENSOR_LOG("*******TC%d: SCMI Sensor setting end!*******\r\n\r\n", tc_number);

    /* SENSOR_TRIP_POINT_CONFIG */
    sensor_id = 0;
    uint8_t trip_point_id = 0;
    enum scmi_sensor_trip_point_event_ctrl ctrl = SCMI_SENSOR_TP_EVENT_DISABLE;
    uint32_t trip_point_val_low = 0;
    uint32_t trip_point_val_high = 0;
    error_flag = 0;
    SENSOR_LOG("*******TC%d: SCMI Sensor trip point config starting*******", ++tc_number);
    for (uint8_t i = 0; i < protocol_attr.num_sensors; i ++){
        ret = scmi_sensor_trip_point_config(i, trip_point_id, ctrl, trip_point_val_low, trip_point_val_high);
        if (ret != SCMI_SUCCESS) {
            SENSOR_LOG("Error: Failed to config SCMI Sensor trip point.");
            error_flag++;
        } else{
            SENSOR_LOG("SCMI Sensor trip point config of sensor %d: 0x%x", i, ret);
        }
    }
    if (error_flag != 0){
        SENSOR_LOG("Test case is FAIL");
    } else{
        SENSOR_LOG("Test case is PASS");
    }
    SENSOR_LOG("*******TC%d: SCMI Sensor trip point config end!*******\r\n\r\n", tc_number);

    /* SENSOR_READING_GET */
    sensor_id = 0;
    uint8_t async_read = 0;
    struct scmi_sensor_reading_desc out_desc = {0};
    size_t out_cap1 = 10;
    error_flag = 0;
    int32_t temp_mc = 0;
    SENSOR_LOG("*******TC%d: SCMI Sensor reading starting*******", ++tc_number);
    for (uint8_t i = 0; i < protocol_attr.num_sensors; i ++){
        sensor_id = i;
        ret = scmi_sensor_reading_get(i, async_read, &out_desc, out_cap1); // &info);
        if (ret != SCMI_SUCCESS) {
            SENSOR_LOG("Error: Failed to read SCMI Sensor.\r\n");
            error_flag++;
        } else{
            SENSOR_LOG("SCMI Sensor %d, sensor_value_low: 0x%x (dec: %d millidegrees C )", sensor_id, out_desc.sensor_value_low, out_desc.sensor_value_low);
            SENSOR_LOG("SCMI Sensor %d, sensor_value_high: 0x%x ", sensor_id, out_desc.sensor_value_high);
            SENSOR_LOG("SCMI Sensor %d, timestamp_low: 0x%x ", sensor_id, out_desc.timestamp_low);
            SENSOR_LOG("SCMI Sensor %d, timestamp_high: 0x%x ", sensor_id, out_desc.timestamp_high);
        }
    }
    if (error_flag != 0){
        SENSOR_LOG("Test case is FAIL");
    } else{
        SENSOR_LOG("Test case is PASS");
    }
    SENSOR_LOG("*******TC%d: SCMI Sensor reading end!*******\r\n\r\n", tc_number);

    /* SENSOR_ DESCRIPTION_GET */
    uint32_t desc_index = 0;
    struct scmi_sensor_desc out[X5H_SENSOR_NUM] = {{0}};
    size_t out_cap = X5H_SENSOR_NUM;
    struct scmi_sensor_desc_page page = {0};
    SENSOR_LOG("*******TC%d: SCMI Sensor description starting*******", ++tc_number);
    ret = scmi_sensor_description_get(desc_index, &out[0], out_cap, &page);
    if (ret != SCMI_SUCCESS) {
        SENSOR_LOG("Error: Failed to get SCMI Sensor description.");
        SENSOR_LOG("Test case is FAIL");
    } else{
        SENSOR_LOG("SCMI Sensor description, num_returned: 0x%x ", page.num_returned);
        SENSOR_LOG("SCMI Sensor description, num_remaining: 0x%x ", page.num_remaining);
        if ((page.num_returned + page.num_remaining) > X5H_SENSOR_NUM) {
            SENSOR_LOG("Number of sensors is over supported sensor number in X5H");
            SENSOR_LOG("Test case is FAIL");
        } else {
            for (uint8_t i = 0; i < page.num_returned; i++){
                SENSOR_LOG("SCMI Sensor description, sensor_id: 0x%x ", out[i].sensor_id);
                SENSOR_LOG("SCMI Sensor description, sensor_attributes_low: 0x%x ", out[i].sensor_attributes_low);
                SENSOR_LOG("SCMI Sensor description, sensor_attributes_high: 0x%x ", out[i].sensor_attributes_high);
                SENSOR_LOG("SCMI Sensor description, sensor_name: %s ", out[i].sensor_name);
                SENSOR_LOG("SCMI Sensor description, ext_attrs_supported: 0x%x ", out[i].ext_attrs_supported);
                SENSOR_LOG("SCMI Sensor description, sensor_power_uW: 0x%x ", out[i].sensor_power_uW);
                SENSOR_LOG("SCMI Sensor description, resolution_valid: 0x%x ", out[i].resolution_valid);
                SENSOR_LOG("SCMI Sensor description, resolution_exponent: 0x%x ", out[i].resolution_exponent);
                SENSOR_LOG("SCMI Sensor description, resolution_res: 0x%x ", out[i].resolution_res);
                SENSOR_LOG("SCMI Sensor description, range_valid: 0x%x ", out[i].range_valid);
                SENSOR_LOG("SCMI Sensor description, min_range: 0x%x ", out[i].min_range);
                SENSOR_LOG("SCMI Sensor description, max_range: 0x%x ", out[i].max_range);
            }
            SENSOR_LOG("Test case is PASS");
        }
    }
    SENSOR_LOG("*******TC%d: SCMI Sensor description end!*******\r\n\r\n", tc_number);
}
