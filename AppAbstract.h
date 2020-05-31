/*****************************************************************************/
/******** interface que define o padrao de estados e o padrao delegate *******/
/*****************************************************************************/
//Inclui as funcoes necessarias para delegar a alteracao de estado
enum Nome_estados {Principal=0, Manual=1, Automatico=2, Fervura=3, ConfiguraFervura=4, Refrigeracao=5, Configuracao=6};

class AppAbstract {
  public:
    virtual void gotoEstado(Nome_estados nome);
    Timer *timer;
    Teclado *teclado;
    OnOff *controlador;
};
