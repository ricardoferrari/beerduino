// Estado concreto do menu manual
/*
 *   Lista de etapas
 *   0    - Questiona utilização de tempo
 *   1    - Definindo tempo de brassagem
 *   2    - Definindo temperatura
 *   20   - Em execução (principal)
 *   21   - Em execução (decorrido)
 *   22   - Em execução (restante)
 *   100  - Timer finalizado
 */
 
class Refrigera_Estado: public EstadoAbstrato {
  public:
    Refrigera_Estado(AppAbstract *delegate, RecuperavelDelegate *_delegateRecuperavel, OnOffInterface *_controlador) {
      _delegate = delegate;
      controlador = _controlador;
      
      //Delegate esponsavel por atualizar o snapshot do estado no app
      setDelegateRestauravel(_delegateRecuperavel);
    }

    //Caso receba notificacao como observer
    void update(TimerInterface *assunto) {}

    //Caso receba notificacao de watchdog como observer
    void updateTela(TimerInterface *assunto) {
        tela_atualizada = false;
    }
    
    void run() {
      //Executa a ação do controlador
      this->controlador->run();

    
      if (!tela_atualizada) {
          lcd.clear();

          switch(etapa) {
            case 0:
              lcd.setCursor(0,0);
              lcd.print("SELECIONE       ");
              lcd.setCursor(0,1);
              lcd.print("Temperatura: ");
              lcd.print(valor);
              break;
            default:
              lcd.setCursor(0,0);
              lcd.print("EM EXECUCAO:    ");
              lcd.setCursor(0,1);
              lcd.print("PV:");
              lcd.print(controlador->getPV());
              lcd.setCursor(6,1);
              lcd.print("SP:");
              lcd.print(param[0]);
              /*** mostra saida rele *******/
              lcd.setCursor(13,1);
              lcd.print((controlador->estadoControlador())?"OFF":"ON");
              break;
          }
          
          tela_atualizada = true;
      }
    }

    void adiciona(int qtd) {
      if ((valor+qtd) <= valorMax) {
          valor+=qtd;
          if (etapa==20) { param[0] = valor; controlador->setSetPoint(valor); }//Modifica a temperatura durante o processo
          tela_atualizada = false;
      }
    }

    void subtrai(int qtd) {
      if ((valor-qtd) >= 0) {
          valor-=qtd;
          if (etapa>=20) { param[0] = valor; controlador->setSetPoint(valor); } //Modifica a temperatura durante o processo
          tela_atualizada = false;
      }
    }

    void enter() {
      switch(etapa) {
            case 0: {
                      param[0] = valor;
                      tela_atualizada = false;
                      this->iniciaExecucao(param);
                      //Captura o estado atual para restaurar em caso de perda de energia
                      this->takeSnapshot(Refrigeracao, param);
            }
            break;
            default:
            break;
      }

    }

    void cancel() {
      controlador->finalizaControlador();
      //Cancela a captura de estado para reinício após falha elétrica
      this->unsetSnapshot();
      _delegate->gotoEstado(Principal);
    }

    //Restauracao de energia
    void iniciaExecucao(byte *_param) {
      Serial.println("Inicia refri");
        //Copia valores para a instância do estado atual
        param[0] = _param[0];
        param[1] = 0;
        param[2] = 0;
        valor = _param[0];
        valorMax = 100; //Atribui limite para seleção de temperatura durante a execução

        //Atribui timer de atualizacao de tela
        _delegate->timer->setWatchDog(1);
        
        //Inicializa o setpoint e ativa o controle
        controlador->setResfriamento(true);
        controlador->inicializaControlador(valor);
        etapa=20; // Pula para as telas de execução
        tela_atualizada = false;
    }

    void initFromSnapshot(byte *_param) {
      iniciaExecucao(_param);
    }
    
  private:
    byte etapa = 0;
    byte valorMax = 100;
    byte valor = 20;
    byte param[3];
    AppAbstract *_delegate;
    bool tela_atualizada = false;
    OnOffInterface *controlador;
};

