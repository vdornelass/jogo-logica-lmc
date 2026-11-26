#include "typing_engine.h"
#include <string.h>
#include <ctype.h>

/* 15 Desafios Explicitos de Logica Proposicional (6 Pilares da Disciplina) */
static const LogicChallenge LOGIC_CHALLENGES[MAX_PHRASES] = {
    /* ===== FASE 1: CONECTIVOS E TABELAS-VERDADE ===== */
    {
        "TABELA-VERDADE",
        "Se P = V e Q = F, qual o valor logico de (P -> Q)?",
        { "VERDADEIRO (V)", "FALSO (F)", "" },
        2, 1, true,
        "V -> F e o unico caso FALSO da condicional!"
    },
    {
        "VACUIDADE",
        "Se P = F e Q = F, qual o valor logico de (P -> Q)?",
        { "VERDADEIRO (V)", "FALSO (F)", "" },
        2, 0, true,
        "Antecedente F torna a condicional V (vacuidade)!"
    },
    {
        "CONECTIVOS",
        "Se P = V e Q = F, qual o valor de (P ^ Q) v (~Q)?",
        { "VERDADEIRO (V)", "FALSO (F)", "" },
        2, 0, true,
        "(V ^ F) v V = F v V = VERDADEIRO!"
    },
    {
        "BICONDICIONAL",
        "Se P = F e Q = F, qual o valor de (P <-> Q)?",
        { "VERDADEIRO (V)", "FALSO (F)", "" },
        2, 0, true,
        "Bicondicional e V quando ambos tem valores iguais!"
    },
    {
        "XOR EXCLUSIVO",
        "Se P = V e Q = V, qual o valor de (P XOR Q)?",
        { "VERDADEIRO (V)", "FALSO (F)", "" },
        2, 1, true,
        "Disjuncao exclusiva (XOR) e F se ambos forem V!"
    },

    /* ===== FASE 2: SEMANTICA, EQUIVALENCIAS E QUANTIFICADORES ===== */
    {
        "SEMANTICA",
        "Como se classifica a formula (P v ~P)?",
        { "TAUTOLOGIA", "CONTRADICAO", "CONTINGENCIA" },
        3, 0, false,
        "P v ~P resulta 100% em V: TAUTOLOGIA!"
    },
    {
        "SEMANTICA",
        "Como se classifica a formula (P ^ ~P)?",
        { "TAUTOLOGIA", "CONTRADICAO", "CONTINGENCIA" },
        3, 1, false,
        "P ^ ~P resulta 100% em F: CONTRADICAO!"
    },
    {
        "NEGACAO COND.",
        "Qual formula equivale a negacao ~(P -> Q)?",
        { "P ^ ~Q", "~P -> ~Q", "~P v Q" },
        3, 0, false,
        "Negar condicional: mantem P e nega Q (P ^ ~Q)!"
    },
    {
        "DE MORGAN",
        "Pela Lei de De Morgan, ~(P ^ Q) equivale a:",
        { "~P ^ ~Q", "~P v ~Q", "P v ~Q" },
        3, 1, false,
        "De Morgan troca ^ por v e nega ambos: ~P v ~Q!"
    },
    {
        "CONTRAPOSITIVA",
        "A formula (P -> Q) e equivalente a (~Q -> ~P)?",
        { "VERDADEIRO (V)", "FALSO (F)", "" },
        2, 0, true,
        "Contrapositiva inverte e nega: (P->Q) === (~Q->~P)!"
    },

    /* ===== FASE 3: METODO DEDUTIVO, FALACIAS E REFATORACAO ===== */
    {
        "MODUS PONENS",
        "Premissas: (P -> Q) e P. Conclusao: Q. Argumento:",
        { "VALIDO (MP)", "FALACIA", "" },
        2, 0, false,
        "Modus Ponens: de (P -> Q) e P conclui-se Q!"
    },
    {
        "MODUS TOLLENS",
        "Premissas: (P -> Q) e ~Q. Conclusao: ~P. Argumento:",
        { "VALIDO (MT)", "FALACIA", "" },
        2, 0, false,
        "Modus Tollens: de (P -> Q) e ~Q conclui-se ~P!"
    },
    {
        "FALACIA",
        "Premissas: (P -> Q) e Q. Conclusao: P. Argumento:",
        { "VALIDO", "FALACIA", "" },
        2, 1, false,
        "Falacia classica: Afirmacao do Consequente!"
    },
    {
        "FALACIA",
        "Premissas: (P -> Q) e ~P. Conclusao: ~Q. Argumento:",
        { "VALIDO", "FALACIA", "" },
        2, 1, false,
        "Falacia classica: Negacao do Antecedente!"
    },
    {
        "REFATORACAO",
        "Pela Lei da Absorcao, P v (P ^ Q) reduz para:",
        { "Q", "P ^ Q", "P" },
        3, 2, false,
        "Lei da Absorcao simplifica P v (P ^ Q) para P!"
    }
};

void typing_engine_init(TypingEngine *te) {
    te->phrase_count = MAX_PHRASES;
    te->control_mode = CONTROL_WASD_ARROWS;
    for (int i = 0; i < MAX_PHRASES; i++) {
        strncpy(te->phrases[i], LOGIC_CHALLENGES[i].question, MAX_PHRASE_LEN - 1);
        te->phrases[i][MAX_PHRASE_LEN - 1] = '\0';
    }
    typing_engine_reset(te);
}

