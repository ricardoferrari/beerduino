/////////////////// Classe do controlador /////////////////
class OnOffDelegate {
  public:
    virtual boolean estadoControlador();
    virtual void inicializaControlador(double _setpoint);
    virtual void finalizaControlador();
    virtual double controladorPV();
};

class OnOff{

public:
  boolean liga = false;
  bool executando = false;
  unsigned long proxima_execucao;
  unsigned long atraso;
  int histerese;
  byte rele;
  double temperatura = 0;
  double temperatura_offset = 0;
  
  double setPoint;
  
  OnOff(int _rele, double _histerese, unsigned long _atraso){
   this->histerese = _histerese;
   this->rele = _rele;
   this->atraso = _atraso;
   this->proxima_execucao = millis() + _atraso;
   //Atribui o pino da saida de controle
   pinMode(this->rele, OUTPUT);
  }


  void ativa() {
    this->executando = true;
  }

  void desativa() {
    this->executando = false;
  }
  
  void adicionaAmostra(double _temperatura){
    temperatura = _temperatura;
  }

  double getPV() {
    return temperatura;
  }
  
  void setSetPoint(double _setPoint){
    setPoint = _setPoint;
  }
  
  double process(){
    // Implementação
    double error = this->temperatura - this->setPoint;

    //Habilita caso o atraso de processo já tenha sido atingido
    unsigned long agora = millis();
    bool habilitado = !((agora - this->proxima_execucao) & 0x80000000);
    if (habilitado) {
        if (error >= 0) {
          liga = true;
          digitalWrite(rele, LOW);
        }
        else if ((error+this->histerese) <= 0) {
          liga = false;
          digitalWrite(rele, HIGH);
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
  
};
