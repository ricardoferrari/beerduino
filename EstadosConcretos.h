class Principal_Estado: public EstadoAbstrato {
  public:
    Principal_Estado(AppAbstract *delegate) {
      _delegate = delegate;
    }

    ~Principal_Estado() {
      Serial.print("MenuPrincDestruido");
    }

    void update(AssuntoI *assunto) {}
    void updateTela(AssuntoI *assunto) {}
    
    void run() {
      if (!tela_atualizada) {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(" MENU PRINCIPAL ");
          switch(opcao) {
            case 1 :
                lcd.setCursor(0,1);
                lcd.print("     MANUAL     ");
                break;
            case 2 :
                lcd.setCursor(0,1);
                lcd.print("RAMPAS PATAMARES");
                break;
            case 3 :
                lcd.setCursor(0,1);
                lcd.print("    FERVURA     ");
                break;
            default:
                lcd.setCursor(0,1);
                lcd.print("  CONFIGURACAO  ");
                break;
          }
          tela_atualizada = true;
      }
    }

    void adiciona(int qtd) {
      if (opcao < opcaoQtd) {
          opcao++;
          tela_atualizada = false;
      }
    }

    void subtrai(int qtd) {
      if (opcao > 1) {
          opcao--;
          tela_atualizada = false;
      }
    }

    void enter() {
      switch(opcao) {
        case 1:_delegate->gotoEstado(Manual);
        break;
        case 2:_delegate->gotoEstado(Automatico);
        break;
        case 3:_delegate->gotoEstado(ConfiguraFervura);
        break;
        default: _delegate->gotoEstado(Configuracao);
        break;
      }
    }

    void cancel() {}
    
  private:
    byte opcaoQtd = 4;
    byte opcao = 1;
};
