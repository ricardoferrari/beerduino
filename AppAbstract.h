/*****************************************************************************/
/******** interface que define o padrao de estados e o padrao delegate *******/
/*****************************************************************************/
//Inclui as funcoes necessarias para delegar a alteracao de estado
enum Nome_estados {Principal, Manual, Automatico, Fervura, ConfiguraFervura, Configuracao};

class AppAbstract: public OnOffDelegate {
  public:
    virtual void gotoEstado(Nome_estados nome);
    Timer *timer;
    Teclado *teclado;
    OnOff *controlador;
};
