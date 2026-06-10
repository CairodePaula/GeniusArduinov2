// ═══════════════════════════════════════════════
//  GENIUS  –  não-bloqueante (millis)
// ═══════════════════════════════════════════════

const int LEDS[4]  = {10, 11, 12, 13};
const int BTNS[4]  = {2, 3, 4, 7};
const int BUZZER   = 8;
const int NOTAS[4] = {262, 330, 392, 523};

// ── Sequência ──
int sequencia[50];
int fase       = 0;
int inputAtual = 0;

// ── Flags de estado ──
bool mostrando = false;
bool esperando = false;
bool ledState[4] = {false,false,false,false};

// ── Serial ──
String serialBuf;

// ── Máquina de estados ──
enum Estado {
  ST_IDLE,
  ST_PRE_SHOW,       // pausa antes de começar a sequência
  ST_LED_ON,         // LED aceso durante a sequência
  ST_LED_OFF,        // pausa entre LEDs da sequência
  ST_WAIT_INPUT,     // esperando jogador
  ST_PLAYER_LED_ON,  // LED aceso após input do jogador
  ST_PLAYER_LED_OFF, // apaga após input
  ST_GAMEOVER_ON,    // flash de game over aceso
  ST_GAMEOVER_OFF    // flash de game over apagado
};

Estado estado = ST_IDLE;
unsigned long tTimer  = 0;
int           seqIdx  = 0;      // qual led da sequência está tocando
int           goFlash = 0;      // contador de flashes de game over
int           playerBtn = -1;   // botão que o jogador pressionou
bool          playerAcertou = false;

// resposta pendente para o Chrome (serial)
// -1 = nenhuma  0 = errou  1 = acertou-continua  2 = acertou-avança
int pendingOk = -1;


// ─────────────────────────────────────────────
// HELPERS
// ─────────────────────────────────────────────

void setLED(int n, bool on) {
  ledState[n] = on;
  digitalWrite(LEDS[n], on ? HIGH : LOW);
}

void allLEDs(bool on) {
  for (int i = 0; i < 4; i++) setLED(i, on);
}

void enviaEstado() {
  Serial.print(F("{\"fase\":"));
  Serial.print(fase);
  Serial.print(F(",\"mostrando\":"));
  Serial.print(mostrando ? F("true") : F("false"));
  Serial.print(F(",\"esperando\":"));
  Serial.print(esperando ? F("true") : F("false"));
  Serial.print(F(",\"leds\":["));
  for (int i = 0; i < 4; i++) {
    Serial.print(ledState[i] ? F("true") : F("false"));
    if (i < 3) Serial.print(F(","));
  }
  Serial.println(F("]}"));
}

void enviaPendingOk() {
  if (pendingOk == -1) return;
  if (pendingOk == 0)  Serial.println(F("{\"ok\":false}"));
  else                 Serial.println(F("{\"ok\":true}"));
  pendingOk = -1;
}

void timer(unsigned long ms) { tTimer = millis() + ms; }
bool timerOk()               { return millis() >= tTimer; }


// ─────────────────────────────────────────────
// INICIAR SEQUÊNCIA
// ─────────────────────────────────────────────

void iniciarSequencia() {
  mostrando  = true;
  esperando  = false;
  seqIdx     = 0;
  estado     = ST_PRE_SHOW;
  timer(800);
  enviaEstado();
}


// ─────────────────────────────────────────────
// PROCESSAR INPUT DO JOGADOR
// btn: qual botão  |  viaSerial: true=Chrome false=físico
// ─────────────────────────────────────────────

void processarInput(int btn, bool viaSerial) {
  if (!esperando) {
    if (viaSerial) Serial.println(F("{\"ok\":false}"));
    return;
  }

  playerBtn    = btn;
  playerAcertou = (btn == sequencia[inputAtual]);

  if (playerAcertou) {
    inputAtual++;
    bool avanca = (inputAtual > fase);
    if (viaSerial) {
      // Chrome recebe ok imediatamente, sem esperar o LED
      pendingOk = avanca ? 2 : 1;
    }
  } else {
    if (viaSerial) pendingOk = 0;
  }

  // Bloqueia input enquanto processa
  esperando = false;
  mostrando = false;

  // Acende LED do botão pressionado
  setLED(btn, true);
  tone(BUZZER, NOTAS[btn], 150);
  estado = ST_PLAYER_LED_ON;
  timer(150);
  enviaEstado();

  // Manda ok pro Chrome antes do delay visual
  enviaPendingOk();
}


// ─────────────────────────────────────────────
// UPDATE DA MÁQUINA DE ESTADOS
// ─────────────────────────────────────────────

