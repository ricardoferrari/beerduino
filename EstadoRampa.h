// Estado concreto do modo automatico
class Automatico_Estado: public EstadoAbstrato {
  public:
    Automatico_Estado(AppAbstract *delegate, byte *_param, OnOffInterface *_controlador) {
      param = _param;
      recupera();
      etapa = (param[0]==0)?200:0;
      _delegate = delegate;
      controlador = _controlador;
    }

    //Restauracao de energia
    void initFromSnapshot(byte *_param) {}

    void update(TimerInterface *assunto) {
      if (assunto->finalizou()) {     // Timer finalizado
        rampa+=2;
        if (rampa>param[0]*2) {
          //Ativa o buzzer ao finalizar
          this->alarme = assunto->getAlarme();
          this->alarme->ligaBuzzer();
          etapa = 103;
          executando = false;
        } else {                    // Continua executando a proxima rampa
          _delegate->timer->setMinutoTotal(param[rampa+1]);
          _delegate->timer->start();
          etapa = 10;
        }
        tela_atualizada = false;
      }
    }
    
    
    //Caso receba notificacao de watchdog como observer
    void updateTela(TimerInterface *assunto) {
      
      if ((etapa>=10) && (etapa<=12)) {
        tela_exec++;
        if (tela_exec > 5) { // Aguarda 4 pulsos do timer para mudar a tela
          tela_exec = 0;
          etapa = (etapa-10+1)%3+10;
        }
        strcpy(tempo_decorrido, assunto->getElapsedFormatado());
        strcpy(tempo_restante, assunto->getRemainingFormatado());
        //Atualiza a temperatura de acordo com o tempo na rampa ou patamar
        Serial.print("assunto->getPercentualCompleto()");
        Serial.println(assunto->getPercentualCompleto());
        this->temperatura_atual = this->calculaTemperaturaAtual(assunto->getPercentualCompleto(), param[rampa], param[rampa+2]);
        this->controlador->setSetPoint(temperatura_atual);
        tela_atualizada = false;
      }

      if(etapa == 101){
//        tempo_pausado = assunto->getPausadoFormatado();
        strcpy(tempo_pausado, assunto->getPausadoFormatado());
        tela_atualizada = false;
      }

    }

    int calculaTemperaturaAtual(int percentual, byte temperatura1, byte temperatura2) {
      return floor( (temperatura1*(100-percentual)+temperatura2*(percentual))/100 );
    }
    
    void run() {
      //Executa a ação do controlador
      this->controlador->run();

      //Checa se o tempo deve ser contabilizado (temperatura com limiar de erro menor do que 5 celsius)
      if ( (etapa >= 10) && (etapa <= 12) ) { //Caso esteja em modo de pausa forçado não executa a checagem de tempo
        if (this->controlador->mashRangeOK(5)) {
          _delegate->timer->resume();
          Serial.println("Resume");
        } else {
          _delegate->timer->pausa();
          Serial.println("Pausa");
        }
      }
      
      if (!tela_atualizada) {
          lcd.clear();

          switch(etapa) {
            case 0:
              lcd.setCursor(0,0);
              lcd.print("INICIA PROGRAMA?");
              lcd.setCursor(0,1);
              lcd.print("OK/CANCEL");
              break;
            case 10:
              lcd.setCursor(0,0);
              lcd.print("Rampa: ");
              lcd.setCursor(8,0);
              lcd.print(    int( floor(rampa/2) + 1 )    );
              lcd.print("/");
              lcd.print(    int( param[0] )    );
              lcd.setCursor(13,0);
              lcd.print((controlador->estadoControlador())?"OFF":"ON");
              lcd.setCursor(0,1);
              lcd.print(linha(controlador->getPV(), this->temperatura_atual, param[rampa+1]));
              break;
            case 11:
              lcd.setCursor(0,0);
              lcd.print("Deco. rampa ");
              lcd.print(    int( floor(rampa/2)+1 )    );
              lcd.setCursor(0,1);
              lcd.print(tempo_decorrido);
              break;
            case 12:
              lcd.setCursor(0,0);
              lcd.print("Rest. rampa ");
              lcd.print(    int( floor(rampa/2)+1 )    );
              lcd.setCursor(0,1);
              lcd.print(tempo_restante);
              break;
            case 100:
              lcd.setCursor(0,0);
              lcd.print("Pular rampa: ");
              lcd.setCursor(0,1);
              lcd.print("Sim/Nao");
              break;
            case 101:
              lcd.setCursor(0,0);
              lcd.print("Pausado: (Ok)");
              lcd.setCursor(0,1);
              lcd.print(tempo_pausado);
              break;
            case 102:
              lcd.setCursor(0,0);
              lcd.print("Cancelar Prog?");
              lcd.setCursor(0,1);
              lcd.print("(Ok/Cancel)");
              break;
            case 103:
              lcd.setCursor(0,0);
              lcd.print("Fim do prog.");
              break;
            default:
              lcd.setCursor(0,0);
              lcd.print("Nao ha programa!");
              break;
          }
          
          tela_atualizada = true;
      }
    }

