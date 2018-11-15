///*******************************//

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_conf.h"
#include "system_stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_flash.h"
#include "flash.h"
//*********************************//
#include "Key.h"
#include "main.h"
#include "delay.h"
#include "lcd_44780.h"
#include "prtf.h"
#include "stepmotor.h"
//********************************//
unsigned char simv[]={0x08,0x14,0x08,0x03,0x04,0x04,0x03,0x20};//градус сельсия
unsigned char moln[]={0x01,0x02,0x04,0x0f,0x02,0x04,0x08,0x10};// молния
unsigned char buf[10];
	int L = 0;//общий путь для осей
	int La = 0;//путь с ускорением + или _
	int Lt = 0;//путь с отрицательным торможением
	u8 m; //знакоместо
	float  d;
	_Bool DRIV = 0;
	_Bool PUSK = 0;
#define STRT PUSK = 1
#define STP  PUSK = 0

	u8 list=0;//менюшка

	void TIM4_IRQHandler ();//обработка прерываний

	u16   PER_MIN;
	u16   AX;
	float   OB_P;//число пинов на оборот
	//u16 STEP=0 ;//кол шагов на градус
	//float P_DLN;//кол шагов на деление
	int     DLN=0;//колличество делеий
	int     GRD=0;//колличество градусов

	  float PUT=0; // общий путь
	  float W=0; //внешний остаток при нечётном делении

	volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;

