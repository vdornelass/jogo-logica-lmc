# Lógica para Computação: Batalha contra SOFISMA-67

Jogo educacional em **C99 + Raylib** desenvolvido para a disciplina de **Lógica para Computação (LMC)** — **CESAR School (ADS 2026.2)**.

Em uma arena estilo *bullet hell*, o jogador assume o papel de um auditor lógico que precisa desviar dos ataques de **SOFISMA-67** enquanto avalia, classifica e refuta **15 proposições explícitas de Lógica Proposicional** distribuídas em 3 fases progressivas.

---

## Como Jogar

### Windows (recomendado)
1. Clone ou baixe este repositório.
2. Dê duplo clique em **`jogar_logica.bat`** (o script detecta seu compilador local `gcc`/`MinGW`/`WSL` ou usa o executável já incluído em `bin/jogo.exe`).

### Linux / WSL
```bash
make
make run
```

---

## Controles

| Ação | Teclas / Interação |
| :--- | :--- |
| **Mover na arena** | `WASD`, `Setas` ou `Mouse` *(configurável em Configurações)* |
| **Responder desafio lógico** | `[1]`, `[2]`, `[3]`, `[V]` / `[F]` *(questões de Verdadeiro/Falso)* ou **clique do mouse** no cartão da alternativa |
| **Tutorial guiado** | `[T]` no menu principal *(ensina as 3 fases e treina sem perda de vida)* |
| **Tela cheia / Maximizar** | `[F11]` |
| **Pausar / Voltar** | `[ESC]` |

---

## Aplicação Explícita da Lógica Proposicional (3 Fases — 15 Desafios)

Cada proposição correta aplica **100 de dano** ao chefe **SOFISMA-67** (`1500 HP` no total) e exibe na tela a **justificativa formal** do resultado:

### Fase 1 — Tabelas-Verdade e Conectivos (`V` / `F`)
1. `P = V, Q = F` → Avalie `(P -> Q)` → **`FALSO (F)`** *(único caso falso da condicional: `V -> F = F`)*
2. `P = F, Q = F` → Avalie `(P -> Q)` → **`VERDADEIRO (V)`** *(verdade por vacuidade)*
3. `P = V, Q = F` → Avalie `(P ^ Q) v (~Q)` → **`VERDADEIRO (V)`**
4. `P = F, Q = F` → Avalie `(P <-> Q)` → **`VERDADEIRO (V)`** *(bicondicional com valores iguais)*
5. `P = V, Q = V` → Avalie `(P XOR Q)` → **`FALSO (F)`** *(disjunção exclusiva)*

### Fase 2 — Classificação Semântica e Equivalências Notáveis
6. Classifique `(P v ~P)` → **`TAUTOLOGIA`**
7. Classifique `(P ^ ~P)` → **`CONTRADICAO`**
8. Negação da condicional `~(P -> Q)` → **`P ^ ~Q`**
9. Lei de De Morgan `~(P ^ Q)` → **`~P v ~Q`**
10. Contrapositiva `(P -> Q) === (~Q -> ~P)` → **`VERDADEIRO (V)`**

### Fase 3 — Método Dedutivo, Falácias e Refatoração Booleana
11. `P -> Q; P |- Q` *(Modus Ponens)* → **`VALIDO (MP)`**
12. `P -> Q; ~Q |- ~P` *(Modus Tollens)* → **`VALIDO (MT)`**
13. `P -> Q; Q |- P` *(Afirmação do Consequente)* → **`FALACIA`**
14. `P -> Q; ~P |- ~Q` *(Negação do Antecedente)* → **`FALACIA`**
15. Lei da Absorção `P v (P ^ Q)` → **`P`**

---

## Mecânicas Criativas Ligadas à Lógica

* **Pulso Lógico (EMP) no acerto de 1ª:** acertar a proposição sem errar concede `+150 PTS`, causa `100` de dano no chefe e dispara um pulso que **destrói todos os projéteis em um raio de `250px`**.
* **Sistema Anti-Chute (Punição por Contradição):** errar uma proposição impede avançar por eliminação e aplica 4 penalidades imediatas:
  1. **`-1 HP` direto** (errar repetidamente leva a *Game Over* por Contradição Crítica);
  2. **`-50 PTS`** no placar + redução na **Precisão Lógica (%)**;
  3. **Trava de Contradição (`1.8s`)** com barra regressiva vermelha no console enquanto **SOFISMA-67 dispara um anel retaliatório de 8 projéteis**;
  4. **Reciclagem da questão na fase:** a proposição errada volta para o final da fila da fase atual e outra questão entra no lugar.
* **Relatório e Ranking Local:** ao concluir a batalha, o jogo exibe a Precisão Lógica, pontuação final, ranking Top 10 e gera um certificado de conclusão.

---

## Estrutura do Projeto

```text
.
├── assets/             # Fontes TrueType e recursos visuais
├── bin/                # Executável Windows (jogo.exe) e raylib.dll
├── include/            # Cabeçalhos Raylib
├── lib/                # Bibliotecas Raylib (Linux e Windows)
├── src/
│   ├── main.c          # Loop principal, menus, combate e Pulso Lógico EMP
│   ├── typing_engine.c # Motor dos 15 desafios explícitos, trava e reciclagem
│   ├── ui.c            # Console interativo lógico, HUD, partículas e telas finais
│   ├── tutorial.c      # Modo treino guiado para as 3 fases de LMC
│   ├── boss.c          # IA e padrões de ataque de SOFISMA-67
│   ├── bullet.c        # Gerenciamento de projéteis e anomalias
│   ├── player.c        # Movimentação (WASD / Setas / Mouse) e vida
│   ├── audio.c         # Efeitos sonoros procedurais
│   ├── ranking.c       # Ranking Top 10 e estatísticas
│   ├── save.c          # Persistência de sessão e histórico de partidas
│   └── report.c        # Geração do certificado final
├── Makefile            # Build C99 estrito (-Wall -Wextra -std=c99 -pedantic)
└── jogar_logica.bat    # Compilador/lançador rápido para Windows
```
