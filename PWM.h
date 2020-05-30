/////////////////// Classe do controlador /////////////////
class PWMInterface {
  public:
    virtual void inicializaControlador(int _duty_cycle);
    virtual void finalizaControlador();
    virtual void setDuty(int _duty_cycle);
    virtual void run();
};

class PWM: public PWMInterface{

public:
  boolean liga = LOW;
  bool executando = false;
  unsigned long proxima_execucao;
  unsigned long frequencia;
  byte rele;
  int duty_cycle = 50;
  
  PWM(int _rele, unsigned long _frequencia){
   this->rele = _rele;
   this->frequencia = _frequencia;
   this->proxima_execucao = millis() + _frequencia;
   //Atribui o pino da saida de controle
   pinMode(this->rele, OUTPUT);
   digitalWrite(this->rele, HIGH);
  }


  void ativa() {
    this->executando = true;
  }

  void desativa() {
    this->executando = false;
  }

  void setDuty(int _duty_cycle){
    duty_cycle = _duty_cycle;
  }

  void inicializaControlador(int _duty_cycle) {
    this->setDuty(_duty_cycle);
    this->ativa();
  }

  void finalizaControlador() {
    this->desativa();
    digitalWrite(rele, HIGH);
  }

  double process(){

    //Habilita caso a frequencia de processo já tenha sido atingida
    unsigned long agora = millis();
    bool habilitado = !((agora - this->proxima_execucao) & 0x80000000);
    if (habilitado) {
        duty_count = (duty_count < duty_full) ? (duty_count + 1) : 0;
        liga = (duty_count < duty_cycle) ? LOW : HIGH;
        digitalWrite(rele, liga);
        //Insere atraso para reduzir chaveamento
        this->proxima_execucao = agora + this->frequencia;
    }
  }

  void run(){
    // Atribui o valor de controle a saída relê
    if (this->executando) { 
      process();
    }
  }

  private:
    int duty_full = 100;
    int duty_count = 0;
};