    void adiciona(int qtd) {
      if ((qtd<=5) && ((etapa >= 10) && (etapa <= 12))) { // Está executando as rampas entao pula etapa
        etapa = 100;
        tela_atualizada = false;
      } else if ((qtd>1) && (etapa == 100)){ // Se manter pressionado pausa o processo
        _delegate->timer->pausa();
        etapa = 101;
        tela_atualizada = false;
      }
    }
    
    void subtrai(int qtd) {
    }

    void enter() {
      switch(etapa) {
              //Confirma inicio
              case 0:
                _delegate->timer->setMinutoTotal(param[rampa+1]);
                _delegate->timer->start();
                temperatura_atual = param[1];
                //Inicializa o setpoint e ativa o controle
                controlador->setResfriamento(false);
                controlador->inicializaControlador(temperatura_atual);
                executando = true;
                etapa=10;
                tela_atualizada = false;
                break;
                
              // Alterna telas
              case 10 ... 12:
                etapa = (etapa-10+1)%3+10;
                tela_exec = 0; // Zera o contador de pulsos para mudar de tela
                tela_atualizada = false;
                break;

                
              //Pula para proxima rampa
              case 100:
                rampa+=2;
                _delegate->timer->reseta();
                if (rampa>param[0]*2) {   // ******** Se for ultima rampa finaliza
                  etapa = 103;
                  executando = false;
                } else {                 // ********* Ainda existem rampas
                  etapa = 10;
                  _delegate->timer->setMinutoTotal(param[rampa+1]);
                  _delegate->timer->start();
                }
                tela_atualizada = false;
                break;
                

              //Pausa no processo
              case 101:
                _delegate->timer->resume();
                etapa = 10;
                tela_atualizada = false;
                break;
              //Cancela processo
              case 102:
                etapa = 103;
                tela_atualizada = false;
                break;
                
              default:
                cancel();
                break;
      }
    }

    void cancel() {
      if (etapa == 100) { //Cancela pula rampa
        etapa = 10;
        tela_atualizada = false;
      } else if (etapa == 101) { //Cancela pausa
        _delegate->timer->resume();
        etapa = 10;
        tela_atualizada = false;
      } else if (etapa == 102) { // Cancela saida
        etapa = 10;
        tela_atualizada = false;
      } else if (etapa == 103) { // Confirma saida
        //Desliga o buzzer
        this->alarme->desligaBuzzer();
        //Finaliza o controlador
        controlador->finalizaControlador();
        _delegate->gotoEstado(Principal);
      } else if (etapa == 200) { // Saida por ausencia de programa
        _delegate->gotoEstado(Principal);
      } else { //Cancela execução normal
        etapa = 102;
        tela_atualizada = false;
      }
    }

    void recupera(){
      int tamanho = min(EEPROM.read(0)*2,20); //tamanho da gravacao
      for (int i = 0 ; i <= tamanho ; i++) {
        param[i] = EEPROM.read(i);
      }
    }

    //Formata a linha para ser mostrada na tela
    char* linha(byte _PV, byte _SP, byte _tempo) {
      snprintf(this->buffer, 15, "SP%02d PV%02d T%03d ", _SP, _PV, _tempo);
      return this->buffer;
    }
    
  private:

    byte etapa = 0;
    byte rampaMax = 10;
    byte rampa = 1;
    byte *param; //Num maximo de pontos da rampa
    bool executando = false;
    byte tela_exec = 0;
    char tempo_pausado[12];
    char tempo_decorrido[12];
    char tempo_restante[12];
    byte temperatura_atual = 0;
    AppAbstract *_delegate;
    bool tela_atualizada = false;
    OnOffInterface *controlador;
    char buffer[16];
    Alarme *alarme;
};





