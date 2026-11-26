#ifndef TYPING_ENGINE_H
#define TYPING_ENGINE_H

#include "config.h"

#define MAX_PHRASES 15
#define MAX_PHRASE_LEN 128
#define MAX_LOGIC_OPTIONS 3

typedef struct {
    const char *category;
    const char *question;
    const char *options[MAX_LOGIC_OPTIONS];
    int option_count;
    int correct_option; /* 0-based index */
    bool is_vf;         /* true se aceita atalhos [V] e [F] */
    const char *explanation;
} LogicChallenge;

typedef struct {
    char phrases[MAX_PHRASES][MAX_PHRASE_LEN];
    int challenge_order[MAX_PHRASES];
    int phrase_count;
    int phrase_index;
    int char_index;
    float error_flash_timer;
    float lockout_timer;
    float lockout_max;
    float blind_timer;
    int current_phrase_errors;
    int total_completed;
    int total_typed;
    int total_errors;
    int control_mode;
} TypingEngine;

void typing_engine_init(TypingEngine *te);
void typing_engine_reset(TypingEngine *te);
void typing_engine_update(TypingEngine *te, float dt);
const char *typing_engine_handle_key(TypingEngine *te, int key, int *errors_out);
const char *typing_engine_submit_answer(TypingEngine *te, int option_idx, int *errors_out, const char **explanation_out);
const LogicChallenge *typing_engine_get_current_challenge(const TypingEngine *te);
void typing_engine_on_player_hit(TypingEngine *te);
void typing_engine_apply_blindness(TypingEngine *te, float duration);
const char *typing_engine_get_current_phrase(const TypingEngine *te);
bool typing_engine_is_finished(const TypingEngine *te);
int typing_engine_get_accuracy(const TypingEngine *te);

#endif /* TYPING_ENGINE_H */
