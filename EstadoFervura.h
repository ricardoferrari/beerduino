// Estado concreto do modo automatico
class Fervura_Estado: public EstadoAbstrato {
  public:
    Fervura_Estado(AppAbstract *delegate, byte *_param, PWMInterface *_controlador) {
      _delegate = delegate;
      this->controlador = _controlador;
      param = &_param[0];
      //param[0] = tempo de fervura
      //param[1] = qtd lupulos
      if (param[1]>0) _delegate->timer->alarme->habilitaAlarme();
      _delegate->timer->reseta();
      _delegate->timer->setMinutoTotal(param[0]); 
      _delegate->timer->start();
      this->controlador->inicializaControlador(valorPWM);
      executando = true;
      etapa=20; // Pula para as telas de execução
      tela_atualizada = false;
    }

    //Restauracao de energia
    void initFromSnapshot(byte *_param) {}

    //Caso receba notificacao como observer
    void update(TimerInterface *timer) {
      if (timer->finalizou()) {     // Timer finalizado
        tela_atualizada = false;
        etapa = 201;
        this->controlador->finalizaControlador();
        _delegate->timer->stop();
      }
      else if (timer->getAlarme()->temAlarme()) {     // Tem alarme
        tela_atualizada = false;
        etapa_anterior = etapa;
        etapa = 100;
      }
    }

    //Caso receba notificacao de watchdog como observer
    void updateTela(TimerInterface *timer) {
      if (executando && !(timer->getAlarme()->temAlarme())) {
        tela_exec++;
        if (tela_exec > 5) { // Aguarda 4 pulsos do timer para mudar a tela
          tela_exec = 0;
          etapa = (etapa-20+1)%3+20;
        }
        tempo_decorrido = timer->getElapsedFormatado();
        tempo_restante = timer->getRemainingFormatado();
        Serial.print("FORMATADO UPDATE");
        Serial.println(tempo_decorrido);
        tela_atualizada = false;
      } else if (timer->getAlarme()->temAlarme()){
        tempo_decorrido = timer->getElapsedFormatado();
        tela_atualizada = false;
      }

    }
    
    void run() {
      /****************************************/
      /*******  Controlador de PWM  ***********/
      /****************************************/
      this->controlador->run();
      
      if (!tela_atualizada) {
          lcd.clear();

          switch(etapa) {
            /****************************************/
            /***  Telas durante a execucao **********/
            /****************************************/
            case 20:
              lcd.setCursor(0,0);
              lcd.print("EM EXECUCAO:    ");
              lcd.setCursor(0,1);
              lcd.print("PWM: ");
              lcd.print(valorPWM);
              lcd.print("%");
              break;
            case 21:
              lcd.setCursor(0,0);
              lcd.print("Decorrido:  ");
              lcd.setCursor(0,1);
              lcd.print(tempo_decorrido);
              break;
            case 22:
              lcd.setCursor(0,0);
              lcd.print("Restante:      ");
              lcd.setCursor(0,1);
              lcd.print(tempo_restante);
              break;
            /***** Tem alarme **********/
            case 100:
              lcd.setCursor(0,0);
              lcd.print(_delegate->timer->getAlarme()->getMensagem());
              lcd.setCursor(0,1);
              lcd.print("Tempo:");
              lcd.print(tempo_decorrido);
              break;
            case 200:
              lcd.setCursor(0,0);
              lcd.print("Finaliza?");
              lcd.setCursor(0,1);
              lcd.print("Ok/Cancel");
              break;
            default:
              lcd.setCursor(0,0);
              lcd.print("Concluido!!!");
              break;
          }
          tela_atualizada = true;
      }
      
    }

    void adiciona(int qtd) {
      if (valorPWM < valorMax) {
          valorPWM+=qtd;
          valorPWM = (valorPWM > valorMax) ? valorMax : valorPWM;
          this->controlador->setDuty(valorPWM);
          tela_exec = 0; // Zera o contador de loop de tela
          tela_atualizada = false;
      }
    }

    void subtrai(int qtd) {
      if (valorPWM > 0) {
          valorPWM-=qtd;
          valorPWM = (valorPWM < 0) ? 0 : valorPWM;
          this->controlador->setDuty(valorPWM);
          tela_exec = 0; // Zera o contador de loop de tela
          tela_atualizada = false;
      }
    }

    void enter() {
      switch(etapa) {
            /***********************************************************/
            /****************** Reconhece o alarme *********************/
            case 100:
                _delegate->timer->getAlarme()->reconheceAlarme();
                etapa=(_delegate->timer->finalizou()) ? 200 : 20; // Volta para a tela de execução ou fim de fervura
                tela_exec = 0; // Zera o contador de loop de tela
                tela_atualizada = false;
                break;
            /**********************************************************/
            /***************** Deseja sair do programa ****************/
            case 200:
                etapa++;
                tela_atualizada = false;
                this->controlador->finalizaControlador();
                _delegate->timer->stop();
                break;
            /**********************************************************/
            /***************** Finaliza o processo ********************/
            case 201:
                _delegate->gotoEstado(Principal);
                break;
            /***********************************************************/
            /***************** Solicita a loop de tela *****************/
            default:
                etapa = (etapa-20+1)%3+20;
                tela_exec = 0; // Zera o contador de loop de tela
                tela_atualizada = false;
                break;
      }
    }

    void cancel() {
      switch(etapa) {
            /***********************************************************/
            /****************** Reconhece o alarme *********************/
            case 100:
                _delegate->timer->getAlarme()->reconheceAlarme();
                etapa=(_delegate->timer->finalizou()) ? 200 : 20; // Volta para a tela de execução ou fim de fervura
                tela_exec = 0; // Zera o contador de loop de tela
                tela_atualizada = false;
                break;
            /***********************************************************/
            /****************** Não sai do processo ********************/
            case 200:
                etapa = 20;
                executando = true;
                break;
            /***********************************************************/
            /****************** Sai do processo     ********************/
            case 201:
                _delegate->gotoEstado(Principal);
                break;
            /***********************************************************/
            /****************** Cancela processo  **********************/
            default:
                etapa=200;
                executando = false;
                break;
      }
      tela_atualizada = false;
    }

    
  private:
    byte etapa = 20;
    byte etapa_anterior = 0; //Utilizado pelo alarme
    byte tela_exec = 0;
    byte valorMax = 100;
    byte valorPWM = 90;
    String tempo_decorrido;
    String tempo_restante;
    Alarme alarme;
    bool executando = false;
    byte *param; //Num maximo de pontos da rampa
    PWMInterface *controlador;
    AppAbstract *_delegate;
    bool tela_atualizada = false;
};
