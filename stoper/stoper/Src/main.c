/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "display.h"
#include "state_machine.h"
#include "stdio.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t ms, s  = 0;
uint8_t start = 0;
uint8_t cnt = 0;
uint8_t cnt1 = 0;
int tab[] = {0, 0, 0, 0};
uint8_t data[100];
uint16_t size = 4; 
uint8_t bufor_cnt =0;
char bufor[5];
char Received;
enum states{IDLE, COUNT, PAUSE};
enum states state = IDLE;
uint32_t n=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void Display(int tab[4])
{
	Digit(cnt+1);
	bit_mask(tab[cnt]);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
	if(GPIO_Pin == USER_Pin)
	{
		/* Prevent unused argument(s) compilation warning */
		if (state == IDLE)
		{
			state = COUNT;
			size = sprintf(data, "Trwa odmierzanie czasu...\n\r"); 
			HAL_UART_Transmit_IT(&huart2, data, size); 
		}
			
		else if(state == COUNT)
		{
			state = IDLE;
			size = sprintf(data, "Licznik wyzerowany. Zmierzony czas: %d ms.\n\r", s*1000+ms); 
			HAL_UART_Transmit_IT(&huart2, data, size);
		}
		else if(state == PAUSE)
		{
			state = COUNT;
			size = sprintf(data, "Trwa odmierzanie czasu...\n\r");
			HAL_UART_Transmit_IT(&huart2, data, size); 
		}
	
	}
	
	else if(GPIO_Pin == TAMPER_Pin)
	{
		if(state == COUNT)
		{
			state = PAUSE;
			size = sprintf(data, "Licznik zatrzymany. Zmierzony czas: %d ms.\n\r", s*1000+ms); 
			HAL_UART_Transmit_IT(&huart2, data, size); 
		}
		
		else if(state == IDLE)
		{
			state = PAUSE;
			size = sprintf(data, "Licznik zatrzymany.\n\r"); 
			HAL_UART_Transmit_IT(&huart2, data, size); 
			
		}			
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
	{
		// mruganie wyswietlaczem podczas PAUSE
		if(state == PAUSE)
		{
			cnt1++;
			if (cnt1 < 100)
			{
				cnt++;	
				if (cnt == 4)
				{cnt = 0;}
			}
			
			if (cnt1 > 100)
			{
				cnt = 4;
			}
			
			if (cnt1 > 200)
			{
				cnt1 = 0;
				cnt = 0;
			}
		}
		
		else
		{
			cnt++;
			if (cnt >= 4)
				cnt = 0;		
		}
	}
	
	if(htim->Instance == TIM2 && start)
	{		
		s = n/1000;
		ms = n%1000;
		
		if (ms > 100)
		{
			tab[3] = (ms/10)%10;
			tab[2] = ms/100;
		}
		
		if (s >= 1)
		{
			tab[1] = s%10;
			tab[0] = s/10;
			}
		
		if (s == 99 && ms == 999)
		{
			size = sprintf(data, "Osiagnieto maksymalny zakres: %d ms. Licznik wyzerowany.\n\r", s*1000+ms); 
			HAL_UART_Transmit_IT(&huart2, data, size);
			state = IDLE;
		}
		n++;
		if (n ==99999)
		{
			n=0;
		}
	}
	
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
 {
	 if (Received == 13)
	 {
		 for(int i = 0 ; i <= bufor_cnt; i++)
		 {
			 bufor[i]= 0;
		 }
		 bufor_cnt = 0;
	 } 
	 else 
	 {
		 bufor[bufor_cnt]=Received;
		 bufor_cnt++;
	 }
	
		if((bufor[0] == 83 && bufor[1] == 84 && bufor[2] == 65 && bufor[3] == 82 && bufor[4] == 84) ||
			 (bufor[0] == 83 && bufor[1] == 84 && bufor[2] == 79 && bufor[3] == 80))// START/STOP
		{
			if(state == IDLE)
			{
				state = COUNT;	
				size = sprintf(data, "Trwa odmierzanie czasu...\n\r"); 
				HAL_UART_Transmit_IT(&huart2, data, size); 
			}
			
			else if(state == COUNT)
			{
				state = IDLE;
				size = sprintf(data, "Licznik wyzerowany. Zmierzony czas: %d ms.\n\r", s*1000+ms); 
				HAL_UART_Transmit_IT(&huart2, data, size); 
			}
			
			else if(state == PAUSE)
			{
				state = COUNT;
				size = sprintf(data, "Trwa odmierzanie czasu...\n\r");  
				HAL_UART_Transmit_IT(&huart2, data, size); 
			}
		}
		 
		 if (bufor[0] == 80 && bufor[1] == 65 && bufor[2] == 85 && bufor[3] == 83 && bufor[4] == 69) //PAUSE
			{
				if(state == COUNT)
				{
					state = PAUSE;
					size = sprintf(data, "Licznik zatrzymany. Zmierzony czas: %d ms.\n\r", s*1000+ms);  
					HAL_UART_Transmit_IT(&huart2, data, size); 
				}
				else if(state == IDLE)
				{
					state = PAUSE;
					size = sprintf(data, "Licznik zatrzymany.\n\r");  
					HAL_UART_Transmit_IT(&huart2, data, size); 
				}
			}
			
 HAL_UART_Receive_IT(&huart2, &Received, 1); // Ponowne wlaczenie nasluchiwania
		}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart2, &Received, 1);
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		state_machine(state);
		Display(tab);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the Systick interrupt time 
  */
  __HAL_RCC_PLLI2S_ENABLE();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
