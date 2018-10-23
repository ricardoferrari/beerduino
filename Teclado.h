class TecladoDelegate {
  public:
    virtual void adiciona(int qtd);
    virtual void subtrai(int qtd);
    virtual void enter();
    virtual void cancel();
};

class Teclado {
        public:
        
        Teclado(TecladoDelegate *delegate) {
          _delegate = delegate;
        }

        void setup() {
          pinMode(Button_enter, INPUT_PULLUP);
          pinMode(Button_cancel, INPUT_PULLUP);
          pinMode(Button_up, INPUT_PULLUP);
          pinMode(Button_dn, INPUT_PULLUP);
        }
        
        void run() {
          if (botaoEnter()) { _delegate->enter(); }
          if (botaoCancel()) { _delegate->cancel(); }
          //Botao up/down precionado
          int incremento = botaoUpDownPressionado();
          if (incremento>0) { _delegate->adiciona(abs(incremento)); }
          else if (incremento<0) { _delegate->subtrai(abs(incremento)); }
        }
        
        int botaoUpDownPressionado(){
          int step_size = 1;
          bool pressionado = false;

          if (  botaoPulsado (Button_up, &bt_up) || botaoPulsado (Button_dn, &bt_down)  ) return ((bt_up)-(bt_down));
          else if(bt_up || bt_down){
            unsigned long millis_pressionado = (millis()-lastPressionado); //Contagem desde o inicio da pressao
            bool pulso = (millis()-lastPulso)>bouncing;                         //COntagem a cada ciclo de pulso

            if ((millis_pressionado>=bouncing) && pulso) { // Tempo minimo para evitar bouncing
              lastPulso = millis();
              if(singleton) step_size=1;
              else if((millis_pressionado/1000)>=8) step_size=10;
              else if((millis_pressionado/1000)>=4) step_size=5;
              else step_size = 1;
              singleton=false;
            } else step_size = 0;
        
            //Incrementa a variavel em questao
            return ((bt_up)*step_size-(bt_down)*step_size);
          }

          return 0;
        }
        
        
        // some seconds button press
        byte botaoPulsado (byte Button_press, boolean* pressionado){
        
          if (digitalRead (Button_press)==0){  //Pressiona o botao
            if(!*pressionado){lastPressionado=millis(); singleton=true;}  //Se nao estava pressionado antes inicia a contagem do tempo
            *pressionado = true;
          } else *pressionado=false;
        
          if(*pressionado){
            unsigned long millis_pressionado = (millis()-lastPressionado);
            if ((millis_pressionado>=debounce) && singleton) {singleton=false; return 1;}
          }   
         
          return 0;
        }

        byte botaoEnter(){
          return botaoPulsado (Button_enter, &bt_enter);
        }

        byte botaoCancel(){
          return botaoPulsado (Button_cancel, &bt_cancel);
        }
        
        private:
        TecladoDelegate *_delegate;      
        unsigned long lastPressionado;
        unsigned long lastPulso;
        boolean bt_enter = false;
        boolean bt_cancel = false;
        boolean bt_up = false;
        boolean bt_down = false;
        boolean singleton = false; //Utilizado para botoes que devem gerar somente um pulso ao se manterem pressionados
        const long debounce = 45;
        const int bouncing = 250;

        const char Button_enter = A0;
        const char Button_cancel = A1;
        const char Button_up    = A2;
        const char Button_dn    = A3;

};
