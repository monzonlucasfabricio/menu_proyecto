#include <Arduino.h>
#include <EEPROM.h>
#include "button.hpp"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define MAXITEMS 15
#define ROWNUM 2
#define COLNUM 16

#define MAINMENU_NUM 3
#define AJUSTES_NUM 6
#define MEDICION_NUM 2

const char menu[MAINMENU_NUM][MAXITEMS] = {"Ajustes","Medicion","Ult. Medidas"};
const char ajustes[AJUSTES_NUM][MAXITEMS] = {"Cfg. Helices","Cfg. Periodo","Ref. Lugar","Cfg. Date","Buzzer","Atras"};
const char medicion[MEDICION_NUM][MAXITEMS] = {"Inicio","Atras"};

typedef enum{
	OUT,
    AJUSTES,
	MEDICION,
	ULT_MEDIDAS,
	CFG_HELICES,
	CFG_PERIODO,
	REF_LUGAR,
	CFG_DATE,
	BUZZER,
	ATRAS_AJUSTES,
	INICIO_MEDICION,
	ATRAS_MEDICION,
	TOMAR_MEDICION
}Menu_e;

typedef enum{
	MAIN,
	AJUSTES_SUBMENU,
	MEDICION_SUBMENU
}Menu_state_e;

typedef enum{
    A_ELISE = 0,
    B_ELISE = 1,
	PERIODO = 2
}address_t;

typedef enum{
    IN = -1,
    ROW_1 = 0,
    ROW_2 = 1,
    OUT_ROW = 2
}row_t;

typedef enum{
	DONTMOVE = 0,
	UP = 1,
	DOWN = 2,
	ENTER = 3
}move_t;

const bool pullup = true;
const int up_button = 2;
const int down_button = 3;
const int enter_button = 4;
move_t buttonProcess = DONTMOVE;

Button Up(up_button,pullup);
Button Down(down_button,pullup);
Button Enter(enter_button,pullup);

Menu_e estado_actual;
Menu_e estado_anterior;
Menu_state_e menu_submenu_state;
row_t ROW_STATUS;


void SetEEPROMValue(address_t address, float value);
float GetEEPROMValue(address_t address);
move_t CheckButton(void);
bool lcd_UpdateCursor(Menu_e Menu, int row, int col);
void lcd_ClearOneLine(int row);
void lcd_ClearCursor(int row);
void lcd_DisplayMenu(Menu_e Menu, Menu_state_e menu_submenu_state);
void lcd_PrintCursor(Menu_state_e menu_submenu_state, uint8_t start, uint8_t count, uint8_t cursorPosition);
void StateMachine_Control(Menu_e Menu, Menu_state_e menu_submenu_state);

LiquidCrystal_I2C lcd(0x27,16,2);

void setup()
{
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  Serial.begin(9600);

  estado_actual = AJUSTES;
  estado_anterior = AJUSTES;
  menu_submenu_state = MAIN;

  SetEEPROMValue(PERIODO,10000);
  lcd.clear();
  lcd.setCursor(0,ROW_STATUS);
  lcd.print(">");
  lcd_DisplayMenu(estado_actual, menu_submenu_state);
}

void loop(void)
{
	bool ret = 0;
	while(true)
	{
		ret = lcd_UpdateCursor(estado_actual,ROWNUM,COLNUM);
		if (ret == 1){
			lcd_DisplayMenu(estado_actual,menu_submenu_state);
		}
		StateMachine_Control(estado_actual,menu_submenu_state);
	}
}


float GetEEPROMValue(address_t address)
{
    float val; 
    float value = EEPROM.get(address, val);
    return value;
}

void SetEEPROMValue(address_t address, float value)
{
    EEPROM.put(address, value);
}

move_t CheckButton(void)
{
    if (Up.check() == LOW)
    {
      return UP;
    }
    else if (Down.check() == LOW)
    {
      return DOWN;
    }
    else if (Enter.check() == LOW)
    {
      return ENTER;
    }
    else
    {
        // DONT MOVE
    }
  return DONTMOVE;
}

