#include "main.h"
#include "stm32f4xx.h"


#define GPIOD_CLOCK (1u<<3)
#define GPIOC_CLOCK (1<<2)

// Definicion de variables
int sec, dsec, min, dmin, hou, dhou;
int S = 0, M = 14, H = 18;						// Hora inicial
uint32_t count=0;


// Definicion de funciones

RTC_HandleTypeDef hrtc;  
void SystemClock_Config(void);  				// Definicion del reloj
static void MX_GPIO_Init(void); 				// Inicializacion de nuestra interrupcion
static void MX_RTC_Init(void);  				// Inicializacion de RTC
void msDelay	(uint32_t mSTime);				// Funcion para retardos
void CambiarSeg(void);									// Funcion para cambiar los segundos
void CambiarMin(void);									// Funcion para cambiar los minutos
void CambiarHr(void);										// Funcion para cambiar las horas



// Inicio funcion principal main
int main(void)
{
  /* USER CODE BEGIN 1 */

	// Nombramos las variables donde pondremos el tiempo
	RTC_TimeTypeDef sTime1;		// Tiempo
	RTC_DateTypeDef sDate1;		// Fecha
	
	//Inicializa los reloj del puerto D y C
	RCC->AHB1ENR = GPIOD_CLOCK|GPIOC_CLOCK;
	RCC->APB2ENR = 0x0100;
	GPIOD->MODER = 0x00001555; 		// Inicializa lo primeros 7 bits del puerto D como salida
	GPIOC->MODER = 0x0003;				// Inicializa la entrada analoga
	ADC1->CR2 = 0x003;
	ADC1->SMPR1 = 0x0006;
	ADC1->SQR3 = 0x000A;	

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  SystemClock_Config();

 
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();

  /* Infinite loop */
  while (1)
  {
    /* USER CODE END WHILE */
		// Obtenemos el tiempo y lo configuramos para obtenerlo en binario
		HAL_RTC_GetTime(&hrtc, &sTime1, RTC_FORMAT_BIN);		// Tiempo
		HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BIN);		// Fecha


		// Procesamos lo obtenido para mostrarlo en un barrido
    dhou	=	(sTime1.Hours / 10);			// Decimal de horas
		hou		=	(sTime1.Hours % 10);			// Unidad horas
		dmin	=	(sTime1.Minutes / 10);		// Decimal minutos
		min		=	(sTime1.Minutes % 10);		// Unidad minutos
		dsec	=	(sTime1.Seconds / 10);    // Decimal segundos
		sec		=	(sTime1.Seconds % 10);		// Unidad de segundos

		
		// Muestra el barrido 
		// Para mostrar el barrido tenemos que
		// agregarle un valor para que se muestre en los displays
		// cada numero es equivalente al display al que va conectado
		GPIOD->ODR = dhou + 0x70;				//Y7
		msDelay(3);
		GPIOD->ODR = hou + 0x60;				//Y6
		msDelay(3);
		GPIOD->ODR = dmin + 0x40;				//Y4
		msDelay(3);
		GPIOD->ODR = min + 0x30;				//Y3
		msDelay(3);
		GPIOD->ODR = dsec + 0x10;				//Y1
		msDelay(3);
		GPIOD->ODR = sec;								//Y0
		msDelay(3);
		/* USER CODE BEGIN 3 */
		
		// Inicio if 
		// En caso de contar una interrupcion
		// va a la funcion para cambiar segundos
		if (count == 1)
		{
			CambiarSeg();
		}
		// En caso de contar dos interrupciones
		// va a la funcion de cambiar minutos
		else if (count == 2)
		{
			CambiarMin();
		}
		// En caso de contar 3 interrupciones
		// va a la funcion de cambiar horas
		else if(count == 3)
		{
			CambiarHr();
		}
  }
  /* USER CODE END 3 */
}

// Funcion para retardos
void msDelay	(uint32_t msTime)
{
	for(uint32_t i=0; i<msTime*4000; i++);
}

// Funcion para configurar l reloj
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

// Funcion para inicializar el rtc
static void MX_RTC_Init(void)
{

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 226;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
	
	
	//AJUSTAR HORA INICIAL
	sTime.Hours = H;
  sTime.Minutes = M;
  sTime.Seconds = S;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
	
	//AJUSTAR FECHA INICIAL
  sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
  sDate.Month = RTC_MONTH_NOVEMBER;
  sDate.Date = 0x03;
  sDate.Year = 0x20;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
	
}

// Incializacion de interrupcion
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

}

// Funcion para cambiar segundos
void CambiarSeg()
{
	// Lee el valor de el ADC
	// Lo divide entre 69 para tener un aproximado de 59 partes para segundos
	ADC1->CR2|=0x40000000;
	while(!(ADC1->SR&(1<<1)));
	S = (ADC1->DR)/69;
	
	// Va a la funcion para guardar el valor
	MX_RTC_Init();
}

void CambiarMin()
{
	// Lee el valor de el ADC
	// Lo divide entre 69 para tener un aproximado de 59 partes para minutos
	ADC1->CR2|=0x40000000;
	while(!(ADC1->SR&(1<<1)));
	M = ((ADC1->DR)/69); 
	
	// Va a la funcion para guardar el valor
	MX_RTC_Init();
}
void CambiarHr()
{
	// Lee el valor de el ADC
	// Lo divide entre 171 para tener un aproximado de 23 partes para horas
	ADC1->CR2|=0x40000000;
	while(!(ADC1->SR&(1<<1)));	
	H = ((ADC1->DR)/171);
	
	// Va a la funcion para guardar el valor
	MX_RTC_Init();	
}

void Error_Handler(void)
{

}


void assert_failed(uint8_t *file, uint32_t line)
{ 
	
}

