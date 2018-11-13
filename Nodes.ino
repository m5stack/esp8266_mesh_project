//************************************************************
// this is prints (and sends around) information used for debugging library
//
// 1. blinks led once for every node on the mesh
// 2. blink cycle repeats every BLINK_PERIOD and is in sync between nodes
//
// this is the nodes 
//************************************************************

// Collect nodeInformation (on every change and 10 seconds)
// send it every 5 seconds
// Print on change/newConnection/receive from other node
#define UNITY
#include <painlessMesh.h>
#include "dht12.h"
#include "ir_remote.h"
#include "namedMesh.h"
#include "formctl.h"



// Task to blink the number of nodes

Task dht12Task(10000,TASK_FOREVER, &dht12_read);
Task irRemoteTask(1000,TASK_FOREVER, &ir_remote_sent);
//Task formControlTask(80000,TASK_FOREVER,&network_meddle);

Scheduler     dataScheduler; // to control your personal task


void setup() {
  Serial.begin(115200);
  dht12_init();
  ir_remote_init();
  namedMesh_init();

   dataScheduler.addTask(irRemoteTask );
  irRemoteTask.enable();
  dataScheduler.addTask( dht12Task );
  dht12Task.enable();
  
}


void loop() {

  dataScheduler.execute();
  namedMesh_run();
  
}