void update() {
  if (!timerOk()) return;

  switch (estado) {

    // ── Pré-sequência: pausa inicial ──
    case ST_PRE_SHOW:
      setLED(sequencia[seqIdx], true);
      tone(BUZZER, NOTAS[sequencia[seqIdx]], 300);
      enviaEstado();
      estado = ST_LED_ON;
      timer(300);
      break;

    // ── LED aceso: apaga e decide próximo ──
    case ST_LED_ON:
      setLED(sequencia[seqIdx], false);
      noTone(BUZZER);
      enviaEstado();
      estado = ST_LED_OFF;
      timer(200);
      break;

    // ── Pausa entre LEDs ──
    case ST_LED_OFF:
      seqIdx++;
      if (seqIdx <= fase) {
        // próximo LED
        setLED(sequencia[seqIdx], true);
        tone(BUZZER, NOTAS[sequencia[seqIdx]], 300);
        enviaEstado();
        estado = ST_LED_ON;
        timer(300);
      } else {
        // sequência terminou
        mostrando  = false;
        esperando  = true;
        inputAtual = 0;
        estado     = ST_WAIT_INPUT;
        enviaEstado();
      }
      break;

    // ── Aguardando: nada a fazer aqui ──
    case ST_WAIT_INPUT:
      break;

    // ── LED do jogador aceso: apaga ──
    case ST_PLAYER_LED_ON:
      setLED(playerBtn, false);
      noTone(BUZZER);
      enviaEstado();
      estado = ST_PLAYER_LED_OFF;
      timer(80);
      break;

    // ── Pós-input: decide o que fazer ──
    case ST_PLAYER_LED_OFF:
      if (playerAcertou) {
        if (inputAtual > fase) {
          // avançou de fase
          fase++;
          sequencia[fase] = random(4);
          timer(600);
          estado = ST_PRE_SHOW;
          mostrando = true;
          esperando = false;
          enviaEstado();
        } else {
          // continua na mesma fase
          esperando  = true;
          mostrando  = false;
          estado     = ST_WAIT_INPUT;
          enviaEstado();
        }
      } else {
        // game over: inicia flashes
        goFlash = 0;
        allLEDs(true);
        tone(BUZZER, 150, 300);
        enviaEstado();
        estado = ST_GAMEOVER_ON;
        timer(300);
      }
      break;

    // ── Game over flash aceso ──
    case ST_GAMEOVER_ON:
      allLEDs(false);
      noTone(BUZZER);
      enviaEstado();
      estado = ST_GAMEOVER_OFF;
      timer(200);
      break;

    // ── Game over flash apagado ──
    case ST_GAMEOVER_OFF:
      goFlash++;
      if (goFlash < 3) {
        allLEDs(true);
        tone(BUZZER, 150, 300);
        enviaEstado();
        estado = ST_GAMEOVER_ON;
        timer(300);
      } else {
        // reinicia
        fase = 0;
        sequencia[fase] = random(4);
        iniciarSequencia();
      }
      break;

    default:
      break;
  }
}


// ─────────────────────────────────────────────
// COMANDOS SERIAL
// ─────────────────────────────────────────────

void processarComando(String cmd) {
  cmd.trim();

  if (cmd == "E") {
    enviaEstado();
  }
  else if (cmd.startsWith("B")) {
    int btn = cmd.charAt(1) - '0';
    if (btn >= 0 && btn <= 3) processarInput(btn, true);
    else Serial.println(F("{\"ok\":false}"));
  }
  else if (cmd == "R") {
    fase = 0;
    sequencia[fase] = random(4);
    Serial.println(F("{\"ok\":true}"));
    iniciarSequencia();
  }
}


// ─────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  serialBuf.reserve(64);

  for (int i = 0; i < 4; i++) {
    pinMode(LEDS[i], OUTPUT);
    pinMode(BTNS[i], INPUT_PULLUP);
  }
  pinMode(BUZZER, OUTPUT);

  randomSeed(analogRead(A3));
  sequencia[0] = random(4);

  delay(1000);
  iniciarSequencia();
}


// ─────────────────────────────────────────────
// LOOP  –  nunca bloqueia
// ─────────────────────────────────────────────

void loop() {

  // ── Máquina de estados ──
  update();

  // ── Botões físicos (só quando esperando) ──
  if (esperando) {
    for (int i = 0; i < 4; i++) {
      if (digitalRead(BTNS[i]) == LOW) {
        delay(30); // debounce mínimo
        if (digitalRead(BTNS[i]) == LOW) {
          processarInput(i, false);
          while (digitalRead(BTNS[i]) == LOW); // espera soltar
        }
      }
    }
  }

  // ── Serial ──
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      processarComando(serialBuf);
      serialBuf = "";
    } else {
      serialBuf += c;
    }
  }
}
