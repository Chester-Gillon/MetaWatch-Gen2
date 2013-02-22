//==============================================================================
//  Copyright 2013 Meta Watch Ltd. - http://www.MetaWatch.org/
// 
//  Licensed under the Meta Watch License, Version 1.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.MetaWatch.org/licenses/license-1.0.html
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//==============================================================================

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hal_board_type.h"
#include "hal_lpm.h"
#include "Messages.h"
#include "MessageQueues.h"
#include "DebugUart.h"
#include "Utilities.h"
#include "IdleTask.h"

#if TASK_DEBUG
/*! Task Information Structure */
typedef struct
{
  void * taskHandle;
  unsigned int Depth;
  unsigned int FreeEntries;
} tTaskInfo;

static unsigned char TaskIndex = 0;
static tTaskInfo taskInfoArray[MAX_NUM_TASK_LOG_ENTRIES];

/*! Keep track of a Task.
 *
 * \return TRUE if task could be registered
 *         FALSE if the task did not fit into the list
 */
unsigned char UTL_RegisterFreeRtosTask(void * TaskHandle,
                                       unsigned int StackDepth)
{
  unsigned char retValue = 1;
  
  if(TaskIndex >= MAX_NUM_TASK_LOG_ENTRIES)
  {
    PrintString("Task Log is Full \r\n");
    retValue = 0;  
  }
  else
  {
    taskInfoArray[TaskIndex].taskHandle = TaskHandle;
    taskInfoArray[TaskIndex].Depth = StackDepth;
    taskInfoArray[TaskIndex].FreeEntries =  StackDepth;
    
	/* if this is true then there are big problems */
    if ( StackDepth > 1000 )
    {
      __no_operation();  
    }
    TaskIndex++;
  }
  return retValue;
}

/*! Check if tasks are running out of stack space.
 *
*/
static unsigned char PrintTaskIndex = 3;

void UTL_FreeRtosTaskStackCheck(void)
{
  taskInfoArray[PrintTaskIndex].FreeEntries = 
    uxTaskGetStackHighWaterMark( taskInfoArray[PrintTaskIndex].taskHandle );
  
  /* free, used, total */
  PrintStringSpaceAndThreeDecimals
    ((char*)pcTaskGetTaskName( taskInfoArray[PrintTaskIndex].taskHandle ),
     taskInfoArray[PrintTaskIndex].FreeEntries,
     taskInfoArray[PrintTaskIndex].Depth - taskInfoArray[PrintTaskIndex].FreeEntries,
                                   taskInfoArray[PrintTaskIndex].Depth);
               
  PrintTaskIndex ++;
  if ( rintTaskIndex >= TaskIndex)
  {
    PrintTaskIndex = 3;    
    CreateAndSendMessage(&Msg, QueryMemoryMsg, MSG_OPT_NONE);
  }
}

#endif /* TASK_DEBUG */

/* called in each task but currently disabled */
void CheckStackUsage(xTaskHandle TaskHandle, tString *TaskName)
{
#if CHECK_STACK_USAGE

  portBASE_TYPE HighWater = uxTaskGetStackHighWaterMark(TaskHandle);
  
  if (HighWater < 20)
  {
    PrintTimeStamp();
    PrintStringAndDecimal(TaskName, HighWater);
  }
#endif
}

void vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
  /* try to print task name */
  PrintString2("# Stack overflow:",(tString*)pcTaskName);
  SoftwareReset();
}
