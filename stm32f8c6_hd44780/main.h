


//включить  в main.h
#define PORT_LCD       GPIOB
#define RS 	 	GPIO_Pin_5
#define RW  	GPIO_Pin_4
#define E 	 	GPIO_Pin_3

#define D7		GPIO_Pin_15		//Ўина данных
#define D6		GPIO_Pin_14		//Ўина данных
#define D5		GPIO_Pin_13		//Ўина данных
#define D4		GPIO_Pin_12  	//Ўина данных


#define     PORT_KEY GPIOA
#define		PIN_KEY_1 GPIO_Pin_7
#define		PIN_KEY_2 GPIO_Pin_6
#define		PIN_KEY_3 GPIO_Pin_5
#define		PIN_KEY_4 GPIO_Pin_4
#define		RCC_PORT_KEY  RCC_APB2Periph_GPIOA

#define  PORT_IN_OUT 		GPIOA  //порт управлени€ нагрузкой
#define  PIN_OUT1 		GPIO_Pin_3 //выход управлени€ нагрузкой1
#define  PIN_OUT2 		GPIO_Pin_3 //выход управлени€ нагрузкой2
#define	 RCC_PORT_IN_OUT  RCC_APB2Periph_GPIOA

#define  RCC_PORT_ILL  RCC_APB2Periph_GPIOA
#define  PORT_ILL      GPIOA
#define  PIN_ILL       GPIO_Pin_8

#define  RCC_PORT_BIP  RCC_APB2Periph_GPIOC
#define  PORT_BIP      GPIOC
#define  PIN_BIP       GPIO_Pin_13


#define  PORT_MOTO    GPIOB
#define  X_PIN_STEP    GPIO_Pin_9
#define  X_PIN_DIR    GPIO_Pin_7
#define  X_PIN_EN    GPIO_Pin_6

//************************************************************//
#define  K_UP   2
#define  K_UPD  20
#define  K_DN   3
#define  K_DND  30
#define  K_STOP 1
#define  K_Next 1
#define  K_MENY 4
#define	 K_MENYD 40  //значени€ удерженной кнопки

#define K_DL  15//длительность удержани€ кнопки

//************************************************************//
  void list_0();
  void list_1();
  void LineToL();
  int Okrug(float D);
  void LoadSetings();
//**************************************************************//
  uint32_t   adr;
#define adr_start		    0x0800fC00

  //***********************************************************//

#define Write_settings_to_flash    FLASH_WriteHalfDate(adr_start,&X,sizeof(X))

#define Read_settings_to_flash     FLASH_ReadHalfData(adr_start,&X,sizeof(X))



  //*************************************************************//
