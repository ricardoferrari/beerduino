// Estado concreto do modo automatico
class ConfiguraFervura_Estado: public EstadoAbstrato {
  public:
    ConfiguraFervura_Estado(AppAbstract *delegate, byte *_param) {
      _delegate = delegate;
      param = &_param[0];
    }


    //Caso receba notificacao como observer
    void update(TimerInterface *assunto) {};

    //Caso receba notificacao de watchdog como observer
    void updateTela(TimerInterface *assunto) {};
    
    void run() {
      if (!tela_atualizada) {
          lcd.clear();

          switch(etapa) {
            case 0:
              lcd.setCursor(0,0);
              lcd.print("SELECIONE       ");
              lcd.setCursor(0,1);
              lcd.print("Tempo: ");
              lcd.print(valor);
              break;
            /****************************************/
            /************ Qtd lupulo ****************/
            /****************************************/
            case 1:
              lcd.setCursor(0,0);
              lcd.print("QTD. AD. LUPULO ");
              lcd.setCursor(0,1);
              lcd.print("Qtd: ");
              lcd.print(valor);
              break;
            /****************************************/
            /************ Tempo lupulo ****************/
            /****************************************/
            case 2:
            case 3:
            case 4:
              lcd.setCursor(0,0);
              char buffer[16];
              sprintf(buffer, "Add Lupulo %d",etapa-1);
              lcd.print(buffer);
              lcd.setCursor(0,1);
              lcd.print("Tempo: ");
              lcd.print(valor);
              break;
            default:
              lcd.setCursor(0,0);
              lcd.print("INICIA FERVURA?");
              lcd.setCursor(0,1);
              lcd.print("OK/CANCEL");
              break;
          }
          tela_atualizada = true;
      }
      
    }

    void adiciona(int qtd) {
      if (valor < valorMax) {
          valor+=qtd;
          valor = (valor > valorMax) ? valorMax : valor;
          tela_atualizada = false;
      }
    }

    void subtrai(int qtd) {
      if (valor > 0) {
          valor-=qtd;
          valor = (valor < 0) ? 0 : valor;
          tela_atualizada = false;
      }
    }

    void enter() {
      switch(etapa) {
            /***********************************************************/
            /***********************************************************/
            case 0: 
                param[0] = valor; //Tempo de fervura
                valor = 0;
                valorMax = 3;
                etapa++;
                tela_atualizada = false;
                break;
            /****************************************/
            /************ Qtd lupulo ****************/
            case 1: 
            {
                param[1] = valor; //Qtd de lupulos
                _delegate->timer->getAlarme()->setQtdAlarme(param[1]);
                //Prepara próxima tela
                valorMax = param[0];
                valor = floor(param[0]/2);
                etapa = (param[1]>0)? (etapa+1) : 10;
                tela_atualizada = false;
            }
            break;
            /****************************************/
            /*********** Momento da adicao **********/
            case 2 ... 4:
            {
                char buffer[16];
                sprintf(buffer, "Add Lupulo %d",etapa-1);
                _delegate->timer->getAlarme()->addAlarme(valor,buffer);
                etapa = (param[1]>(etapa-1))? (etapa+1) : 10;
                tela_atualizada = false;
            }
                break;
            /***********************************************************/
            /*************** Inicia o processo *************************/
            default:
                _delegate->gotoEstado(Fervura);
                break;
      }
    }

    void cancel() {
      _delegate->gotoEstado(Principal);
    }
    
  private:
    byte etapa = 0;
    byte valorMax = 100;
    byte valor = 60;
    byte *param; //Parametros de processo
};

