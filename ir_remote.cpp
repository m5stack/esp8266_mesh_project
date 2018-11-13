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

#include <Wire.h>
#include <Arduino.h>
#include "ir_remote.h"
#include "ir_Midea.h"

#define IR_LED 14  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRMideaAC Midea(IR_LED);

void ir_remote_init(void)
{
  Midea.begin();
}

//Read a number of words from startAddress. Store into Data array.
//Returns 0 if successful, -1 if error
void ir_remote_sent(void)
{
  Midea.setTemp(23,1);
  Midea.send();
}

void gpio1_test(void)
{
;
}
