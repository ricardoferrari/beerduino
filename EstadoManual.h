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
 
class Manual_Estado: public EstadoAbstrato {
  public:
    Manual_Estado(AppAbstract *delegate, RecuperavelDelegate *_delegateRecuperavel, OnOffInterface *_controlador) {
      _delegate = delegate;
      controlador = _controlador;
      
      //Delegate esponsavel por atualizar o snapshot do estado no app
      setDelegateRestauravel(_delegateRecuperavel);
    }

    //Caso receba notificacao como observer
    void update(TimerInterface *assunto) {
      if (assunto->finalizou()) {     // Timer finalizado
        tela_atualizada = false;
        etapa = 100;
      }
    }

    //Caso receba notificacao de watchdog como observer
    void updateTela(TimerInterface *assunto) {
      if (executando) {
        tela_exec++;
        if (tela_request || (tela_exec>5)) { // Aguarda 5 pulsos do timer para mudar a tela ou pressionar enter
          tela_request = false;
          tela_exec = 0;
          etapa = (com_tempo) ? ((etapa-20+1)%3+20) : ((etapa-20+1)%2+20); //Próxima tela
        }
        tempo_decorrido = assunto->getElapsedFormatado();
        tempo_restante = assunto->getRemainingFormatado();
        Serial.print("FORMATADO UPDATE");
        Serial.println(tempo_decorrido);
        tela_atualizada = false;
      }
    }
    
    void run() {
      //Executa a ação do controlador
      this->controlador->run();

      //Checa se o tempo deve ser contabilizado (temperatura com limiar de erro menor do que 5 celsius)
      if (this->controlador->mashRangeOK(5)) {
        _delegate->timer->resume();
      } else {
        _delegate->timer->pausa();
      }
      
      if (!tela_atualizada) {
          lcd.clear();

          switch(etapa) {
            case 0:
              lcd.setCursor(0,0);
              lcd.print("Definir tempo?");
              lcd.setCursor(0,1);
              lcd.print((valor==0)?"Nao":"Sim");
              break;
            case 1:
              lcd.setCursor(0,0);
              lcd.print("SELECIONE       ");
              lcd.setCursor(0,1);
              lcd.print("Tempo: ");
              lcd.print(valor);
              break;
            case 2:
              lcd.setCursor(0,0);
              lcd.print("SELECIONE       ");
              lcd.setCursor(0,1);
              lcd.print("Temperatura: ");
              lcd.print(valor);
              break;
            case 20:
              lcd.setCursor(0,0);
              lcd.print("EM EXECUCAO:    ");
              lcd.setCursor(0,1);
              lcd.print("PV:");
              lcd.print(controlador->getPV());
              lcd.setCursor(6,1);
              lcd.print("SP:");
              lcd.print(param[1]);
              /*** mostra saida rele *******/
              lcd.setCursor(13,1);
              lcd.print((controlador->estadoControlador())?"OFF":"ON");
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
            default:
              lcd.setCursor(0,0);
              lcd.print("Concluido!!!    ");
              break;
          }
          
          tela_atualizada = true;
      }
    }

    void adiciona(int qtd) {
      if ((valor+qtd) <= valorMax) {
          valor+=qtd;
          if ((etapa>=20) && (etapa<=22)) { param[1] = valor; controlador->setSetPoint(valor); }//Modifica a temperatura durante o processo
          tela_atualizada = false;
          tela_exec = 0;
      }
    }

    void subtrai(int qtd) {
      if ((valor-qtd) >= 0) {
          valor-=qtd;
          if ((etapa>=20) && (etapa<=22)) { param[1] = valor; controlador->setSetPoint(valor); } //Modifica a temperatura durante o processo
          tela_atualizada = false;
          tela_exec = 0;
      }
    }

    void enter() {
      switch(etapa) {
            case 0: {
                      com_tempo = (valor==1);
                      //Se for sem tempo pula para a etapa 2
                      valorMax = com_tempo ? 200 : 100;
                      etapa = com_tempo ? 1 : 2;
                      valor = 60;
                      tela_atualizada = false;
            }
            break;
            case 1: {
                      param[0] = valor;
                      valor = 66;
                      valorMax = 100;
                      etapa++;
                      tela_atualizada = false;
            }
            break;
            case 2: {
                      param[1] = valor;
                      param[2] = com_tempo;
                      this->iniciaExecucao(param);
                      //Captura o estado atual para restaurar em caso de perda de energia
                      this->takeSnapshot(Manual, param);
            break;
            }
            default:
                      // Gera requisição para mudar para a proxima tela
                      tela_request = true;
            break;
      }

    }

    void cancel() {
      controlador->finalizaControlador();
      _delegate->timer->stop();
      //Cancela a captura de estado para reinício após falha elétrica
      this->unsetSnapshot();
      _delegate->gotoEstado(Principal);
    }

    //Restauracao de energia
    void iniciaExecucao(byte *_param) {
        //Copia valores para a instância do estado atual
        param[0] = _param[0];
        param[1] = _param[1];
        param[2] = _param[2];
        valor = _param[1];
        valorMax = 100; //Atribui limite para seleção de temperatura durante a execução
        com_tempo = (param[2]!=0);

        //Inicializa o controle e a temporização
        _delegate->timer->reseta();
        if (com_tempo) {
            _delegate->timer->setMinutoTotal(param[0]);
            _delegate->timer->start();
        } else {
          _delegate->timer->setWatchDog(1);
        }
        executando = true;
        //Inicializa o setpoint e ativa o controle
        controlador->setResfriamento(false);
        controlador->inicializaControlador(valor);
        etapa=20; // Pula para as telas de execução
        tela_atualizada = false;
    }

    void initFromSnapshot(byte *_param) {
      iniciaExecucao(_param);
    }
    
  private:
    byte etapa = 0;
    byte tela_exec = 0;
    bool tela_request = false;
    bool com_tempo = false;
    byte valorMax = 1;
    byte valor = 0;
    byte param[3];
    String tempo_decorrido;
    String tempo_restante;
    bool executando = false;
    AppAbstract *_delegate;
    bool tela_atualizada = false;
    OnOffInterface *controlador;
};

