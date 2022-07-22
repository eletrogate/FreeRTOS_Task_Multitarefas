/**
 * @file main.cpp
 * @author Evandro Teixeira
 * @brief
 * @version 0.1
 * @date 06-01-2022
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <Arduino.h>
#include <freertos/queue.h>
#include <freertos/task.h>

// Macro com as cores 
#define COLOR_BLACK     "\e[0;30m"
#define COLOR_RED       "\e[0;31m"
#define COLOR_GREEN     "\e[0;32m"
#define COLOR_YELLOW    "\e[0;33m"
#define COLOR_BLUE      "\e[0;34m"
#define COLOR_PURPLE    "\e[0;35m"
#define COLOR_CYAN      "\e[0;36m"
#define COLOR_WRITE     "\e[0;37m"
#define COLOR_RESET     "\e[0m"
#define PRIMARY_CORE    PRO_CPU_NUM
#define SECONDARY_CORE  APP_CPU_NUM
#define TASK_DELAY      1000

enum                          // Estado da tarefa A
{
  task_unsuspended = false,   //tarefa não suspensa 
  task_suspended = true       //tarefa suspensa
};

// Prototipo das funções
void Tarefa_A(void *parameters);
void Tarefa_B(void *parameters);
void Tarefa_C(void *parameters);
void status_tarefa_a_set(bool sts);
bool status_tarefa_a_get(void);

// Variaveis globais
TaskHandle_t Handle_Tarefa_A = NULL;
TaskHandle_t Handle_Tarefa_B = NULL;
const uint32_t TaskDelay = TASK_DELAY;
static bool status_tarefa_a;

void setup()
{
  // Inicializa a Serial 
  Serial.begin(115200);
  Serial.printf("\n\rFreeRTOS - Tarefas\n\r");

  // Set estado inicial da variavel status_tarefa_a como tarefa não suspensa 
  status_tarefa_a_set(task_unsuspended);

  // Cria as tarefa do projeto
  xTaskCreatePinnedToCore(Tarefa_A, "Tarefa_A", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &Handle_Tarefa_A, PRIMARY_CORE);
  //xTaskCreatePinnedToCore(Tarefa_B, "Tarefa_B", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &Handle_Tarefa_B, tskNO_AFFINITY);
  xTaskCreate(Tarefa_B, "Tarefa_B", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &Handle_Tarefa_B);
  xTaskCreatePinnedToCore(Tarefa_C, "Tarefa_C", configMINIMAL_STACK_SIZE, (void*)TaskDelay, tskIDLE_PRIORITY + 3, NULL, SECONDARY_CORE);
}

void loop()
{
  Serial.printf("LOOP\n\r");
  vTaskSuspend(NULL);
}

/**
 * @brief 
 * 
 * @param sts 
 */
void status_tarefa_a_set(bool sts)
{
  status_tarefa_a = sts;
}

/**
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool status_tarefa_a_get(void)
{
  return status_tarefa_a; 
}

/**
 * @brief 
 * 
 * @param parameters 
 */
void Tarefa_A(void *parameters)
{
  static uint8_t counter_to_suspend = 0;          // Contador de suspender a tarefa A

  while (1)
  {
    Serial.print(COLOR_RED);                      // altera para vermelho impressão da mensagem na serial
    Serial.print( pcTaskGetTaskName(NULL) );
    Serial.print(" Core: ");
    Serial.println( xPortGetCoreID() );           // busca o identificador do CPU que esta executando a tarefa 
    Serial.print(COLOR_RESET);                    // reset a cor da impressão da mensagem na serial 

    if(counter_to_suspend < 4)
    {
      counter_to_suspend++;                       // incrementa contador 
      vTaskDelay(TASK_DELAY/portTICK_PERIOD_MS);  // Pausa a execução da tarefa por 1000 milessegundos 
    }
    else 
    {
      counter_to_suspend = 0;                     // reset contador 
      status_tarefa_a_set(task_suspended);        // Set estado inicial da variavel status_tarefa_a como tarefa suspensa 
      vTaskSuspend(Handle_Tarefa_A);              // Suspende a execução da tarefa 
    }
  }
}

/**
 * @brief 
 * 
 * @param parameters 
 */
void Tarefa_B(void *parameters)
{
  static uint8_t counter_to_activate_task = 0;    // contador para tirar da suspensão a tarefa A

  while (1)
  {
    Serial.print(COLOR_GREEN);                    // altera para verde impressão da mensagem na serial
    Serial.print( pcTaskGetTaskName(NULL) );
    Serial.print(" Core: ");
    Serial.println(xPortGetCoreID());             // busca o identificador do CPU que esta executando a tarefa 
    Serial.print(COLOR_RESET);                    // reset a cor da impressão da mensagem na serial 
    
    if(status_tarefa_a_get() == task_suspended)   // Checa se a tarefa A esta suspensa
    {
      if(counter_to_activate_task < 2)
      {
        counter_to_activate_task++;               // Incrementa contador
      }
      else 
      {
        counter_to_activate_task = 0;             // Reset contador
        status_tarefa_a_set(task_unsuspended);    // Set estado inicial da variavel status_tarefa_a como tarefa não suspensa 
        vTaskResume(Handle_Tarefa_A);             // Reativa a tarefa A
      }
    }
    vTaskDelay(TASK_DELAY/portTICK_PERIOD_MS);    // Pausa a execução da tarefa por 1000 milessegundos 
  }
}

/**
 * @brief 
 * 
 * @param parameters 
 */
void Tarefa_C(void *parameters)
{
  const TickType_t Delay = (TickType_t)parameters;  // recebe o parametro adicionado na criação da tarefa
  static uint8_t counter_to_delete_task = 0;        // contador para deletar a tarefa 

  while (1)
  {
    Serial.print(COLOR_YELLOW);                     // altera para verde impressão da mensagem na serial
    Serial.print( pcTaskGetTaskName(NULL) );
    Serial.print(" Core: ");
    Serial.println(xPortGetCoreID());               // busca o identificador do CPU que esta executando a tarefa 
    Serial.print(COLOR_RESET);                      // reset a cor da impressão da mensagem na serial 

    if(counter_to_delete_task < 4)
    {
      counter_to_delete_task++;                     // Incrementa contador
      vTaskDelay(Delay/portTICK_PERIOD_MS);         // Pausa a execução da tarefa por 1000 milessegundos 
    }
    else 
    {
      vTaskDelete(NULL);                            // Deleta tarefa da execução 
    }
  }
}