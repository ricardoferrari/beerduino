// Parametros utilizados:
// 40 está preparado para recuperar
// 41 etapa
// 42..44 parametros (tempo,temperatura,etc)
class PersistenteObserver {
  public:
    virtual void notify(Nome_estados estado, byte *param);
};

//Delegate do app a ser recuperado em caso de queda de energia
class RecuperavelDelegate{
  public:
    virtual void restoreState(Nome_estados estado, byte *param);
    void updateState(Nome_estados estado, byte * params) {
      persistente->notify(estado, params);
    };
    void attach(PersistenteObserver *_persistente) {
      this->persistente = _persistente;
    }

  private:
    PersistenteObserver *persistente;
};



/******* Classe que recupera o estado anterior do estado concreto ******/
class Restauravel{
  public:
    void setDelegateRestauravel(RecuperavelDelegate *_delegateRecuperavel) {
      delegateRecuperavel = _delegateRecuperavel;
    }
    virtual void initFromSnapshot(byte *_param); 
    void takeSnapshot(Nome_estados estado, byte *_param) {
      delegateRecuperavel->updateState(estado, _param);
    }
    void unsetSnapshot() {
      delegateRecuperavel->updateState(Principal, NULL);
    }
    RecuperavelDelegate *delegateRecuperavel;
};



// Estado que mantem a última configuração após queda de energia
class Persistente: public PersistenteObserver {
  public:
    Persistente(RecuperavelDelegate *_delegate) {
      this->delegate = _delegate;
    }

    //Gatilho para restaurar estado anterior
    void triggerRestoration() {
      if (this->isSecured()) {
        this->recupera();
        Serial.print("Etapa: ");
        Serial.println(this->etapa);
        Serial.print("P0: ");
        Serial.println(this->param[0]);
        this->delegate->restoreState(this->etapa, &this->param[0]);
      }      
    }

    //Notificação para salvar estado atual
    void notify(Nome_estados estado, byte *_param) {
      Serial.print("Param(salva): ");
      Serial.print(_param[0]);
      Serial.print(" - ");
      Serial.print(_param[1]);
      Serial.print(" - ");
      Serial.print(_param[2]);
      if (estado == 0) {//Caso esteja no menu Principal
        unsetRecovery();
      } else {
        salva(estado, _param);
      }
    };

    boolean isSecured() {
      byte secured = EEPROM.read(40);
      return (secured == 0) ? false : true;
    }

    //Marca para recuperação na próxima inicialização
    void setRecovery() {
      EEPROM.write(40, 1);
    }

    //Desmarca para recuperação na próxima inicialização
    void unsetRecovery() {
      EEPROM.write(40, 0);
    }
    
    void salva(byte etapa, byte *param){
      if (EEPROM.length() > 44) { //tamanho da gravacao
        this->setRecovery();
        EEPROM.write(41, etapa);
        EEPROM.write(42, param[0]);
        EEPROM.write(43, param[1]);
        EEPROM.write(44, param[2]);
      }
    }

    void recupera(){
      //{Principal=0, Manual=1, Automatico=2, Fervura=3, ConfiguraFervura=4, Refrigeracao=5, Configuracao=6};
      if (EEPROM.length() > 44) { //tamanho da gravacao
        byte etapa_temp = EEPROM.read(41);
        switch(etapa_temp) {
          case 0 : this->etapa = Principal; break;
          case 1 : this->etapa = Manual; break;
          case 2 : this->etapa = Automatico; break;
          case 3 : this->etapa = Fervura; break;
          case 4 : this->etapa = ConfiguraFervura; break;
          case 5 : this->etapa = Refrigeracao; break;
          case 6 : this->etapa = Configuracao; break;
          default: this->etapa = Principal; break;
        }
        this->param[0] = EEPROM.read(42); // tempo
        this->param[1] = EEPROM.read(43); // temperatura
        this->param[2] = EEPROM.read(44); // com_tempo
      }
    }

  private:
    Nome_estados etapa; //etapa salva
    byte param[3]; // [0]tempo [1] temperatura salva [2] com_tempo
    RecuperavelDelegate *delegate;
};

