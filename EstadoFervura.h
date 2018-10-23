// Estado concreto do modo automatico
class Fervura_Estado: public EstadoAbstrato {
  public:
    Fervura_Estado(AppAbstract *delegate, byte *_param) {
      _delegate = delegate;
      param = &_param[0];
      //param[0] = tempo de fervura
      //param[1] = qtd lupulos
      if (param[1]>0) _delegate->timer->habilitaAlarme();
      _delegate->timer->setMinutoTotal(param[0]); 
      _delegate->timer->start();
      executando = true;
      etapa=20; // Pula para as telas de execução
      tela_atualizada = false;
    }

    //Caso receba notificacao como observer
    void update(AssuntoI *assunto) {
      if (assunto->finalizou()) {     // Timer finalizado
        tela_atualizada = false;
        etapa = 201;
      }
      if (assunto->temAlarme()) {     // Tem alarme
        tela_atualizada = false;
        etapa_anterior = etapa;
        etapa = 100;
      }
    }

    //Caso receba notificacao de watchdog como observer
    void updateTela(AssuntoI *assunto) {
      
      if (executando && !(assunto->temAlarme())) {
        tela_exec++;
        if (tela_exec > 5) { // Aguarda 4 pulsos do timer para mudar a tela
          tela_exec = 0;
          etapa = (etapa-20+1)%3+20;
        }
        tempo_decorrido = assunto->getElapsedFormatado();
        tempo_restante = assunto->getRemainingFormatado();
        tela_atualizada = false;
      } else if (assunto->temAlarme()){
        tempo_decorrido = assunto->getElapsedFormatado();
        tela_atualizada = false;
      }

    }
    
    void run() {
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
              lcd.print(valor);
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
              lcd.print(_delegate->timer->getMensagem());
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
            /****************** Reconhece o alarme *********************/
            case 100:
                _delegate->timer->reconheceAlarme();
                etapa=(_delegate->timer->finalizou()) ? 200 : 20; // Volta para a tela de execução ou fim de fervura
                tela_exec = 0; // Zera o contador de loop de tela
                tela_atualizada = false;
                break;
            /**********************************************************/
            /***************** Deseja sair do programa ****************/
            case 200:
                etapa++;
                tela_atualizada = false;
                break;
            /**********************************************************/
            /***************** Finaliza o processo ********************/
            case 201:
                _delegate->timer->stop();
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
      if (etapa==200) {
          etapa = 20;
          executando = true;
      } else if (etapa==201) {
          _delegate->timer->stop();
          _delegate->gotoEstado(Principal);
      } else {
          etapa=200;
          executando = false;
      }
      tela_atualizada = false;
    }
    
  private:
    byte etapa = 20;
    byte etapa_anterior = 0; //Utilizado pelo alarme
    byte tela_exec = 0;
    byte valorMax = 100;
    byte valor = 60;
    String tempo_decorrido;
    String tempo_restante;
    bool executando = false;
    byte *param; //Num maximo de pontos da rampa
};
