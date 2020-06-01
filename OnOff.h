/////////////////// Classe do controlador /////////////////
class OnOffInterface {
  public:
    virtual boolean estadoControlador();
    virtual boolean mashRangeOK(byte limiar);
    virtual void inicializaControlador(byte _setpoint);
    virtual void finalizaControlador();
    virtual double getPV();
    virtual void setSetPoint(byte _setpoint);
    virtual void setResfriamento(boolean yes);
    virtual void run();
};

class OnOff: public OnOffInterface{

public:
  boolean liga = false;
  bool executando = false;
  unsigned long proxima_execucao;
  unsigned long atraso;
  int histerese;
  byte rele;
  
  double setPoint;
  
  OnOff(int _rele, double _histerese, unsigned long _atraso){
   this->histerese = _histerese;
   this->rele = _rele;
   this->atraso = _atraso;
   //Atribui o pino da saida de controle
   pinMode(this->rele, OUTPUT);
   digitalWrite(this->rele, HIGH);
  }

  /********** Funcoes do Delegate do Controlador  *********************/
    boolean estadoControlador() {
      return this->estado();
    }

    boolean mashRangeOK(byte limiar) {
      return (abs(getError()) < limiar);
    }
    
    void inicializaControlador(byte _setpoint) {
      this->proxima_execucao = millis() + this->atraso;
      this->setSetPoint(_setpoint);
      this->ativa();
    }

    void finalizaControlador() {
      this->desativa();
    }

    void setResfriamento(boolean yes) {
      acao_direta = !yes;
    }

    double getPV() {
      return temperatura;
    }
    /************************************************************/


  void ativa() {
    this->executando = true;
  }

  void desativa() {
    this->executando = false;
  }
  
  void adicionaAmostra(double _temperatura){
    temperatura = _temperatura;
  }

  void setSetPoint(byte _setpoint){
    setPoint = _setpoint;
  }

  double getError() {
    return this->temperatura - this->setPoint;
  }
  
  double process(){
    // Implementação
    double error = getError();

    //Habilita caso o atraso de processo já tenha sido atingido
    unsigned long agora = millis();
    bool habilitado = !((agora - this->proxima_execucao) & 0x80000000);
    if (habilitado) {
        if (error >= 0) {
          liga = (acao_direta) ? true : false;
          digitalWrite(rele, liga);
        }
        else if ((error+this->histerese) <= 0) {
          liga = (acao_direta) ? false : true;
          digitalWrite(rele, liga);
        }
        //Insere atraso para reduzir chaveamento
        this->proxima_execucao = agora + this->atraso;
    }
  }

  boolean estado() {
    return liga;
  }

  void run(){
    // Atribui o valor de controle a saída relê
    //Primeiro processa o estado em relação ao setpoint
    
    this->adicionaAmostra(termopar.readCelsius()+temperatura_offset);
    
    if (this->executando) { 
      process();
    }
  }

  private:
    boolean acao_direta = true; //Define o modo de operação como direto ou inverso
    double temperatura = 0;
    double temperatura_offset = 0;
};
