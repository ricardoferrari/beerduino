/****** Interfaces necessarias para a atualizacao apos o assunto notificar o observer *******/

class TimerInterface {
  public:
    virtual bool finalizou();
    virtual String getElapsedFormatado();
    virtual String getRemainingFormatado();
    virtual String getPausadoFormatado();
    virtual Alarme *getAlarme();
    virtual int getPercentualCompleto();
};


class ObserverAbstrato{
  public:
    virtual void update(TimerInterface *assunto) {};
    virtual void updateTela(TimerInterface *assunto) {};
};