// Estado concreto do menu configuração
class Configuracao_Estado: public EstadoAbstrato {
  public:
    Configuracao_Estado(AppAbstract *delegate, byte *_param) {
      param = &_param[0];
      _delegate = delegate;
      param[0] = 0; //Num de pontos da rampa inicial
    }

    //Restauracao de energia
    void initFromSnapshot(byte *_param) {}

    void update(TimerInterface *assunto) {}
    void updateTela(TimerInterface *assunto) { if (etapa == 101) etapa = 102; tela_atualizada=false;} // Aguarda 2s com a mensagem na tela
    
    void run() {
      if (!tela_atualizada) {
          lcd.clear();

          switch(etapa) {
            case 0:
              lcd.setCursor(0,0);
              lcd.print("Definir rampas? ");
              lcd.setCursor(0,1);
              lcd.print("Enter/Cancela");
              break;
            case 1:
              lcd.setCursor(0,0);
              lcd.print("Qtd de pts:     ");
              lcd.setCursor(0,1);
              lcd.print(valor);
              break;
            case 100:
              lcd.setCursor(0,0);
              lcd.print("SALVAR DADOS?   ");
              lcd.setCursor(0,1);
              lcd.print("OK/CANCEL");
              break;
            case 101:
              lcd.setCursor(0,0);
              lcd.print("Gravando!");
              break;
            case 102:
              lcd.setCursor(0,0);
              lcd.print("Prog. Gravado!");
              break;
            default:
              lcd.setCursor(0,0);
              if ((etapa%2)==1) {
                  lcd.print("Temperatura_");
                  lcd.print(int(floor((etapa-10)/2)));
              } else {
                  lcd.print("Tempo_");
                  lcd.print(int(floor((etapa-10)/2)));
              }
              lcd.setCursor(0,1);
              lcd.print(valor);
              break;
          }
          
          tela_atualizada = true;
      }
    }

    void adiciona(int qtd) {
      if ((valor+qtd) <= valorMax) {
          valor+=qtd;
          valor = (valor > valorMax) ? valorMax : valor;
          tela_atualizada = false;
      }
    }

    void subtrai(int qtd) {
      if ((valor-qtd) >= 0) {
          valor-=qtd;
          valor = (valor < 0) ? 0 : valor;
          tela_atualizada = false;
      }
    }

    void enter() {
      if (etapa == 102) { // Confirma o fim da configuração
        _delegate->timer->resetWatchDog();
        _delegate->gotoEstado(Principal);
      } else {
                switch(etapa) {
                  case 0: //Prompt inicia programacao
                    etapa++;
                    recupera(); //Busca dados da EPROM
                    valor = param[0];
                    break;
                  case 1: //Atribui quantidade de pontos
                    param[0] = valor;
                    if (valor == 0) {
                      etapa = 100;
                    } else {
                      etapa = 11;
                      valor = param[1];
                      valorMax = 100; //Max para temperatura
                    }
                    break;
                  case 100: //Confirma
                    etapa=101;
                    salva(); //Salva os dados na EPROM
                    _delegate->timer->setWatchDog(2);
                    break;
                  case 101: //Mostrando mensagem e aguradando o watchdog
                    break;
                  /******************* Define valores de tempo e temperatura ********************/
                  default:
                    /*********************** Alterna entre tempo e temperatura ***********************/
                    param[etapa-10] = valor;
                    etapa++;
                    valorMax = (etapa%2)?100:200; //Max para tempo e temperatura
                    valor = param[etapa-10]; //Parametros previamente gravados
                    /*********************** Solicita a confirmação da gravacao dos dados********************/
                    if ( etapa > (param[0]*2+11) ){
                      etapa=100;
                    } 
                    break;
                }
                
                tela_atualizada = false;
      }
    }

    void cancel() {
      _delegate->gotoEstado(Principal);
    }

    void salva(){
      int tamanho = min(EEPROM.length(), param[0]*2+1); //tamanho da gravacao
      for (int i = 0 ; i < tamanho ; i++) {
        EEPROM.write(i, param[i]);
      }
    }

    void recupera(){
      int tamanho = min(EEPROM.read(0)*2,20); //tamanho da gravacao
      for (int i = 0 ; i <= tamanho ; i++) {
        param[i] = EEPROM.read(i);
      }
    }
    
  private:
    byte etapa = 0;
    byte valorMax = 5; //qtd máxima inicial de rampas
    byte valor = 0;
    byte *param; //Num maximo de pontos da rampa
    AppAbstract *_delegate;
    bool tela_atualizada = false;
};
