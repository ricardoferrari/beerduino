/****************** Contexto do Estado  **************************************/
class App: public AppAbstract, public TecladoDelegate, public RecuperavelDelegate {
  public:
    byte param[21];
    
    App (EstadoAbstrato *estado, Timer *timer, Teclado *teclado) {
      this->estado_atual = estado;
      this->timer = timer;
      this->teclado = teclado;
    }

    
    void setup() {
      this->teclado->setup();
    }

    void run() {
      this->estado_atual->run();
      this->teclado->run();
      this->timer->run();
    }

    /********** Funcoes do Delegate do teclado  *********************/
    void adiciona(int qtd) {
      this->estado_atual->adiciona(qtd);
    }

    void subtrai(int qtd) {
      this->estado_atual->subtrai(qtd);
    }

    void enter() {
      this->estado_atual->enter();
    }

    void cancel() {
      this->estado_atual->cancel();
    }

    /*********** Funcao de recuperacao **********************************/
    void restoreState(Nome_estados estado, byte *param) {
      this->gotoEstado(estado);
      this->estado_atual->initFromSnapshot(param);
    }



    /********** State Design Pattern  *********************/
    void gotoEstado(Nome_estados nome) {
      this->timer->detach(this->estado_atual);
      delete this->estado_atual;
      switch(nome) {
        case Manual:
          this->estado_atual = new Manual_Estado(this, this, new OnOff(Heat, histerese, atraso));
          break;
        case Automatico:
          this->estado_atual = new Automatico_Estado(this, &param[0]);
          break;
        case ConfiguraFervura:
          this->estado_atual = new ConfiguraFervura_Estado(this, &param[0]);
          break;
        case Fervura:
          this->estado_atual = new Fervura_Estado(this, &param[0], new PWM(Heat, frequencia));
          break;
        case Configuracao:
          this->estado_atual = new Configuracao_Estado(this, &param[0]);
          break;
        default:
          this->estado_atual = new Principal_Estado(this);
          break;
      }
      this->timer->attach(this->estado_atual);
      
    }

  private:
    EstadoAbstrato *estado_atual;

};
