class Timer: public AssuntoAbstrato {
public:
  
  Alarme *alarme;
  long tempo_inicial = 0;
  long tempo_total = 0;
  long tempo_pausa = 0;
  

  Timer() {
    this->alarme = new Alarme();
  }

  void attach(ObserverAbstrato *observer) {
//    int i=0;
//    while (this->observer[i]!= NULL) { i++; }
//    this->observer[i] = observer;
      this->observer = observer;
  }
  
  void detach(ObserverAbstrato *observer){
//    int i=0;
//    while (this->observer[i] != observer) { i++; }
//    this->observer[i] = NULL;
      this->observer = NULL;
  }
  
  void notify(){
//    for (int i=0; i++; i<2) {
//      this->observer[i]->update(this);
//    }
      this->observer->update(this);
  }

  void notifyTela(){
//    for (int i=0; i++; i<2) {
//      this->observer[i]->update(this);
//    }
      this->observer->updateTela(this);
  }

  Alarme *getAlarme() {
    return this->alarme;
  }
  
  bool finalizou() {
    return concluido;
  }

  long getElapsed(){
    unsigned long tempo_calc = 0;
    if(pausado && (iniciado || watchdog)) {
      tempo_calc = tempo_inicial-tempo_pausa;
    } else if (iniciado || watchdog){
      tempo_calc = millis()-tempo_inicial;
    }
    Serial.print("Tempo decorrido (timer): ");
    Serial.println(tempo_calc);
    return tempo_calc;
  }

  String getElapsedFormatado(){
    return this->formataTempo(this->getElapsed());
  }

  long getRemaining(){
     //return iniciado ? max(tempo_total-this->getElapsed(),0) : 0;
     return max(tempo_total-this->getElapsed(),0);
  }

  String getRemainingFormatado(){
    return this->formataTempo(this->getRemaining());
  }

  String getTempoTotal(){
    return this->formataTempo(tempo_total);
  }

  long getPausado(){
     return pausado ?(millis()-tempo_pausa) : 0;
  }

  String getPausadoFormatado(){
    return this->formataTempo(this->getPausado());
  }


  String formataTempo(long tempo){
    int horas = tempo / hora;
    int minutos = (tempo % hora) / minuto;
    int segundos = ((tempo % hora) % minuto) / segundo;
    
    char buffer[50];
    sprintf(buffer, "%dh%dm%ds",horas,minutos,segundos);

    return buffer;
  }

  void run() {
    if (iniciado && !pausado){
        // Finalizou o tempo
        if (this->getRemaining() <= 0) {
            concluido = true;
            iniciado = false;
            this->resetWatchDog();
            this->notify();
        }
        //Notifica presenca de alarme
        if (   this->alarme->checaAlarme( int(this->getElapsed()/1000/60) )   ) this->notify();
        
    } 
    if (watchdog && ((millis()-ultimo_pulso) > delay_pulso)) {
      this->notifyTela();
      ultimo_pulso = millis();
    }
  }

  void start(){
    this->reseta();
    this->setWatchDog(1);
    iniciado = true;
    concluido = false;
  }

  void setMinutoTotal(int n){
    tempo_total = n*minuto;
  }

  void stop() {
    iniciado = false;
    resetWatchDog();
    this->alarme->limpaAlarme();
  }

  void pausa() {
    //pausa somente se o timer estiver ativado
    if (!pausado && (iniciado || watchdog)) {
      tempo_pausa = millis();
      pausado = true;
    }
  }

  void resume() {
    if(pausado && (iniciado || watchdog)) {
      tempo_inicial += (millis()-tempo_pausa);
      pausado = false;
    }
  }

  boolean isPaused() {
    return this->pausado;
  }
  
  void reseta(){
    tempo_inicial = millis();
    tempo_pausa = tempo_inicial;
  }

  void setWatchDog(long _delay_pulso) {
    delay_pulso = _delay_pulso*1000;
    watchdog = true;
    ultimo_pulso = millis();
  }

  void resetWatchDog() {
    watchdog = false;
  }

  private:
   //ObserverAbstrato *observer[2];
   ObserverAbstrato *observer;
   unsigned long ultimo_pulso = 0;
   int delay_pulso = 1;
   const long hora = 3600000; // 3600000 por hora
   const long minuto = 60000; // 60000 por minuto
   const long segundo =  1000; // 1000 por segundo
   bool concluido = false;
   bool iniciado = false;
   bool pausado = false;
   bool watchdog = false;
};
