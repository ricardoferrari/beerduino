/****************** Contexto do Estado  **************************************/
class App: public AppAbstract, public TecladoDelegate {
  public:
    byte param[21];
    
    App (EstadoAbstrato *estado, Timer *timer, Teclado *teclado, OnOff *controlador) {
      this->estado_atual = estado;
      this->timer = timer;
      this->teclado = teclado;
      this->controlador = controlador;
    }

    
    void setup() {
      this->teclado->setup();
    }

    void run() {
      this->estado_atual->run();
      this->teclado->run();
      this->timer->run();
      this->controlador->run();
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

    /********** Funcoes do Delegate do Controlador  *********************/
    boolean estadoControlador() {
      return this->controlador->estado();
    }

    void inicializaControlador(double _setpoint) {
      this->controlador->setSetPoint(_setpoint);
      this->controlador->ativa();
    }

    void finalizaControlador() {
      this->controlador->desativa();
    }

    double controladorPV(){
      return this->controlador->getPV();
    }

    /********** State Design Pattern  *********************/
    void gotoEstado(Nome_estados nome) {
      this->timer->detach(this->estado_atual);
      delete this->estado_atual;
      switch(nome) {
        case Manual:
          this->estado_atual = new Manual_Estado(this);
          break;
        case Automatico:
          this->estado_atual = new Automatico_Estado(this, &param[0]);
          break;
        case ConfiguraFervura:
          this->estado_atual = new ConfiguraFervura_Estado(this, &param[0]);
          break;
        case Fervura:
          this->estado_atual = new Fervura_Estado(this, &param[0]);
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
