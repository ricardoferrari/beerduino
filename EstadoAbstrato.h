//Estado abstrato
class EstadoAbstrato: public ObserverAbstrato {
  public:
    virtual void run();
    virtual void adiciona(int qtd);
    virtual void subtrai(int qtd);
    virtual void enter();
    virtual void cancel();

  protected:
    AppAbstract *_delegate;
    bool tela_atualizada = false;
};
