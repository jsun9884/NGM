/*
 * MosquittoClient.h
 *
 *  Created on: Aug 9, 2016
 *      Author: sergei
 */

#ifndef MOSQUITTOCLIENT_H_
#define MOSQUITTOCLIENT_H_

#include <mosquittopp.h>
#include "Log.h"

class CloudManager;
class MosquittoClient: public mosqpp::mosquittopp
{
    LOG_MODULE(Mosquitto, g_main_log);

public:
    MosquittoClient(CloudManager *mgr);
	virtual ~MosquittoClient();

    const char *configUpdateTopic() const;
    const char *configGetTopic() const;
    const char *commandTopic() const;
    const char *bluetoothPresenceTopic() const;
    const char *networksGetTopic() const;
    const char *networksAddTopic() const;
    const char *networksDeleteTopic() const;

protected:
	virtual void on_connect(int rc) override;
	virtual void on_message(const struct mosquitto_message *message) override;

private:
    CloudManager *m_mgr;
};

#endif /* MOSQUITTOCLIENT_H_ */
