class Alarme {
  public:
      Alarme () {
      }

      void addAlarme(byte tempo, String msg) {
          if (alarme_posicao < alarme_qtd) {
            alarmes[alarme_posicao] = tempo;
            mensagens[alarme_posicao] = msg;
            alarme_posicao++;
          }
      }

      void setQtdAlarme(byte _alarme_qtd) {
        limpaAlarme();
        alarme_qtd = _alarme_qtd;
      }

      byte getQtdAlarme() {
        return alarme_qtd;
      }

      String getMensagem() {
        if (temAlarme()) {
          return mensagens[alarme_atual];
        }
        return "";
      }
      
      void limpaAlarme() {
        alarme_posicao = 0;
        alarme_ativo = false;
        alarme_disparado = false;
      }
      
      void habilitaAlarme() {
        alarme_ativo = true;
      }
    
      void reconheceAlarme() {
        if (alarme_atual<(alarme_posicao-1)) alarme_atual++;
        else alarme_ativo = false;
        
        alarme_disparado = false;
      }

      bool checaAlarme(byte pv) {
        if (alarme_ativo && (pv >= alarmes[alarme_atual]) && !alarme_disparado) {
            alarme_disparado = true;
            return true;
        } else return false;
      }
      
      bool temAlarme() {
        return alarme_disparado;
      }
      
      
  protected:
    byte alarmes[3];
    String mensagens[3];
    byte alarme_posicao = 0; //posicao para adicionar o alarme
    byte alarme_atual= 0; // posicao do alarme atual
    bool alarme_ativo = false;
    bool alarme_disparado = false;
  private:
    byte alarme_qtd = 0;
};
