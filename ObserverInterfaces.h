/****** Interfaces necessarias para a atualizacao apos o assunto notificar o observer *******/

class AssuntoI: public Alarme {
  public:
    virtual bool finalizou();
    virtual String getElapsedFormatado();
    virtual String getRemainingFormatado();
    virtual String getPausadoFormatado();
};


class ObserverAbstrato{
  public:
    virtual void update(AssuntoI *assunto) {};
    virtual void updateTela(AssuntoI *assunto) {};
};
