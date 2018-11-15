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
unsigned char simv[]={0x08,0x14,0x08,0x03,0x04,0x04,0x03,0x20};//������ �������
unsigned char moln[]={0x01,0x02,0x04,0x0f,0x02,0x04,0x08,0x10};// ������
unsigned char buf[10];
	int L = 0;//����� ���� ��� ����
	int La = 0;//���� � ���������� + ��� _
	int Lt = 0;//���� � ������������� �����������
	u8 m; //����������
	float  d;
	_Bool DRIV = 0;
	_Bool PUSK = 0;
#define STRT PUSK = 1
#define STP  PUSK = 0

	u8 list=0;//�������

	void TIM4_IRQHandler ();//��������� ����������

	u16   PER_MIN;
	u16   AX;
	float   OB_P;//����� ����� �� ������
	//u16 STEP=0 ;//��� ����� �� ������
	//float P_DLN;//��� ����� �� �������
	int     DLN=0;//����������� ������
	int     GRD=0;//����������� ��������

	  float PUT=0; // ����� ����
	  float W=0; //������� ������� ��� �������� �������

	volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;

int main(void)
{
	//SystemInit();
	//SetSysClockTo72();

	Read_settings_to_flash;//��������� ��������� �� ������
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
	LCD_PUTS("����������");
	delay_s(2);
	LCD_CLEAR;//�������
	LCD_GOTO(5,0);
	LCD_PUTS("������");
	delay_s(2);
	LCD_CLEAR;//�������
	LCD_GOTO(1,0);
	LCD_PUTS("���������� ���");
	LCD_GOTO(5,1);
	LCD_PUTS("�����");

	delay_s(3);



	while(1){

//******************************************************************************//
//******************************************************************************//
	if(list == 0){//��������� ����� �������� ������
		Read_settings_to_flash;//��������� ��������� �� ������
		LoadSetings();
			LCD_CLEAR;//�������
				//list_0 ();

//****************************************************************************//
	while(list==0){

		list_0();
		key_sk();

		//������� �������
//****************************************************************************//
		if(keys == K_UP||keys == K_DN)
		{//���������� ��������� ��������
			LoadSetings();
			PER = NPER;

			if(GRD){ Line = (OB_P*GRD)/3600;}//���������� ���� �� ��������
			if(DLN){ Line =  OB_P/DLN;}//���������� ����

				   LineToL(); //������������� ���� ������������ �����������

				   if(L)
				{  // La = NPER/AX; //���������� ���� �� ������� �������
					d=NPER;//��������� ��������
					La=0;////��������� ��������
				while(d > PER_MIN+(d/AX)) {d = d - (d/AX++);La++;}
				AX = X.AX_P;

				if(2*La > L){Lt = L/2;}  			 //��������� ���� ����������� ��������
				else {Lt = L - La;}		  			// ��������� ���� �� ������ ����������

				}
				   else DRIV = 1;
				STRT;
				//Lt=1;;
				TIM4->CCR4 = STEP;
				TIM4->ARR = PER; //��������� ���������� ��������� ���� ���
				//TIM_Cmd(TIM4, ENABLE); //��������� ������
				StartMotor();
				}
		//*************************************************************************************//
		while (PUSK) { //��� �� ����� ���� ��� ���������� ������ ��� ��� ����
					key_sk();
					if(keys == K_STOP){keys = 0 ; TIM_Cmd (TIM4, DISABLE); L=0; Lt=0; STP; break;}
					if(keys == K_DN && DR == X.D_PAR-1){DRIV = 1;}
					else {if(keys == K_UP && DR == X.D_PAR)  {DRIV = 1;}else {DRIV = 0;}}
					//�� ��� ������������, ����� ������ �����������
					list_0();}

		//****************************************************************************************//
		 if(keys == K_MENY) {list=100;LCD_CLEAR; delay_ms(500);}
		 if(keys == K_STOP) {PUT=0;}
			}

	}
	//**********************************************************************************************//
	//***********************************************************************************************//
	while(list==100){
						StoptMotor();//��������� ������� ����� ��� �� �������� ������ ������������
		 	 	 	 LCD_HOME;
					 LCD_PUTS("��������� <DLN>");

				    LCD_GOTO(0,1);
				    LCD_PUTS("R*");
					key_st(K_DL);
					if(keys == K_MENY){m++;}//������� ����������
					if(m>4){m=0;}

					if(keys == K_UP){keys=0; DLN += pow(10, m);if(DLN>99999){DLN=0;}} //10 � �������� � ��
					if(keys == K_DN){keys=0; DLN -= pow(10, m);if(DLN<0){DLN=0;}}

					 _sprtffd(0,buf,pow(10, m)); //������ ������� ������ � ��������
					 LCD_GOTO(2,1);LCD_PUTST(buf,5);

					 _sprtffd(0,buf,DLN); //������ ������� ������ � ��������
					LCD_GOTO(10,1);LCD_PUTST(buf,5);
					if(DLN){GRD=0;}
					Line=0;//�������� ����
					delay_ms(200);

					if(keys == K_Next){list++;LCD_CLEAR; delay_ms(500);m = 0;}
	}

	//***********************************************************************************************//
	while(list==101){
						LCD_HOME;
						LCD_PUTS("��������� <GRD>");

						LCD_GOTO(0,1);
					    LCD_PUTS("R*");
						key_st(K_DL);
						if(keys == K_MENY){m++;}//������� ����������
						if(m>4){m=0;}


						if(keys == K_UP){keys=0; GRD += pow(10, m);if(GRD>36000){GRD=0;}} //10 � �������� � ��
						if(keys == K_DN){keys=0; GRD -= pow(10, m);if(GRD<0){GRD=0;}}

						 _sprtffd(1,buf,pow(10, m)); //������ ������� ������ � ��������
						 chek_str(buf,5);
						 LCD_GOTO(2,1);LCD_PUTST(buf,5);

						 _sprtffd(1,buf,GRD); //������ ������� ������ � ��������
						 chek_str(buf,5);
						LCD_GOTO(10,1);LCD_PUTST(buf,5);

						if(GRD){DLN=0;  /*Line = (OB_P*GRD)/3600;*/}//���������� ����
						delay_ms(200);

						if(keys == K_Next){list++;LCD_CLEAR; delay_ms(500);m = 0;PUT=0;}
		}
	//***********************************************************************************************//
	while(list==102){
				 LCD_HOME;
								  LCD_PUTS("���������� ���������?");
								  LCD_GOTO(0,1); LCD_PUTS("��");
								  LCD_GOTO(12,1);  LCD_PUTS("���");
								  key_st(K_DL);
							if(keys == K_UP){list = 1;LCD_CLEAR; delay_ms(500);m = 0;}
							if(keys == K_DN){list = 0;LCD_CLEAR; delay_ms(500);}
			}
	//***********************************************************************************************//
	//**********************************************************************************************//
		while(list==1){
				  LCD_HOME;
				  LCD_PUTS("�����/�� ������");

			LCD_GOTO(0,1);
			LCD_PUTS("R*");
			key_st(K_DL);
			if(keys == K_MENY){m++;}//������� ����������
			if(m>4){m=0;}

			if(keys == K_UP){keys=0; X.OB_P += pow(10, m);if(X.OB_P>99999){X.OB_P=0;}} //10 � �������� � ��
			if(keys == K_DN){keys=0; X.OB_P -= pow(10, m);if(X.OB_P<0){X.OB_P=0;}}

			 _sprtffd(0,buf,pow(10, m)); //������ ������� ������ � ��������
			 LCD_GOTO(2,1);LCD_PUTST(buf,5);

			 _sprtffd(0,buf,X.OB_P); //������ ������� ������ � ��������
			LCD_GOTO(10,1);LCD_PUTST(buf,5);

			delay_ms(200);
			if(keys == K_Next){list ++;LCD_CLEAR; delay_ms(500);m = 0;}
						}

		//************��������� ��������� DIR********************************************************************//
		while(list==2){

					LCD_HOME;
				  	LCD_PUTS("����������� <D> ");

				    LCD_GOTO(0,1);
				  	LCD_PUTS("D = ");
					key_st(K_DL);			//������� �������

					if(keys == K_UP){ X.D_PAR = 1;}
					if(keys == K_DN){ X.D_PAR = 0;}

					 _sprtffd(0,buf,X.D_PAR); //������ ������� ������ � ��������
					//LCD_GOTO(4,1);
					 LCD_PUTST(buf,5);

					delay_ms(200);
					if(keys == K_Next){list++;LCD_CLEAR; delay_ms(500);}
								}



		//*****************��������� ��������� ENABL************************************************************//
				while(list==3){

					LCD_HOME;
					LCD_PUTS("ENABLED <E> ");

					LCD_GOTO(0,1);
					LCD_PUTS("E = ");
					key_st(K_DL);			//������� �������
					if(keys == K_UP){ X.E_PAR = 1;}

				if(keys == K_DN){ X.E_PAR = 0;}

		 _sprtffd(0,buf,X.E_PAR); //������ ������� ������ � ��������
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
								key_st(K_DL);			//������� �������
								if(keys == K_UP){ X.S_PAR = 1;}

								if(keys == K_DN){ X.S_PAR = 0;}

							 _sprtffd(0,buf,X.S_PAR); //������ ������� ������ � ��������
							 LCD_PUTST(buf,5);
							 //��������������� ������

						 INI_MOTO_TIM();

						delay_ms(200);
					if(keys == K_Next){list++;LCD_CLEAR; delay_ms(500);}
				}

		//*****************��������� ��������� <SPID>***********************************************************//
				if(list==5) d = Fmax / X.V_PER_MIN;//���� � �������
				while(list==5){
						  LCD_HOME;
						  LCD_PUTS("�������� <Vmax>");

						LCD_GOTO(0,1);
						LCD_PUTS("R*");
						key_st(K_DL);
						if(keys == K_MENY){m++;}//������� ����������
						if(m>4){m=0;}

						if(keys == K_UP){keys=0; d += pow(10, m);if(d>99999){d=0;}} //10 � �������� � ��
						if(keys == K_DN){keys=0; d -= pow(10, m);if(d<0){d=0;}}

					 _sprtffd(0,buf,pow(10, m)); //������ ������� ������ � ��������
					 LCD_GOTO(2,1);LCD_PUTST(buf,5);

					 _sprtffd(0,buf,d); //�������� ��� � �/
					LCD_GOTO(10,1);LCD_PUTST(buf,5);
					X.V_PER_MIN = Fmax /d;//�������� � �������� ��������/
					if(keys == K_Next){list++;LCD_CLEAR; delay_ms(500);m = 0;}
							}


		//*******************************************************************************************//


					if(list==6){d = 0xff-X.AX_P;} // ���������

				while(list==6){
					  LCD_HOME;
					  LCD_PUTS("��������� <Ax>");
					  LCD_GOTO(0,1);
					  LCD_PUTS("R*");

				key_st(K_DL);

					if(keys == K_MENY){m++;}//������� ����������
					if(m>4){m=0;}

					if(keys == K_UP){keys=0; d += pow(10, m);if(d>0xff-1){d=1;}} //10 � �������� � ��
					if(keys == K_DN){keys=0; d -= pow(10, m);if(d<0){d=1;}}

					 _sprtffd(0,buf,pow(10, m)); //������ ������� ������ � ��������
					 LCD_GOTO(2,1);LCD_PUTST(buf,5);
					 X.AX_P = 0xff-d;
					 //d=  Fmax/(0xFFFF - AX);
					 //AX = 0xFFFF - (Fmax/d);
					 _sprtffd(0,buf,d); //������ ������� ��
					LCD_GOTO(10,1);LCD_PUTST(buf,5);

					delay_ms(200);
					if(keys == K_Next){list++; LCD_CLEAR; delay_ms(500);m = 0;PUT=0;//�������� ���������� ��� �������������

									}
											}

		//*******************************************************************************************//
				while(list==7){
					  LCD_HOME;
				  LCD_PUTS("����.��� <STEP>");
					  LCD_GOTO(0,1);
					  LCD_PUTS("R*");

					key_st(K_DL);

				if(keys == K_MENY){m++;}//������� ����������
					if(m>4){m=0;}

				if(keys == K_UP){keys=0; X.STEP += pow(10, m);if(X.STEP>0xFFFE){X.STEP=0;}} //10 � �������� � ��
				if(keys == K_DN){keys=0; X.STEP -= pow(10, m);if(X.STEP<0){X.STEP=0;}}

					 _sprtffd(0,buf,pow(10, m)); //������ ������� ������ � ��������
					 LCD_GOTO(2,1);LCD_PUTST(buf,5);

				 _sprtffd(0,buf,X.STEP); //������ ������� ������ � ��������
					LCD_GOTO(10,1);LCD_PUTST(buf,5);

					delay_ms(200);
					//************������� � ���������*******************************************//
					if(keys == K_Next){list=0; LCD_CLEAR;LCD_HOME;
					LCD_PUTS("   ���������");  Write_settings_to_flash;
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

 			if(!L){	//DRIV ���� ������ ������
 				if(!DR){PUT++;}else {PUT--;} //������� ��������� ����
 				if(PER > PER_MIN && DRIV){if(PER > STEP+(PER/AX)) {PER = PER - (PER/AX++);}/* else{PER=(PER/AX);}*/} //������������� ��������� �� ������ ��������
 			    if(!DRIV){if(PER < NPER-(PER/AX)){ PER = PER + (PER/AX--);}
 			    				else {TIM_Cmd (TIM4, DISABLE); STP;PER=0;BipStop();return;}} //������������ ��������� �� ����� ��������

 			}
 			else{
 				//if(!L){TIM_Cmd (TIM4, DISABLE);STP;PER=0;BipStop(); return;} //������������� , �������
 				if(PER> PER_MIN && Lt){if(PER > STEP+(PER/AX)) {PER = PER - (PER/AX++);}/*else{PER=(PER/AX);}*/} //������������� ���������
				if(PER <=0xFFFE &&!Lt){if(PER < NPER-(PER/AX)) {PER = PER + (PER/AX--);}/*else{PER=(PER/AX);}*/} //������������ ���������
				if(!DR){PUT++;}else {PUT--;}
				if(Lt){Lt--;} L--;
				if(!L){TIM_Cmd (TIM4, DISABLE);STP;PER=0;BipStop(); return;} //������������� , �������

 			}

 					TIM4->ARR = PER; //��������� ���������� ��������� ���� ���
 			    TIM_Cmd(TIM4, ENABLE); //��������� ������
 	}

   }
//**************************************************************************************************//
  void list_0()
  {					//LCD_CLEAR;//��

					OB_P = X.OB_P;//����������� ������ �� ������
					float P_GRD = OB_P/3600;
					float P_DLN = OB_P/DLN;


					LCD_GOTO(0,0);
					LCD_PUTS("��= ");
					_sprtffd(1,buf,Okrug(PUT/P_GRD));//������ ������� ������ � ��������
					chek_str(buf,5);
					LCD_PUTST(buf,5);

					LCD_GOTO(9,0);
					LCD_PUTS("��=");
					_sprtffd(0,buf,Okrug(PUT/P_DLN));//������ ������� ������ � ��������
					//chek_str(buf,5);
					LCD_PUTST(buf,5);


		if(DLN){	LCD_GOTO(0,1);
					LCD_PUTS("DLN="); //������� �� ������
					_sprtffd(0,buf,DLN);
					//chek_str(buf,2);
					LCD_PUTST(buf,5);}
		else {
					LCD_GOTO(0,1);
					LCD_PUTS("GRD="); //������ �� ������
					_sprtffd(1,buf,GRD);
					chek_str(buf,5);
					LCD_PUTST(buf,5);}

 					LCD_GOTO(9,1);
					LCD_PUTS("Vp=");
					_sprtffd(0,buf,Fmax/PER);// �������� �����
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
  		  	 A = Line;//�������� ������������� ��������
  		  	 ostatok = Line-A;//���������� �������

  			 	 W+=ostatok;//���������� �������
  			  if(W>1){L++;W--;}
  			if( PUT+L == OB_P-1){L++;}

  		  }


//**************************************************************************************************//
  	int Okrug(float D){
  		 int A=0;
  		 float O =0;
  		 A = D;//��������� ��� �������
  		 O = D - A;//�������� �������
  		 if(O>0.5){O = A++;}
  		 return A;
  	 }

  //**********************************************************************************************//
  	void LoadSetings(){
  						DR = X.D_PAR;
  						if(keys == K_DN)DR--;
  						EN = X.E_PAR;  //�������� ��������� ��� ���
  						AX = X.AX_P;	// ���������
  						STEP = X.STEP;
  						OB_P = X.OB_P;
  						PER_MIN = X.V_PER_MIN;
  						PER = 0;
  						S = X.S_PAR;

  	}
