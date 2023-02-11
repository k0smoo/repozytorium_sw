/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "stdio.h"
#include "math.h"
#include "cmsis_os.h"
#include "string.h"
#include "lcd.h"

TIM_HandleTypeDef htim6;
osThreadId Task1Handle;
osThreadId Task2Handle;
osThreadId ButtonTaskHandle;

void SystemClock_Config (void);
static void MX_GPIO_Init (void);
static void MX_TIM6_Init (void);

void Task1_init (void const * argument);
void Task2_init (void const * argument);
void ButtonTask (void* argument);

/**
 * Inicjacja kolejek
 */
xQueueHandle queue_player_cnt;
xQueueHandle queue_flags;
xQueueHandle queue_white_or_black;

/**
 * funkcja opóźniająca działanie programu
 */
void delay (uint32_t time) {
  __HAL_TIM_SET_COUNTER(&htim6, 0);
  while ((__HAL_TIM_GET_COUNTER(&htim6))<time);
}

/**
 * Funkcja ustawiąjący piny w tryb output
W momencie wywołania funkcji Set_Pin_As_Output() na początku tworzony jest obiekt
GPIO_InitStruct będący konfiguracją portów GIPO. Następnym poleceniem jest informacja, że
konfigurowana jest zmienna GPIO_Pin będąca adresem dowolnego pinu. Później pin ten jest
ustawiany jako wyjście typu Push-Pull. Instrukcja GPIO_SPEED_FREQ_LOW() informuje że pin będzie
przełączany z niską częstotliwością. Ostatnia jest wywołana funkcja z biblioteki HAL inicjalizująca
moduł/port GPIOx.
 */
void Set_Pin_As_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init (GPIOx, &GPIO_InitStruct);
}

/**
 * Funkcja ustawiąjący piny w tryb input
W momencie wywołania funkcji Set_Pin_As_Input() na początku tworzony jest obiekt
GPIO_InitStruct będący konfiguracją portów GIPO. Następnym poleceniem jest informacja, że
konfigurowana jest zmienna GPIO_Pin będąca adresem dowolnego pinu. Do zmiennej GPIO_InitStruct.Pull i wpisana zostaje wartości GPIO_NOPULL
oznaczającej, że rezystor podciągający jest wyłączony. Wtedy domyślnym stanem na pinie jest 0.
Ostatnia jest wywołana funkcja z biblioteki HAL inicjalizująca moduł/port GPIOx.
 */
void Set_Pin_As_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init (GPIOx, &GPIO_InitStruct);
}

/**
 * Funkcja int2string konwertuje zmienną typu uint16 na tablice char’ów przez algorytm przedstawiany na rysunku
 */
void int2string (uint16_t ParTemp, char* ParStringPtr) {
  int min = (ParTemp / 60) % 60;
  int s = ParTemp % 60;

  *ParStringPtr++ = (min/10) + '0'; min %= 10;
  *ParStringPtr++ = (min/1) + '0';
  *ParStringPtr++ = ':';
  *ParStringPtr++ = (s/10) + '0'; s %= 10;
  *ParStringPtr++ = (s/1) + '0';
  *ParStringPtr++ =  ' ';
}

/**
 * Główna funkcja main
 */
int main (void) {
  HAL_Init ();
  SystemClock_Config ();
  MX_GPIO_Init ();
  MX_TIM6_Init ();
  HAL_TIM_Base_Start (&htim6);
  LCD_Init ();

  queue_player_cnt = xQueueCreate(2, 4*sizeof(uint16_t));
  queue_flags = xQueueCreate(2, 4*sizeof(uint8_t));
  queue_white_or_black  = xQueueCreate(2, 4*sizeof(uint8_t));

  osThreadDef (Task1, Task1_init, osPriorityNormal, 0, 128);
  Task1Handle = osThreadCreate (osThread(Task1), NULL);

  osThreadDef (Task2, Task2_init, osPriorityNormal, 0, 128);
  Task2Handle = osThreadCreate (osThread(Task2), NULL);

  osThreadDef (Button, ButtonTask, osPriorityNormal , 0, 128);
  ButtonTaskHandle = osThreadCreate (osThread(Button), NULL);

  osKernelStart ();

  while (1) {}
}

/**
 * Ustawienia zegara systemowego
 */