bool lcd_UpdateCursor(Menu_e Menu, int row, int col)
{
	static move_t lastButtonProcess = DONTMOVE;
	static Menu_e firstMenu = AJUSTES;
	static Menu_e lastMenu = ULT_MEDIDAS;
	static Menu_state_e lastMenuState = MAIN;

	buttonProcess = CheckButton();

	if (buttonProcess != DONTMOVE)
	{
		lastButtonProcess = buttonProcess;
		if (buttonProcess == DOWN){
			if(estado_actual != lastMenu)
			{
				estado_actual = estado_actual + 1;
			}
		}
		else if (buttonProcess == UP){
			if(estado_actual != firstMenu)
			{
				estado_actual = estado_actual - 1;
			}
		}
		else if (buttonProcess == ENTER)
		{
			Serial.println("Enter");
			if (lastButtonProcess == DOWN || lastButtonProcess == UP)
			{
				lcd.clear();
			}
			switch(estado_actual)
			{
				case AJUSTES:
				{
					menu_submenu_state = AJUSTES_SUBMENU;
					estado_actual = CFG_HELICES;
				}
				break;
				case ATRAS_AJUSTES:
				{
					menu_submenu_state = MAIN;
					estado_actual = AJUSTES;
				}
				break;
				case MEDICION:
				{
					menu_submenu_state = MEDICION_SUBMENU;
					estado_actual = INICIO_MEDICION;	
				}
				break;

				case INICIO_MEDICION:
				{
					menu_submenu_state = MEDICION_SUBMENU;
					estado_actual = TOMAR_MEDICION;
				}
				break;

				case ATRAS_MEDICION:
				{
					menu_submenu_state = MAIN;
					estado_actual = AJUSTES;
				}
				break;
			}
		}

		switch(menu_submenu_state)
		{
		case MAIN: 
			firstMenu = AJUSTES;
			lastMenu = ULT_MEDIDAS; 
			break;
		case AJUSTES_SUBMENU: 
			firstMenu = CFG_HELICES;
			lastMenu = ATRAS_AJUSTES; 
			break;
		case MEDICION_SUBMENU:
			firstMenu = INICIO_MEDICION;
			lastMenu = ATRAS_MEDICION;
			break;
		}
		

		Serial.println("Cambio de estado");
		Serial.println(estado_actual);
		return 1;
	}
	return 0;
}

void lcd_ClearOneLine(int row)
{
	for(uint8_t i=0; i < COLNUM ; i++)
	{
		lcd.setCursor(i,row);
		lcd.print(" ");
	}
}

void lcd_ClearCursor(int row)
{
	lcd.setCursor(0,row);
	lcd.print(" ");
}

void lcd_DisplayMenu(Menu_e Menu, Menu_state_e menu_submenu_state)
{
	switch(Menu)
	{
		case OUT:
			Menu = AJUSTES;
		break;

		case AJUSTES:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,0);
		}
		break;

		case MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,1);
		}
		break;

		case ULT_MEDIDAS:
		{
			lcd_PrintCursor(menu_submenu_state,2,1,0);
		}
		break;

		case CFG_HELICES:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,0);
		}
		break;

		case CFG_PERIODO:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,1);
		}
		break;
		
		case REF_LUGAR:
		{
			lcd_PrintCursor(menu_submenu_state,2,2,0);
		}
		break;

		case CFG_DATE:
		{
			lcd_PrintCursor(menu_submenu_state,2,2,1);
		}
		break;

		case BUZZER:
		{
			lcd_PrintCursor(menu_submenu_state,4,2,0);
		}
		break;

		case ATRAS_AJUSTES:
		{
			lcd_PrintCursor(menu_submenu_state,4,2,1);
		}
		break;

		case INICIO_MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,0);
		}
		break;

		case ATRAS_MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,1);
		}
		break;

		case TOMAR_MEDICION:
		{
			// no hacer nada
		}
		break;

		default:
			Menu = AJUSTES;
		break;
	}
}

void lcd_PrintCursor(Menu_state_e menu_submenu_state, uint8_t start, uint8_t count, uint8_t cursorPosition)
{
	lcd.clear();
	lcd.setCursor(0,cursorPosition);
	lcd.print(">");
	bool cursor = 0;
	if (count <= ROWNUM){
		for (uint8_t i=start; i<count+start; i++)
		{
			lcd.setCursor(1,cursor);
			cursor = !cursor;

			if (menu_submenu_state == MAIN){
				lcd.print(menu[i]);
			}
			else if (menu_submenu_state == AJUSTES_SUBMENU){
				lcd.print(ajustes[i]);
			}
			else if (menu_submenu_state == MEDICION_SUBMENU){
				lcd.print(medicion[i]);
			}
		}
	}
}


void StateMachine_Control(Menu_e Menu, Menu_state_e menu_submenu_state)
{
	switch(Menu)
	{
		case TOMAR_MEDICION:
		{
			move_t buttonProcess = DONTMOVE;
			while(buttonProcess != ENTER)
			{
				lcd.clear();
				lcd.setCursor(0,0);
				lcd.print("Periodo = ");
				float periodo = GetEEPROMValue(PERIODO);
				lcd.setCursor(10,0);
				lcd.print(periodo);
				double lastmillis = millis();
				while((millis() - lastmillis) <= periodo || (CheckButton() != ENTER))
				{
					//hola
				}
				buttonProcess = ENTER;
			}
			estado_actual = INICIO_MEDICION;
		}
		break;
	}
}