#ifndef _FORMCTL_H_
#define _FORMCTL_H_

#include <stdint.h>
#include<map>

#include "painlessMesh.h"

typedef std::function<void(String &from, String &msg)> namedReceivedCallback_t;

class namedMesh : public painlessMesh {
    public:
        namedMesh() {
            receivedCallback = [this](uint32_t from, String &msg) {
                // Try to parse it.. Need to test it with non json function
                Serial.printf("receivedCallback:%s\r\n",msg.c_str());
                DynamicJsonBuffer jsonBuffer;
                JsonObject& root = jsonBuffer.parseObject(msg);
                if (root.success() && root.containsKey("topic") &&
                        String("nameBroadCast").equals(root["topic"].as<String>())) {
                    nameMap[from] = root["name"].as<String>();
                    Serial.println("nameMap");
                } else {
                    if (userReceivedCallback)
                        // If needed send it on to userReceivedCallback
                        userReceivedCallback(from, msg);
                    if (userNamedReceivedCallback) {
                        String name;
                        // If needed look up name and send it on to userNamedReceivedCallback
                        if (nameMap.count(from) > 0) {
                            name = nameMap[from];
                        } else {
                            name = String(from);
                        }
                        userNamedReceivedCallback(name, msg);
                    }
                }
            };
            changedConnectionsCallback = [this]() {
                if (nameBroadCastTask.isEnabled()) {
                    nameBroadCastTask.forceNextIteration();
                }
                if (userChangedConnectionsCallback)
                    userChangedConnectionsCallback();
            };
        }

        String getnameMap(uint32_t nodeid) {
            return nameMap[nodeid];
        }
        String getName() {
            return nodeName;
        }

        void setName(String &name) {
            nodeName = name;
            // Start broadcast task if not done yet
            if (!nameBroadCastInit) {
                // Initialize
                nameBroadCastTask.set(5*TASK_MINUTE, TASK_FOREVER,
                        [this]() {
                            String msg;
                            // Create arduinoJson msg
                            DynamicJsonBuffer jsonBuffer;
                            JsonObject& root = jsonBuffer.createObject();
                            root["topic"] = "nameBroadCast";
                            root["name"] = this->getName();
                            root.printTo(msg);
                            this->sendBroadcast(msg);
                        }
                );
                // Add it
                _scheduler.addTask(nameBroadCastTask);
                nameBroadCastTask.enableDelayed();

                nameBroadCastInit = true;
            }
            nameBroadCastTask.forceNextIteration();
        }

        using painlessMesh::sendSingle;
        bool sendSingle(String &name, String &msg) {
            // Look up name
            for (auto && pr : nameMap) {
                if (name.equals(pr.second)) {
                    uint32_t to = pr.first;
                    Serial.printf("to:%d\r\n",to);
                    return painlessMesh::sendSingle(to, msg);
                }
            }
             Serial.println("send single failed");
            return false;
        }

        virtual void stop() {
            nameBroadCastTask.disable();
            _scheduler.deleteTask(nameBroadCastTask);
            painlessMesh::stop();
        }

        virtual void onReceive(receivedCallback_t onReceive) {
            userReceivedCallback = onReceive;
        }
        void onReceive(namedReceivedCallback_t  onReceive) {
            userNamedReceivedCallback = onReceive;
        }
        virtual void onChangedConnections(changedConnectionsCallback_t onChangedConnections) {
            userChangedConnectionsCallback = onChangedConnections;
        }

    protected:
        String nodeName;
        std::map<uint32_t, String> nameMap;

        receivedCallback_t              userReceivedCallback;
        namedReceivedCallback_t         userNamedReceivedCallback;

        changedConnectionsCallback_t    userChangedConnectionsCallback;

        bool nameBroadCastInit = false;
        Task nameBroadCastTask;
};


void namedMesh_init(void);
void namedMesh_run(void);



#endif
