/**
   @copyright (C) 2017 Melexis N.V.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#define UNITY
#include <Arduino.h>
#include "namedMesh.h"
#include <painlessMesh.h>

extern int Temprature;
extern int  Humi;

// some gpio pin that is connected to an LED...
// on my rig, this is 5, change to the right number of your LED.
#define   LED             (15)       // GPIO number of connected LED, ON ESP-12 IS GPIO2

#define   BLINK_PERIOD    3000 // milliseconds until cycle repeat
#define   BLINK_DURATION  100  // milliseconds LED is on for

#define   MESH_SSID       "whateverYouLike3"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
uint8_t   station_ip[4] =  {0,0,0,0}; // IP of the server
#define   ISROOT          false

bool onFlag = false;
String state;
String conn;
uint8_t is_sta = 0;
String conned_ap = 0;

// Prototypes
void receivedCallback(uint32_t from, String & msg);
void namedReceivedCallback(String &from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(); 
void collectData();
void nodeTimeAdjustedCallback(int32_t offset); 
void delayReceivedCallback(uint32_t from, int32_t delay);

//size_t is unsigned integer type
size_t rootId = 986195773;

// TASK

Task taskGatherState( TASK_SECOND * 30, TASK_FOREVER, &collectData); // start with a one second interval
Task blinkNoNodes;


Scheduler     meshScheduler; // to control your personal task

String nodeName = "Room5"; // Name needs to be unique

namedMesh  mesh;


Task taskPrintState(TASK_SECOND * 20, TASK_FOREVER, []() {
    Serial.println("Node state:");
    Serial.printf("%s\r\n", state.c_str());
    //stateObj.prettyPrintTo(Serial);
});


Task taskSendState(TASK_SECOND * 10, TASK_FOREVER, []() {
    String str = "Room5 : ";
    str += (String)Temprature;
    str += " C";
    str += " / ";
    str += (String) Humi;
    str += " %";
    String msg = String("This is a message from: ") + nodeName + String(" for logNode");
    String to = "Room8";
    mesh.sendSingle(to, msg);
    //Serial.println("taskSendState");
    //mesh.sendBroadcast(str);
   //Serial.println(str);
    //mesh.sendSingle(rootId, str);
    //mesh.sendBroadcast(state);
});







void namedMesh_init(void)
{
  mesh.setDebugMsgTypes(ERROR| CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &meshScheduler, MESH_PORT,WIFI_AP_STA,6);
  mesh.onReceive(&receivedCallback);
  mesh.onReceive(&namedReceivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);
  mesh.setRoot(ISROOT);
  mesh.setContainsRoot(false);
  mesh.setName(nodeName);
  
  meshScheduler.addTask( taskGatherState );
  taskGatherState.enable();
  meshScheduler.addTask( taskPrintState );
  taskPrintState.enable();
  meshScheduler.addTask( taskSendState );
  taskSendState.enable();


  blinkNoNodes.set(BLINK_PERIOD, (mesh.getNodeList().size() + 1) * 2, []() {
      // If on, switch off, else switch on
      if (onFlag)
        onFlag = false;
      else
        onFlag = true;
      blinkNoNodes.delay(BLINK_DURATION);

      if (blinkNoNodes.isLastIteration()) {
        // Finished blinking. Reset task for next run 
        // blink number of nodes (including this node) times
        blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
        // Calculate delay based on current mesh time and BLINK_PERIOD
        // This results in blinks between nodes being synced
        blinkNoNodes.enableDelayed(BLINK_PERIOD - 
            (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
      }
  });
  meshScheduler.addTask(blinkNoNodes);
  blinkNoNodes.enable();

  randomSeed(analogRead(A0));
}




void namedMesh_run(void)
{
  meshScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}


void collectData() {
    state = "";
    DynamicJsonBuffer jsonBuffer;
    JsonObject& stateObj = jsonBuffer.createObject();
    stateObj["nodeId"] = mesh.getNodeId();
#ifdef ESP32
    stateObj["hardware"] = "ESP32";
#else
    stateObj["hardware"] = "ESP8266";
#endif

    stateObj["isRoot"] = mesh.isRoot();
    stateObj["isRooted"] = mesh.isRooted();
    String subs = mesh.subConnectionJson();
    DynamicJsonBuffer subsBuffer;
    JsonArray& subsArr = jsonBuffer.parseArray(subs, 255);
    if (subsArr.success())
        stateObj["subs"] = subsArr;
    //else
    stateObj["subsOrig"] = subs;
    JsonArray& connections = stateObj.createNestedArray("connections");
    stateObj["csize"] = mesh._connections.size();
    for(auto && conn : mesh._connections) {
        JsonObject& connection = connections.createNestedObject();
        connection["nodeId"] = conn->nodeId;
        connection["connected"] = conn->connected;
        connection["station"] = conn->station;
        connection["root"] = conn->root;
        connection["rooted"] = conn->rooted;
        connection["subs"] = conn->subConnections;
        
    }
    stateObj.prettyPrintTo(connections);
   // stateObj.prettyPrintTo(state);
}

void network_forming(void)
{
  ;
}








void receivedCallback(uint32_t from, String & msg) {
  Serial.printf("startHere: Received from %u msg = %s\r\n", from, msg.c_str());
}
void namedReceivedCallback(String &from, String & msg) {
  Serial.printf("startHere: Received from %s msg = %s\r\n", from.c_str(), msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
  Serial.printf("New Connection, nodeId = %u\r\n", nodeId);
  taskGatherState.forceNextIteration();
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\r\n");
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
  taskGatherState.forceNextIteration();
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\r\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\r\n", from, delay);
}