int main(void)
{
	//SystemInit();
	//SetSysClockTo72();

	Read_settings_to_flash;//выгружаем настройки из памяти
	INI_STEP_MOTO();
	delay_init(72);
	delay_ms(100);
	key_ini();
	LCD_INI();

//	LCD_SETPC(moln);

	LCD_GOTO(0,0);
//	LCD_PUTC(0);
	//BipExit();
	LCD_GOTO(3,0);
	LCD_PUTS("STEP MOTOR");
	LCD_GOTO(3,1);
	LCD_PUTS("КОНТРОЛЛЕР");
	delay_s(2);
	LCD_CLEAR;//очистка
	LCD_GOTO(5,0);
	LCD_PUTS("АХТУНГ");
	delay_s(2);
	LCD_CLEAR;//очистка
	LCD_GOTO(1,0);
	LCD_PUTS("СПЕЦИАЛЬНО ДЛЯ");
	LCD_GOTO(5,1);
	LCD_PUTS("МГЕРА");

	delay_s(3);



	while(1){

//******************************************************************************//
//******************************************************************************//
	if(list == 0){//первичный экран загрузка данных
		Read_settings_to_flash;//выгружаем настройки из памяти
		LoadSetings();
			LCD_CLEAR;//очистка
				//list_0 ();

//****************************************************************************//
	while(list==0){

		list_0();
		key_sk();

		//двигаем менюшку
//****************************************************************************//
		if(keys == K_UP||keys == K_DN)
		{//определяем параметры движения
			LoadSetings();
			PER = NPER;

			if(GRD){ Line = (OB_P*GRD)/3600;}//отределяем путь по градусам
			if(DLN){ Line =  OB_P/DLN;}//отределяем путь

				   LineToL(); //устанавливаем путь корректируем погрешность

				   if(L)
				{  // La = NPER/AX; //определяем путь до полного газгона
					d=NPER;//начальное значение
					La=0;////начальное значение
				while(d > PER_MIN+(d/AX)) {d = d - (d/AX++);La++;}
				AX = X.AX_P;

				if(2*La > L){Lt = L/2;}  			 //проверяем путь спостоянной скорости
				else {Lt = L - La;}		  			// вычисляем путь до начала торможения

				}
				   else DRIV = 1;
				STRT;
				//Lt=1;;
				TIM4->CCR4 = STEP;
				TIM4->ARR = PER; //загружаем переменные запускаем один шаг
				//TIM_Cmd(TIM4, ENABLE); //запускаем таймер
				StartMotor();
				}
		//*************************************************************************************//
		while (PUSK) { //ждём до конка пути или отпускания кнопки или ком СТОП
					key_sk();
					if(keys == K_STOP){keys = 0 ; TIM_Cmd (TIM4, DISABLE); L=0; Lt=0; STP; break;}
					if(keys == K_DN && DR == X.D_PAR-1){DRIV = 1;}
					else {if(keys == K_UP && DR == X.D_PAR)  {DRIV = 1;}else {DRIV = 0;}}
					//не даём остановиться, потом меняем направление
					list_0();}

		//****************************************************************************************//
		 if(keys == K_MENY) {list=100;LCD_CLEAR; delay_ms(500);}
		 if(keys == K_STOP) {PUT=0;}
			}

	}
	//**********************************************************************************************//
	//***********************************************************************************************//
	while(list==100){
						StoptMotor();//отключаем драйвер мотра что бы избежать ложных срабатываний
		 	 	 	 LCD_HOME;
					 LCD_PUTS("УСТАНОВКА <DLN>");

				    LCD_GOTO(0,1);
				    LCD_PUTS("R*");
					key_st(K_DL);
					if(keys == K_MENY){m++;}//двигаем знакоместо
					if(m>4){m=0;}

					if(keys == K_UP){keys=0; DLN += pow(10, m);if(DLN>99999){DLN=0;}} //10 в квадрате и тд
					if(keys == K_DN){keys=0; DLN -= pow(10, m);if(DLN<0){DLN=0;}}

					 _sprtffd(0,buf,pow(10, m)); //рисуем сколько прошол в делениях
					 LCD_GOTO(2,1);LCD_PUTST(buf,5);

					 _sprtffd(0,buf,DLN); //рисуем сколько прошол в делениях
					LCD_GOTO(10,1);LCD_PUTST(buf,5);
					if(DLN){GRD=0;}
					Line=0;//обнуляем путь
					delay_ms(200);

					if(keys == K_Next){list++;LCD_CLEAR; delay_ms(500);m = 0;}
	}

	//***********************************************************************************************//
	while(list==101){
						LCD_HOME;
						LCD_PUTS("УСТАНОВКА <GRD>");

						LCD_GOTO(0,1);
					    LCD_PUTS("R*");
						key_st(K_DL);
						if(keys == K_MENY){m++;}//двигаем знакоместо
						if(m>4){m=0;}


						if(keys == K_UP){keys=0; GRD += pow(10, m);if(GRD>36000){GRD=0;}} //10 в квадрате и тд
						if(keys == K_DN){keys=0; GRD -= pow(10, m);if(GRD<0){GRD=0;}}

						 _sprtffd(1,buf,pow(10, m)); //рисуем сколько прошол в делениях
						 chek_str(buf,5);
						 LCD_GOTO(2,1);LCD_PUTST(buf,5);

						 _sprtffd(1,buf,GRD); //рисуем сколько прошол в делениях
						 chek_str(buf,5);
						LCD_GOTO(10,1);LCD_PUTST(buf,5);

						if(GRD){DLN=0;  /*Line = (OB_P*GRD)/3600;*/}//отределяем путь
						delay_ms(200);

						if(keys == K_Next){list++;LCD_CLEAR; delay_ms(500);m = 0;PUT=0;}
		}
	//***********************************************************************************************//
	while(list==102){
				 LCD_HOME;
								  LCD_PUTS("ПРОДОЛЖИТЬ НАСТРОЙКИ?");
								  LCD_GOTO(0,1); LCD_PUTS("ДА");
								  LCD_GOTO(12,1);  LCD_PUTS("НЕТ");
								  key_st(K_DL);
							if(keys == K_UP){list = 1;LCD_CLEAR; delay_ms(500);m = 0;}
							if(keys == K_DN){list = 0;LCD_CLEAR; delay_ms(500);}
			}
	//***********************************************************************************************//
	//**********************************************************************************************//
		while(list==1){
				  LCD_HOME;
				  LCD_PUTS("ШАГОВ/НА ОБОРОТ");

			LCD_GOTO(0,1);
			LCD_PUTS("R*");
			key_st(K_DL);
			if(keys == K_MENY){m++;}//двигаем знакоместо
			if(m>4){m=0;}

			if(keys == K_UP){keys=0; X.OB_P += pow(10, m);if(X.OB_P>99999){X.OB_P=0;}} //10 в квадрате и тд
			if(keys == K_DN){keys=0; X.OB_P -= pow(10, m);if(X.OB_P<0){X.OB_P=0;}}

			 _sprtffd(0,buf,pow(10, m)); //рисуем сколько прошол в делениях
			 LCD_GOTO(2,1);LCD_PUTST(buf,5);

			 _sprtffd(0,buf,X.OB_P); //рисуем сколько прошол в делениях
			LCD_GOTO(10,1);LCD_PUTST(buf,5);

			delay_ms(200);
			if(keys == K_Next){list ++;LCD_CLEAR; delay_ms(500);m = 0;}
						}

		//************Установка параметра DIR********************************************************************//
		while(list==2){

					LCD_HOME;
				  	LCD_PUTS("НАПРАВЛЕНИЕ <D> ");

				    LCD_GOTO(0,1);
				  	LCD_PUTS("D = ");
					key_st(K_DL);			//двигаем менюшку

					if(keys == K_UP){ X.D_PAR = 1;}
					if(keys == K_DN){ X.D_PAR = 0;}

					 _sprtffd(0,buf,X.D_PAR); //рисуем сколько прошол в делениях
					//LCD_GOTO(4,1);
					 LCD_PUTST(buf,5);

					delay_ms(200);
					if(keys == K_Next){list++;LCD_CLEAR; delay_ms(500);}
								}



		//*****************Установка параметра ENABL************************************************************//
				while(list==3){

					LCD_HOME;
					LCD_PUTS("ENABLED <E> ");

					LCD_GOTO(0,1);
					LCD_PUTS("E = ");
					key_st(K_DL);			//двигаем менюшку
					if(keys == K_UP){ X.E_PAR = 1;}

				if(keys == K_DN){ X.E_PAR = 0;}

		 _sprtffd(0,buf,X.E_PAR); //рисуем сколько прошол в делениях
		 LCD_PUTST(buf,5);
			delay_ms(200);
			if(keys == K_Next){list++;LCD_CLEAR; delay_ms(500);}
		}
				//***************************************************************************//

				while(list==4){

									LCD_HOME;
								LCD_PUTS("ENABLED <S> ");

								LCD_GOTO(0,1);
								LCD_PUTS("S = ");
								key_st(K_DL);			//двигаем менюшку
								if(keys == K_UP){ X.S_PAR = 1;}

								if(keys == K_DN){ X.S_PAR = 0;}

							 _sprtffd(0,buf,X.S_PAR); //рисуем сколько прошол в делениях
							 LCD_PUTST(buf,5);
							 //перенастраиваем таймер

						 INI_MOTO_TIM();

						delay_ms(200);
					if(keys == K_Next){list++;LCD_CLEAR; delay_ms(500);}
				}

		//*****************Установка параметра <SPID>***********************************************************//
				if(list==5) d = Fmax / X.V_PER_MIN;//степ в секунду
				while(list==5){
						  LCD_HOME;
						  LCD_PUTS("СКОРОСТЬ <Vmax>");

						LCD_GOTO(0,1);
						LCD_PUTS("R*");
						key_st(K_DL);
						if(keys == K_MENY){m++;}//двигаем знакоместо
						if(m>4){m=0;}

						if(keys == K_UP){keys=0; d += pow(10, m);if(d>99999){d=0;}} //10 в квадрате и тд
						if(keys == K_DN){keys=0; d -= pow(10, m);if(d<0){d=0;}}

					 _sprtffd(0,buf,pow(10, m)); //рисуем сколько прошол в делениях
					 LCD_GOTO(2,1);LCD_PUTST(buf,5);

					 _sprtffd(0,buf,d); //СКОРОСТЬ РИН В С/
					LCD_GOTO(10,1);LCD_PUTST(buf,5);
					X.V_PER_MIN = Fmax /d;//преводим в значение счётчика/
					if(keys == K_Next){list++;LCD_CLEAR; delay_ms(500);m = 0;}
							}


		//*******************************************************************************************//


					if(list==6){d = 0xff-X.AX_P;} // ускорение

				while(list==6){
					  LCD_HOME;
					  LCD_PUTS("УСКОРЕНИЕ <Ax>");
					  LCD_GOTO(0,1);
					  LCD_PUTS("R*");

				key_st(K_DL);

					if(keys == K_MENY){m++;}//двигаем знакоместо
					if(m>4){m=0;}

					if(keys == K_UP){keys=0; d += pow(10, m);if(d>0xff-1){d=1;}} //10 в квадрате и тд
					if(keys == K_DN){keys=0; d -= pow(10, m);if(d<0){d=1;}}

					 _sprtffd(0,buf,pow(10, m)); //рисуем сколько прошол в делениях
					 LCD_GOTO(2,1);LCD_PUTST(buf,5);
					 X.AX_P = 0xff-d;
					 //d=  Fmax/(0xFFFF - AX);
					 //AX = 0xFFFF - (Fmax/d);
					 _sprtffd(0,buf,d); //рисуем сколько пр
					LCD_GOTO(10,1);LCD_PUTST(buf,5);

					delay_ms(200);
					if(keys == K_Next){list++; LCD_CLEAR; delay_ms(500);m = 0;PUT=0;//обнуляем переменную для использования

									}
											}

		//*******************************************************************************************//
				while(list==7){
					  LCD_HOME;
				  LCD_PUTS("ДЛИТ.ИМП <STEP>");
					  LCD_GOTO(0,1);
					  LCD_PUTS("R*");

					key_st(K_DL);

				if(keys == K_MENY){m++;}//двигаем знакоместо
					if(m>4){m=0;}

				if(keys == K_UP){keys=0; X.STEP += pow(10, m);if(X.STEP>0xFFFE){X.STEP=0;}} //10 в квадрате и тд
				if(keys == K_DN){keys=0; X.STEP -= pow(10, m);if(X.STEP<0){X.STEP=0;}}

					 _sprtffd(0,buf,pow(10, m)); //рисуем сколько прошол в делениях
					 LCD_GOTO(2,1);LCD_PUTST(buf,5);

				 _sprtffd(0,buf,X.STEP); //рисуем сколько прошол в делениях
					LCD_GOTO(10,1);LCD_PUTST(buf,5);

					delay_ms(200);
					//************выходим и сохраняем*******************************************//
					if(keys == K_Next){list=0; LCD_CLEAR;LCD_HOME;
					LCD_PUTS("   СОХРАНЯЕМ");  Write_settings_to_flash;
					delay_s(1);   m = 0;}
				}

		//******************************************************************************************//

		//******************************************************************************************//


	}
}
//************************************************************************************************//
//**************************************************************************************************//
//****************************************************************************************************//
  void TIM4_IRQHandler ()
   {
		TIM_Cmd (TIM4, DISABLE);
 	if(TIM_GetITStatus(TIM4, TIM_IT_Update)== SET)
 	{
 		TIM_ClearITPendingBit (TIM4, TIM_IT_Update );

 			if(!L){	//DRIV пока нажата кнопка
 				if(!DR){PUT++;}else {PUT--;} //счетаем пройденый путь
 				if(PER > PER_MIN && DRIV){if(PER > STEP+(PER/AX)) {PER = PER - (PER/AX++);}/* else{PER=(PER/AX);}*/} //положительное ускорение до максим скорость
 			    if(!DRIV){if(PER < NPER-(PER/AX)){ PER = PER + (PER/AX--);}
 			    				else {TIM_Cmd (TIM4, DISABLE); STP;PER=0;BipStop();return;}} //отрицательно ускорение до миним скорость

 			}
 			else{
 				//if(!L){TIM_Cmd (TIM4, DISABLE);STP;PER=0;BipStop(); return;} //останавливаем , выходим
 				if(PER> PER_MIN && Lt){if(PER > STEP+(PER/AX)) {PER = PER - (PER/AX++);}/*else{PER=(PER/AX);}*/} //положительное ускорение
				if(PER <=0xFFFE &&!Lt){if(PER < NPER-(PER/AX)) {PER = PER + (PER/AX--);}/*else{PER=(PER/AX);}*/} //отрицательно ускорение
				if(!DR){PUT++;}else {PUT--;}
				if(Lt){Lt--;} L--;
				if(!L){TIM_Cmd (TIM4, DISABLE);STP;PER=0;BipStop(); return;} //останавливаем , выходим

 			}

 					TIM4->ARR = PER; //загружаем переменные запускаем один шаг
 			    TIM_Cmd(TIM4, ENABLE); //запускаем таймер
 	}

   }