void SystemClock_Config (void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL13;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * Timer
 */
static void MX_TIM6_Init (void) {
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 50-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 0xffff-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * GPIO Init
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_D0_Pin|LCD_D1_Pin|LCD_D2_Pin|LCD_D3_Pin|LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_RS_Pin|LCD_E_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = LCD_E_Pin|LCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : RS_Pin RW_Pin EN_Pin D4_Pin
                           D5_Pin D6_Pin D7_Pin */
  GPIO_InitStruct.Pin = LCD_RS_Pin|LCD_E_Pin|LCD_D0_Pin|LCD_D1_Pin|LCD_D2_Pin|LCD_D3_Pin|LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * Pierwszy task FreeRTOS
 */
void Task1_init (void const * argument) {
  char cnt_str_player_1 [5];
  char cnt_str_player_2 [5];

  uint16_t cnt [2];
  uint16_t cnt_player_1;
  uint16_t cnt_player_2;

  uint8_t button_que [2];
  uint8_t en_player_1 = 1; //queue_white_or_black

  uint8_t flags [2];
  uint8_t p1_win;      //queue_flags
  uint8_t p2_win;      //queue_flags

  for (;;) {

	xQueueReceive(queue_flags, &flags, portMAX_DELAY);
	xQueueReceive(queue_white_or_black, &button_que, portMAX_DELAY);
	xQueueReceive(queue_player_cnt, &cnt, portMAX_DELAY);

	p1_win = flags[0];
	p2_win = flags[1];
	en_player_1 = button_que[0];
	cnt_player_1 = cnt[0];
	cnt_player_2 = cnt[1];

	osDelay (100);
	if (p1_win == 1) {
	  LCD_Clear ();
	  LCD_WriteString ("Black has won!");
	}
	else if (p2_win == 1) {
	 LCD_Clear ();
	LCD_WriteString ("White has won!");
	}
	else {
      LCD_Clear ();

      if (en_player_1 == 1) {
        LCD_WriteString ("X ");
      } else {
        LCD_WriteString ("  ");
      }

	  LCD_WriteString (" Black: ");
	  int2string (cnt_player_1, cnt_str_player_1);
   	  LCD_WriteString (cnt_str_player_1);

   	  LCD_WriteString ("                        ");

      if (en_player_1 == 0) {
        LCD_WriteString ("X ");
      } else {
        LCD_WriteString ("  ");
      }

	  LCD_WriteString (" White: ");
	  int2string (cnt_player_2, cnt_str_player_2);
	  LCD_WriteString (cnt_str_player_2);
	}
  }
}

/**
 * Drugi task FreeRTOS
 */
void Task2_init (void const * argument) {
    uint16_t cnt [2];
	uint16_t cnt_player_1 = 10;
    uint16_t cnt_player_2 = 10;

    uint8_t button_que [2];
    uint8_t en_player_1;
    uint8_t timer_running;

    uint8_t flags [2];
    uint8_t p1_win = 0;
    uint8_t p2_win = 0;

  for (;;) {

	xQueueReceive(queue_white_or_black, &button_que,  portMAX_DELAY);
	en_player_1 = button_que[0];
	timer_running = button_que[1];

	if (timer_running == 1) {
      if (en_player_1) {
	    if (cnt_player_1 > 0) {
	      cnt_player_1--;
	    }
	  } else {
	    if (cnt_player_2 > 0) {
		  cnt_player_2--;
	    }
      }

      osDelay (1000);

      if (cnt_player_1 == 0) {
		timer_running = 0;
		p2_win = 1;
      } else if (cnt_player_2 == 0) {
    	timer_running = 0;
    	p1_win = 1;
      }
	}

	flags[0] = p1_win;
	flags[1] = p2_win;
	xQueueSend(queue_flags, &flags,( TickType_t ) 0 );

	cnt[0] = cnt_player_1;
	cnt[1] = cnt_player_2;
	xQueueSend(queue_player_cnt, &cnt,( TickType_t ) 0 );
  }
}

/**
 * Trzeci task obsługujący przycisk
 *
 * <pre><pre>
 *
 * void ButtonTask (void* pvParameters) {
 * uint8_t en_player_1 = 1;
 * uint8_t timer_running;
 * uint8_t button_que [2];
 * while (1) {
 *   if (HAL_GPIO_ReadPin (BUTTON_BLUE_PORT,BUTTON_BLUE_PIN) == 0) {
 *     en_player_1 = !en_player_1;
 *     timer_running = 1;
 *     osDelay (200);
 *   }
 *   button_que[0] = en_player_1;
 *   button_que[1] = timer_running;
 *   xQueueSend(queue_white_or_black, &button_que,( TickType_t ) 0 );
 * }
 *}
 *</pre></pre>
 *<pre>
 *	\brief Zadanie obsługującea przycisk i przesyłające dane za pomocą kolejki xQueueSend
 *
 *	Task zawiera 3 zmienne:
 *
 *   \param[uint8_t]   en_player_1     Wskazuje, który gracz aktualnie wykonuje ruch
 * 	 \param[uint8_t]   timer_running   Flaga informująca o trwaniu parti, gdy dojdzie do 0 partia się kończy.
 * 	 \param[uint8_t]   button_que	   Tablica przechowująca zmienne en_player_1 oraz timer_running
 *
 * 	 Tablica button_que jest przesyłana za pomocą kolejki xQueueSend
 *
 *	Funkcja ustawia wartość startową zmiennej en_player_1 na wartość 1 oznaczając tym samym, który gracz powinien teraz wykonać ruch
 *	W przypadku naciśnięcia przycisku wartość zmiennej en_player_1 jest zmieniana na przeciwną zmieniając gracza
 *	Timer running domyślnie przyjmuje wartość 0 - przed rozpoczęciem parti, po naciśnięciu przycisku jest ustawiany na wartość 1 (partia rozpoczęta, trwa)
 *	Do tablicy button_que wpisywane są obie zmienne i przesyłane są one kolejką do dwóch tasków obsługujących wyświetlanie oraz obsługe timera.
 </pre>
 */
void ButtonTask (void* pvParameters) {
  uint8_t en_player_1 = 1;
  uint8_t timer_running;
  uint8_t button_que [2];
  while (1) {
    if (HAL_GPIO_ReadPin (BUTTON_BLUE_PORT,BUTTON_BLUE_PIN) == 0) {
      en_player_1 = !en_player_1;
      timer_running = 1;
      osDelay (200);
    }
    button_que[0] = en_player_1;
    button_que[1] = timer_running;
    xQueueSend(queue_white_or_black, &button_que,( TickType_t ) 0 );
  }
}



/**
 * Error Handler
 */
void Error_Handler (void) {
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq ();
  while (1) {}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
