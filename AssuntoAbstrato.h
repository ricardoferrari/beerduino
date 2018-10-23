class AssuntoAbstrato: public AssuntoI{
  public:
      virtual void attach(ObserverAbstrato *observer);
      virtual void detach(ObserverAbstrato *observer);
      virtual void notify();
      virtual void notifyTela();
};