//**************************************************************************************************//
  void list_0()
  {					//LCD_CLEAR;//оч

					OB_P = X.OB_P;//колличество степов на оборот
					float P_GRD = OB_P/3600;
					float P_DLN = OB_P/DLN;


					LCD_GOTO(0,0);
					LCD_PUTS("ГР= ");
					_sprtffd(1,buf,Okrug(PUT/P_GRD));//рисуем сколько прошол в градусах
					chek_str(buf,5);
					LCD_PUTST(buf,5);

					LCD_GOTO(9,0);
					LCD_PUTS("ДЛ=");
					_sprtffd(0,buf,Okrug(PUT/P_DLN));//рисуем сколько прошол в делениях
					//chek_str(buf,5);
					LCD_PUTST(buf,5);


		if(DLN){	LCD_GOTO(0,1);
					LCD_PUTS("DLN="); //делений на оборот
					_sprtffd(0,buf,DLN);
					//chek_str(buf,2);
					LCD_PUTST(buf,5);}
		else {
					LCD_GOTO(0,1);
					LCD_PUTS("GRD="); //градус на сектор
					_sprtffd(1,buf,GRD);
					chek_str(buf,5);
					LCD_PUTST(buf,5);}

 					LCD_GOTO(9,1);
					LCD_PUTS("Vp=");
					_sprtffd(0,buf,Fmax/PER);// скорость пинах
					//_sprtffd(0,buf,PER_MIN);
					//chek_str(buf,5);
					LCD_PUTST(buf,5);
	 }
 //**************************************************************************************************//

  	 void LineToL(){

  		 L=Line;
  		 if(!L){return;}
  		  int A=0;
  		  float ostatok =0;
  		  	 A = Line;//получаем целочисленное значение
  		  	 ostatok = Line-A;//десятичный остаток

  			 	 W+=ostatok;//складываем остатки
  			  if(W>1){L++;W--;}
  			if( PUT+L == OB_P-1){L++;}

  		  }


//**************************************************************************************************//
  	int Okrug(float D){
  		 int A=0;
  		 float O =0;
  		 A = D;//переводим без остатка
  		 O = D - A;//получаем остаток
  		 if(O>0.5){O = A++;}
  		 return A;
  	 }

  //**********************************************************************************************//
  	void LoadSetings(){
  						DR = X.D_PAR;
  						if(keys == K_DN)DR--;
  						EN = X.E_PAR;  //значение параметра для оси
  						AX = X.AX_P;	// ускорение
  						STEP = X.STEP;
  						OB_P = X.OB_P;
  						PER_MIN = X.V_PER_MIN;
  						PER = 0;
  						S = X.S_PAR;

  	}
