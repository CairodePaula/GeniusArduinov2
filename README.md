# Genius Arduino 🧠
### Jogo Genius (Simon Says) com Arduino Uno — USB direto via Web Serial API

> **v2.0** — Migração de ESP8266/WiFi para comunicação USB direta (Web Serial API).  
> Atualização documentada por **Luiz** (novo integrante).

---

## 📚 Objetivo

Recriar o jogo Genius (Simon Says) com Arduino Uno, controlado por uma interface web via USB, aplicando conceitos de IoT, comunicação serial e metodologia ágil.

### Objetivos Específicos

* Comunicação direta entre navegador e Arduino via **Web Serial API** (USB)
* Interface web interativa em HTML/CSS/JavaScript sem servidor intermediário
* Controlar LEDs e buzzer para feedback visual e sonoro
* Lógica de jogo **não-bloqueante** com máquina de estados (`millis()`)
* Suporte simultâneo a botões físicos e cliques na interface web

---

## 🔄 O que mudou na v2.0

| Item | v1.0 | v2.0 |
|---|---|---|
| Comunicação | ESP8266 WiFi + HTTP | USB direto (Web Serial API) |
| Servidor intermediário | `server.py` (Python) | ❌ removido |
| Arquivo secrets | `secrets.h` (SSID/senha) | ❌ removido |
| Baud rate | 9600 | 115200 |
| Loop Arduino | Bloqueante (`delay`) | Não-bloqueante (`millis`) |
| Botão físico | Parcial | ✅ funcional simultâneo |
| Como abrir o HTML | Qualquer navegador | Chrome/Edge (Web Serial) |
| Arquivo principal | `genius.ino` | `genius_serial.ino` |

---

## 🎮 Como funciona

* O Arduino gera uma sequência aleatória de LEDs e sons
* A interface web exibe o painel do jogo no navegador via USB
* O jogador repete a sequência clicando na tela **ou** nos botões físicos
* A cada fase acertada, a sequência cresce
* Se errar, o jogo reinicia automaticamente com animação de game over

---

## 👥 Integrantes

* Cairo de Paula
* Guilherme Ramos Rangel
* Pedro Rangel Silva
* Gustavo Franca
* **Luiz** *(v2.0 — migração USB + documentação)*

---

## 🛠️ Hardware

* Arduino Uno
* 4 LEDs — Vermelho, Amarelo, Verde, Azul
* Buzzer passivo
* 4 botões físicos
* Resistores
* Protoboard
* Cabo USB (dados)

> ⚠️ O módulo **ESP8266 ESP-01S** e todos os jumpers associados foram **removidos** na v2.0.

---

## 🔌 Pinagem

| Componente | Pino Arduino |
|---|---|
| LED vermelho | 10 |
| LED amarelo | 11 |
| LED verde | 12 |
| LED azul | 13 |
| Buzzer | 8 |
| Botão 0 (vermelho) | 2 |
| Botão 1 (amarelo) | 3 |
| Botão 2 (verde) | 4 |
| Botão 3 (azul) | 7 |

---

## 🚀 Como rodar

### 1. Grave o firmware no Arduino

Abra `genius_serial.ino` na Arduino IDE e faça o upload para o Arduino Uno.

---

### 2. Inicie um servidor local

Na pasta do projeto, execute:

```bash
python -m http.server 8000
```

---

### 3. Abra no Chrome ou Edge

```
http://localhost:8000/genius.html
```

> ⚠️ Não abra como `file://` — o Chrome bloqueia Web Serial nesse modo.

---

### 4. Conecte o Arduino

Clique em **CONECTAR USB**, selecione a porta do Arduino e aguarde ~2 segundos.  
O jogo inicia automaticamente.

---

## 🧱 Arquitetura do Sistema

```text
Navegador Chrome/Edge
  (HTML + CSS + JS)
        ↕ Web Serial API (USB)
    Arduino Uno
        ↓
  LEDs + Buzzer + Botões físicos
```

---

## 📡 Protocolo Serial (115200 baud)

| Comando enviado | Resposta | Descrição |
|---|---|---|
| `E\n` | JSON de estado | Solicita estado atual |
| `B{n}\n` | `{"ok":true/false}` | Botão n pressionado (0–3) |
| `R\n` | `{"ok":true}` | Reinicia o jogo |

**JSON de estado:**
```json
{"fase":0,"mostrando":false,"esperando":true,"leds":[false,false,false,false]}
```

> O Arduino envia `enviaEstado()` automaticamente a cada mudança — o navegador não precisa fazer polling.

---

## 📦 Stack Tecnológica

* Arduino Uno
* C++ (máquina de estados com `millis()`)
* HTML5 / CSS3 / JavaScript (Vanilla)
* Web Serial API (Chrome/Edge 89+)

---

## 📁 Estrutura do Projeto

```text
genius-arduino/
├── genius_serial.ino   ← firmware Arduino (v2.0)
├── genius.html         ← interface web (v2.0)
├── README.md
└── Arduino-Picture.png
```

---

## ✅ Funcionalidades

* Sequência aleatória de LEDs e sons
* Interface web responsiva com visual retrô
* Comunicação USB direta — sem WiFi, sem servidor Python
* Botões físicos e cliques na tela funcionam simultaneamente
* Feedback visual imediato de acerto/erro
* Reinício automático com animação de game over
* Loop não-bloqueante — Arduino nunca trava

---

## 📋 Metodologia Ágil

Projeto organizado em quadro **Kanban** no Trello com User Stories e tarefas técnicas distribuídas entre os integrantes.

---

## 📚 Conceitos Aplicados

* Internet das Coisas (IoT)
* Comunicação Serial USB
* Web Serial API
* Máquina de estados embarcada
* Metodologia Ágil / Kanban
* Programação embarcada não-bloqueante
