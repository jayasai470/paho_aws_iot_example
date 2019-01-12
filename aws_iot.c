#include <stdio.h>
#include <string.h>
#include "MQTTClient.h"
#include <stdlib.h>
#include <unistd.h>

#define ADDRESS "ssl://*******.iot.us-east-1.amazonaws.com:8883"
#define CLIENTID "somerandomclient"
#define QOS 0
#define TIMEOUT 10000L
MQTTClient_deliveryToken deliveredtoken;
MQTTClient client;

const char *ping_topic = "ping";
const char *pong_topic = "pong";

void connect_MQTT_client();

static void __connlost(void *context, char *cause)
{
    printf("Connection lost\n");
    printf("cause: %s\n", cause);
    connect_MQTT_client();
}

static void _delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

static int __msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    printf("Message arrived for topic: \"%s\"", topicName);
    char *msgRX = "hello received";
    int ret = MQTTClient_publish(client, pong_topic, strlen(msgRX), msgRX, 1, 1, &deliveredtoken);
    printf("publishing topic to %s, ret %d", pong_topic, ret);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

int sslErrorCallback(const char *str, size_t len, void *u)
{
    printf("ssl error call back triggered ....%s", str);
    return 0;
}

void connect_MQTT_client()
{
    printf("connect_MQTT_client\n");

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;

    static const char *ca_path = "certs/ca";
    static const char *trust_path = "certs/certificate.pem.crt";
    static const char *private_path = "certs/private.pem.key";
    static const char *public_path = "certs/public.pem.key";
    /static const char *rootCA = "certs/ca/rootCA.pem";

    ssl_opts.CApath = ca_path;
    ssl_opts.keyStore = trust_path;
    ssl_opts.trustStore = rootCA;
    ssl_opts.privateKey = private_path;

    ssl_opts.enableServerCertAuth = 1;
    ssl_opts.sslVersion = 3;
    ssl_opts.ssl_error_cb = sslErrorCallback;
    ssl_opts.verify = 1;
    conn_opts.ssl = &ssl_opts;

    int rc = MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc == MQTTCLIENT_SUCCESS)
    {
        printf("MQTT client has been created");
    }
    else
    {
        printf("Failed to create, return code %d", rc);
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, __connlost, __msgarrvd, _delivered);

    printf("trying to connect to server");
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n reason is %s", rc, MQTTClient_strerror(rc));
        exit(EXIT_FAILURE);
    } else {
        printf("Succesfully connected %d",rc);
    }

    printf("Subscribing to topic \"%s\"", ping_topic);
    MQTTClient_subscribe(client, ping_topic, QOS);
}

static void service_app_create()
{
    printf("service_app_create");

    connect_MQTT_client();

    return;
}

int main(int argc, char *argv[])
{
    service_app_create();
}