void typing_engine_reset(TypingEngine *te) {
    for (int i = 0; i < MAX_PHRASES; i++) {
        te->challenge_order[i] = i;
    }
    te->phrase_index = 0;
    te->char_index = 0;
    te->error_flash_timer = 0.0f;
    te->lockout_timer = 0.0f;
    te->lockout_max = 1.8f;
    te->blind_timer = 0.0f;
    te->current_phrase_errors = 0;
    te->total_completed = 0;
    te->total_typed = 0;
    te->total_errors = 0;
}

void typing_engine_update(TypingEngine *te, float dt) {
    if (te->error_flash_timer > 0.0f) {
        te->error_flash_timer -= dt;
        if (te->error_flash_timer < 0.0f) te->error_flash_timer = 0.0f;
    }
    if (te->lockout_timer > 0.0f) {
        te->lockout_timer -= dt;
        if (te->lockout_timer < 0.0f) te->lockout_timer = 0.0f;
    }
    if (te->blind_timer > 0.0f) {
        te->blind_timer -= dt;
        if (te->blind_timer < 0.0f) te->blind_timer = 0.0f;
    }
}

void typing_engine_apply_blindness(TypingEngine *te, float duration) {
    te->blind_timer = duration;
}

void typing_engine_on_player_hit(TypingEngine *te) {
    if (te->char_index > 0) {
        te->char_index -= 3;
        if (te->char_index < 0) te->char_index = 0;
        te->error_flash_timer = 0.4f;
        te->current_phrase_errors++;
        te->total_errors++;
    }
}

const LogicChallenge *typing_engine_get_current_challenge(const TypingEngine *te) {
    if (te->phrase_index >= 0 && te->phrase_index < te->phrase_count) {
        int id = te->challenge_order[te->phrase_index];
        if (id >= 0 && id < MAX_PHRASES) {
            return &LOGIC_CHALLENGES[id];
        }
    }
    return NULL;
}

const char *typing_engine_get_current_phrase(const TypingEngine *te) {
    if (te->phrase_index < te->phrase_count) {
        int id = te->challenge_order[te->phrase_index];
        if (id >= 0 && id < MAX_PHRASES) {
            return te->phrases[id];
        }
    }
    return "";
}

bool typing_engine_is_finished(const TypingEngine *te) {
    return te->phrase_index >= te->phrase_count;
}

int typing_engine_get_accuracy(const TypingEngine *te) {
    int attempts = te->total_typed + te->total_errors;
    if (attempts <= 0) return 100;
    return (te->total_typed * 100) / attempts;
}

const char *typing_engine_submit_answer(TypingEngine *te, int option_idx, int *errors_out, const char **explanation_out) {
    if (typing_engine_is_finished(te)) return NULL;
    if (te->lockout_timer > 0.0f) return NULL;

    const LogicChallenge *ch = typing_engine_get_current_challenge(te);
    if (!ch || option_idx < 0 || option_idx >= ch->option_count) return NULL;

    if (option_idx == ch->correct_option) {
        te->total_typed++;
        if (errors_out) *errors_out = te->current_phrase_errors;
        if (explanation_out) *explanation_out = ch->explanation;
        te->phrase_index++;
        te->char_index = 0;
        te->current_phrase_errors = 0;
        te->total_completed++;
        return "COMPLETED";
    }

    /* Punicao anti-chute: trava o console e recicla a questao para o fim da fase atual */
    float lock_dur = (te->phrase_count > 1) ? 1.8f : 0.9f;
    te->error_flash_timer = lock_dur;
    te->lockout_timer = lock_dur;
    te->lockout_max = lock_dur;
    te->current_phrase_errors++;
    te->total_errors++;
    if (explanation_out) *explanation_out = ch->explanation;

    if (te->phrase_count > 1) {
        int phase_end = ((te->phrase_index / 5) + 1) * 5 - 1;
        if (phase_end >= te->phrase_count) phase_end = te->phrase_count - 1;
        if (te->phrase_index < phase_end) {
            int failed_id = te->challenge_order[te->phrase_index];
            for (int i = te->phrase_index; i < phase_end; i++) {
                te->challenge_order[i] = te->challenge_order[i + 1];
            }
            te->challenge_order[phase_end] = failed_id;
        }
    }

    return "ERROR";
}

/* Mantido para compatibilidade com o modo Tutorial guiado */
static bool is_movement_key(int key, int control_mode) {
    if (key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT) {
        return true;
    }
    if (control_mode == CONTROL_WASD_ARROWS) {
        int k = toupper((unsigned char)key);
        if (k == 'W' || k == 'A' || k == 'S' || k == 'D') return true;
        if (key == KEY_W || key == KEY_A || key == KEY_S || key == KEY_D) return true;
    }
    return false;
}

const char *typing_engine_handle_key(TypingEngine *te, int key, int *errors_out) {
    if (typing_engine_is_finished(te)) return NULL;

    const char *target = typing_engine_get_current_phrase(te);
    if (!target || target[0] == '\0') return NULL;

    char expected = target[te->char_index];
    char typed = (char)key;

    if (key == KEY_SPACE) {
        typed = ' ';
    }

    if (toupper((unsigned char)typed) == toupper((unsigned char)expected)) {
        te->char_index++;
        te->total_typed++;
        if (te->char_index >= (int)strlen(target)) {
            if (errors_out) *errors_out = te->current_phrase_errors;
            te->phrase_index++;
            te->char_index = 0;
            te->current_phrase_errors = 0;
            te->total_completed++;
            return "COMPLETED";
        }
        return "CORRECT";
    }

    if (is_movement_key(key, te->control_mode)) {
        return NULL;
    }

    te->error_flash_timer = 0.25f;
    te->current_phrase_errors++;
    te->total_errors++;
    return "ERROR";
}
