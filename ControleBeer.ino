#include <EEPROM.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>

#include "Constantes.h";

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
MAX6675 termopar (thermoCLK, thermoCS, thermoDO);


#include "Alarme.h";
#include "ObserverInterfaces.h";
#include "AssuntoAbstrato.h";
#include "Timer.h";

#include "Teclado.h";

/************** Controlador *************************/
#include "OnOff.h";
#include "PWM.h"
/************** State Pattern ***********************/

// Arquivos relacionados ao padrao de design de Estados
#include "AppAbstract.h";
#include "EstadoAbstrato.h";

#include "EstadosConcretos.h";
#include "EstadoRampa.h";
#include "EstadoManual.h";
#include "EstadoFervura.h";
#include "EstadoConfiguraFervura.h";

#include "App.h";

App app = App(new Principal_Estado(&app), new Timer(), new Teclado(&app), new OnOff(Heat, histerese, atraso));

void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.backlight();
  app.setup();
}

void loop() {
    app.run();
}

void serialEvent() {
  String comando;
  String variavel;

  //Le os parametros de enrada
  if (Serial.available()) {
    // Le os comandos
    while (Serial.available() > 0) {
      comando = Serial.readStringUntil(':');
      Serial.print("Comando = ");
      Serial.println(comando);
      variavel = Serial.readStringUntil('\n');
      Serial.print("Variavel = ");
      Serial.println(variavel);

      /*****************************************************************/
      /*          Seleçao das etapas do processo                       */
      /*****************************************************************/
      //Comando Proxima Etapa
      if ( comando == "enter" ) {
        app.enter();
      }
      //Comando Proxima opcao
      if (comando == "+") {
        app.adiciona(1);
      }
      if (comando == "-") {
        app.subtrai(1);
      }
      //Atribuir valor
      if ( comando == "cancel" ) {
        app.cancel();
      }



    }
  }

}

