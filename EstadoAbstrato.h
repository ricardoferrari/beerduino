//Estado abstrato
class EstadoAbstrato: public ObserverAbstrato, public Restauravel {
  public:
    virtual void run();
    virtual void adiciona(int qtd);
    virtual void subtrai(int qtd);
    virtual void enter();
    virtual void cancel();
  
};
