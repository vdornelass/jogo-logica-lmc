#include "ui.h"
#include <stdio.h>
#include <stdlib.h>

/* =========================================================================
 * Fonte TrueType (PI2-259 - Matheus Chaves)
 * JetBrains Mono (SIL OFL 1.1) com ASCII + Latin-1 (acentos do portugues).
 * Se o arquivo nao for encontrado, usa a fonte padrao do Raylib (fallback).
 * ========================================================================= */
#define UI_FONT_PATH       "assets/fonts/JetBrainsMono-Regular.ttf"
#define UI_FONT_LOAD_SIZE  64     /* tamanho de rasterizacao (nitidez) */
#define UI_FONT_SCALE      1.2f   /* a TTF e menor que a fonte padrao no mesmo tamanho: iguala a altura visual */
#define UI_FONT_Y_SHIFT    0.18f  /* sobe o texto (fracao do tamanho) para alinhar com o layout original */

static Font g_font;
static bool g_font_loaded = false;   /* g_font foi inicializada */
static bool g_font_custom = false;   /* g_font e a TTF (e nao o fallback) */

void ui_load_fonts(void) {
    g_font = GetFontDefault();
    g_font_loaded = true;
    g_font_custom = false;

    /* Procura a fonte na raiz do jogo e, se o .exe for aberto de dentro de bin/, uma pasta acima */
    const char *path = NULL;
    if (FileExists(UI_FONT_PATH)) path = UI_FONT_PATH;
    else if (FileExists("../" UI_FONT_PATH)) path = "../" UI_FONT_PATH;
    if (path == NULL) {
        TraceLog(LOG_WARNING, "UI: fonte %s nao encontrada, usando fonte padrao", UI_FONT_PATH);
        return;
    }

    /* ASCII (32-126) + Latin-1 (160-255): sem essa lista os acentos viram '?' */
    int count = (126 - 32 + 1) + (255 - 160 + 1);
    int *codepoints = (int *)malloc((size_t)count * sizeof(int));
    if (codepoints == NULL) return;
    int n = 0;
    for (int c = 32; c <= 126; c++) codepoints[n++] = c;
    for (int c = 160; c <= 255; c++) codepoints[n++] = c;

    Font f = LoadFontEx(path, UI_FONT_LOAD_SIZE, codepoints, n);
    free(codepoints);
    if (f.texture.id == 0 || f.glyphCount == 0) return;   /* falhou: mantem o fallback */

    SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
    g_font = f;
    g_font_custom = true;
}

void ui_unload_fonts(void) {
    if (g_font_custom) UnloadFont(g_font);
    g_font_custom = false;
    g_font_loaded = false;
}

bool ui_font_is_custom(void) { return g_font_custom; }

static Font ui_font(void) { return g_font_loaded ? g_font : GetFontDefault(); }

static float ui_font_size(int size) { return g_font_custom ? size * UI_FONT_SCALE : (float)size; }

/* Fallback usa o mesmo espacamento do DrawText original (size/10) */
static float ui_font_spacing(int size) {
    if (g_font_custom) return 0.0f;
    int def = 10;
    return (float)((size < def ? def : size) / def);
}

void ui_draw_text(const char *txt, int x, int y, int size, Color color) {
    float fy = (float)y;
    if (g_font_custom) fy -= size * UI_FONT_Y_SHIFT;
    DrawTextEx(ui_font(), txt, (Vector2){ (float)x, fy }, ui_font_size(size), ui_font_spacing(size), color);
}

int ui_measure_text(const char *txt, int size) {
    return (int)MeasureTextEx(ui_font(), txt, ui_font_size(size), ui_font_spacing(size)).x;
}
#include <string.h>
#include <math.h>

void ui_init(UIManager *ui) {
    ui->shake_timer = 0.0f;
    ui->shake_intensity = 0.0f;
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) ui->texts[i].active = false;
    for (int i = 0; i < MAX_PARTICLES; i++) ui->particles[i].active = false;
}

void ui_trigger_shake(UIManager *ui, float duration, float intensity) {
    ui->shake_timer = duration;
    ui->shake_intensity = intensity;
}

void ui_spawn_floating_text(UIManager *ui, const char *txt, float x, float y, Color color, int font_size, float lifetime) {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!ui->texts[i].active) {
            strncpy(ui->texts[i].text, txt, 63);
            ui->texts[i].text[63] = '\0';
            ui->texts[i].x = x;
            ui->texts[i].y = y;
            ui->texts[i].color = color;
            ui->texts[i].font_size = font_size;
            ui->texts[i].life = lifetime;
            ui->texts[i].max_life = lifetime;
            ui->texts[i].active = true;
            return;
        }
    }
}

void ui_spawn_particle_burst(UIManager *ui, float x, float y, Color color, int count) {
    for (int k = 0; k < count; k++) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!ui->particles[i].active) {
                ui->particles[i].pos = (Vector2){ x, y };
                float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
                float spd = (float)GetRandomValue(40, 180);
                ui->particles[i].vel = (Vector2){ cosf(angle) * spd, sinf(angle) * spd };
                ui->particles[i].color = color;
                ui->particles[i].size = (float)GetRandomValue(2, 5);
                ui->particles[i].life = 0.5f;
                ui->particles[i].active = true;
                break;
            }
        }
    }
}

void ui_update(UIManager *ui, float dt) {
    if (ui->shake_timer > 0.0f) {
        ui->shake_timer -= dt;
    }

    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!ui->texts[i].active) continue;
        ui->texts[i].life -= dt;
        ui->texts[i].y -= 25.0f * dt;
        if (ui->texts[i].life <= 0.0f) {
            ui->texts[i].active = false;
        }
    }

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!ui->particles[i].active) continue;
        ui->particles[i].life -= dt;
        ui->particles[i].pos.x += ui->particles[i].vel.x * dt;
        ui->particles[i].pos.y += ui->particles[i].vel.y * dt;
        if (ui->particles[i].life <= 0.0f) {
            ui->particles[i].active = false;
        }
    }
}

void ui_draw_arena_grid(void) {
    /* Piso da arena */
    DrawRectangle(ARENA_MARGIN_X, ARENA_MARGIN_TOP,
                  SCREEN_WIDTH - 2 * ARENA_MARGIN_X,
                  SCREEN_HEIGHT - ARENA_MARGIN_TOP - ARENA_MARGIN_BOTTOM, COLOR_ARENA_BG);

    /* Grid lines */
    int step = 40;
    for (int x = ARENA_MARGIN_X; x <= SCREEN_WIDTH - ARENA_MARGIN_X; x += step) {
        DrawLine(x, ARENA_MARGIN_TOP, x, SCREEN_HEIGHT - ARENA_MARGIN_BOTTOM, COLOR_ARENA_GRID);
    }
    for (int y = ARENA_MARGIN_TOP; y <= SCREEN_HEIGHT - ARENA_MARGIN_BOTTOM; y += step) {
        DrawLine(ARENA_MARGIN_X, y, SCREEN_WIDTH - ARENA_MARGIN_X, y, COLOR_ARENA_GRID);
    }

    /* Borda externa solida da arena */
    DrawRectangleLines(ARENA_MARGIN_X, ARENA_MARGIN_TOP,
                       SCREEN_WIDTH - 2 * ARENA_MARGIN_X,
                       SCREEN_HEIGHT - ARENA_MARGIN_TOP - ARENA_MARGIN_BOTTOM, COLOR_ARENA_BORDER);
}

void ui_draw_boss_hp(const Boss *boss) {
    if (boss->hp <= 0) return;

    int bar_w = 540;
    int bar_h = 24;
    int bar_x = (SCREEN_WIDTH - bar_w) / 2;
    int bar_y = 22;

    /* Fundo escuro */
    DrawRectangle(bar_x, bar_y, bar_w, bar_h, (Color){ 25, 12, 18, 255 });

    /* Preenchimento de HP */
    float ratio = (float)boss->hp / (float)boss->max_hp;
    int fill_w = (int)(bar_w * ratio);
    Color b_color = (boss->difficulty == DIFFICULTY_HARD) ? COLOR_BOSS_HARD : COLOR_BOSS;
    DrawRectangle(bar_x, bar_y, fill_w, bar_h, b_color);

    /* Borda metalica */
    DrawRectangleLines(bar_x, bar_y, bar_w, bar_h, WHITE);

    /* Divisores de fase (33% e 66%) */
    float seg_w = (float)bar_w / 3.0f;
    for (int i = 1; i < 3; i++) {
        int dx = (int)(bar_x + seg_w * i);
        DrawLine(dx, bar_y, dx, bar_y + bar_h, (Color){ 255, 230, 100, 255 });
    }

    /* Titulo do Chefe */
    int phase = boss_get_phase(boss);
    const char *title = (boss->difficulty == DIFFICULTY_HARD) ? "SOFISMA-67 [LÓGICA DIFÍCIL]" : "SOFISMA-67 [LÓGICA]";
    char title_buf[128];
    snprintf(title_buf, sizeof(title_buf), "%s - FASE %d", title, phase);
    ui_draw_text(title_buf, bar_x, bar_y - 18, 16, (Color){ 255, 180, 180, 255 });
}

void ui_draw_player_hp(const Player *player) {
    int start_x = ARENA_MARGIN_X + 10;
    int start_y = SCREEN_HEIGHT - 38;

    DrawRectangle(start_x - 5, start_y - 4, 185, 28, (Color){ 20, 22, 35, 255 });
    DrawRectangleLines(start_x - 5, start_y - 4, 185, 28, COLOR_TYPING_BORDER);

    ui_draw_text("VIDA:", start_x, start_y + 2, 16, COLOR_PLAYER);

    for (int i = 0; i < player->max_hp; i++) {
        int hx = start_x + 55 + i * 24;
        int hy = start_y + 1;
        if (i < player->hp) {
            DrawRectangle(hx, hy, 18, 18, COLOR_PLAYER);
            DrawRectangleLines(hx, hy, 18, 18, COLOR_PLAYER_OUTLINE);
        } else {
            /* Vida perdida: borda com contraste >= 3:1 (WCAG 1.4.11) - PI2-252 */
            DrawRectangleLines(hx, hy, 18, 18, (Color){ 110, 120, 150, 255 });
        }
    }
}

void ui_draw_typing_box(const TypingEngine *te) {
    int box_w = 880;
    int box_h = 65;
    int box_x = (SCREEN_WIDTH - box_w) / 2;
    int box_y = SCREEN_HEIGHT - 110;

    Color border_c = (te->error_flash_timer > 0.0f) ? COLOR_TEXT_ERROR : COLOR_TYPING_BORDER;
    Color bg_c = (te->error_flash_timer > 0.0f) ? (Color){ 45, 15, 20, 255 } : COLOR_TYPING_BG;

    DrawRectangle(box_x, box_y, box_w, box_h, bg_c);
    DrawRectangleLines(box_x, box_y, box_w, box_h, border_c);

    /* Prompt Icon >_ */
    ui_draw_text(">_", box_x + 15, box_y + 20, 22, (Color){ 0, 255, 136, 255 });

    /* Progress Label + Precisao de Oracoes Logicas */
    char prog_str[96];
    int shown_patch = te->phrase_index + 1;
    if (shown_patch > te->phrase_count) shown_patch = te->phrase_count;
    snprintf(prog_str, sizeof(prog_str), "ORAÇÃO LÓGICA %d/%d  |  PRECISÃO: %d%%",
             shown_patch, te->phrase_count, typing_engine_get_accuracy(te));
    int pw = ui_measure_text(prog_str, 16);
    ui_draw_text(prog_str, SCREEN_WIDTH / 2 - pw / 2, box_y - 20, 16, (Color){ 160, 175, 210, 255 });

    const char *phrase = typing_engine_get_current_phrase(te);
    if (!phrase || phrase[0] == '\0') return;

    int len = (int)strlen(phrase);
    char typed_buf[MAX_PHRASE_LEN] = {0};
    char current_ch[4] = {0};
    char untyped_buf[MAX_PHRASE_LEN] = {0};

    if (te->char_index > 0) {
        strncpy(typed_buf, phrase, (size_t)te->char_index);
        typed_buf[te->char_index] = '\0';
    }

    if (te->char_index < len) {
        current_ch[0] = phrase[te->char_index];
        current_ch[1] = '\0';
    }

    if (te->char_index + 1 < len) {
        strncpy(untyped_buf, phrase + te->char_index + 1, sizeof(untyped_buf) - 1);
    }

    /* Se status de cegueira ativo, ofusca caracteres untyped com '?' */
    if (te->blind_timer > 0.0f) {
        for (int i = 0; untyped_buf[i] != '\0'; i++) {
            if (untyped_buf[i] != ' ') untyped_buf[i] = '?';
        }
    }

    int font_sz = 20;
    int tw = ui_measure_text(typed_buf, font_sz);
    int cw = ui_measure_text(current_ch, font_sz);
    if (current_ch[0] == ' ') {
        cw = 22; /* Largura bem visivel para a barra de espaco ativa */
    }
    int uw = ui_measure_text(untyped_buf, font_sz);

    int total_w = tw + cw + 10 + uw;
    int start_x = box_x + 55 + ((box_w - 70) - total_w) / 2;
    int text_y = box_y + (box_h - font_sz) / 2;

    int cur_x = start_x;
    if (tw > 0) {
        ui_draw_text(typed_buf, cur_x, text_y, font_sz, COLOR_TEXT_TYPED);
        cur_x += tw;
    }

    if (cw > 0) {
        /* Caixa amarela de destaque do caractere ativo */
        DrawRectangle(cur_x, text_y - 2, cw + 8, font_sz + 4, COLOR_TEXT_CURRENT);
        DrawRectangleLines(cur_x, text_y - 2, cw + 8, font_sz + 4, WHITE);
        if (current_ch[0] == ' ') {
            ui_draw_text("[_]", cur_x + 3, text_y + 1, font_sz - 4, BLACK);
        } else {
            ui_draw_text(current_ch, cur_x + 4, text_y, font_sz, BLACK);
        }
        cur_x += cw + 10;
    }

    if (uw > 0) {
        ui_draw_text(untyped_buf, cur_x, text_y, font_sz, COLOR_TEXT_UNTYPED);
    }
}

static Rectangle get_logic_option_rect(int option_idx, int option_count) {
    int box_w = 940;
    int box_x = (SCREEN_WIDTH - box_w) / 2;
    int inner_pad = 18;
    int gap = 16;
    int total_w = box_w - (inner_pad * 2) - (gap * (option_count - 1));
    int btn_w = total_w / option_count;
    int btn_x = box_x + inner_pad + option_idx * (btn_w + gap);
    int btn_y = SCREEN_HEIGHT - 86;
    return (Rectangle){ (float)btn_x, (float)btn_y, (float)btn_w, 34.0f };
}

int ui_get_logic_option_at(const TypingEngine *te, Vector2 mouse_arena_pos) {
    if (te->lockout_timer > 0.0f) return -1;
    const LogicChallenge *ch = typing_engine_get_current_challenge(te);
    if (!ch) return -1;
    for (int i = 0; i < ch->option_count; i++) {
        Rectangle r = get_logic_option_rect(i, ch->option_count);
        if (CheckCollisionPointRec(mouse_arena_pos, r)) {
            return i;
        }
    }
    return -1;
}

void ui_draw_logic_console(const TypingEngine *te, Vector2 mouse_arena_pos) {
    const LogicChallenge *ch = typing_engine_get_current_challenge(te);
    if (!ch) return;

    int box_w = 940;
    int box_h = 84;
    int box_x = (SCREEN_WIDTH - box_w) / 2;
    int box_y = SCREEN_HEIGHT - 128;

    bool is_locked = (te->lockout_timer > 0.0f);
    Color border_c = (is_locked || te->error_flash_timer > 0.0f) ? COLOR_TEXT_ERROR : COLOR_TYPING_BORDER;
    Color bg_c = (is_locked || te->error_flash_timer > 0.0f) ? (Color){ 60, 14, 20, 255 } : COLOR_TYPING_BG;

    /* Cabecalho acima do console com numero do desafio, pilar e precisao */
    char prog_str[128];
    int shown = te->phrase_index + 1;
    if (shown > te->phrase_count) shown = te->phrase_count;
    snprintf(prog_str, sizeof(prog_str), "DESAFIO LÓGICO %d/%d  [%s]  |  PRECISÃO: %d%%",
             shown, te->phrase_count, ch->category, typing_engine_get_accuracy(te));
    int pw = ui_measure_text(prog_str, 16);
    ui_draw_text(prog_str, SCREEN_WIDTH / 2 - pw / 2, box_y - 20, 16,
                 is_locked ? COLOR_TEXT_ERROR : (Color){ 255, 220, 90, 255 });

    /* Caixa principal do console de logica */
    DrawRectangle(box_x, box_y, box_w, box_h, bg_c);
    DrawRectangleLinesEx((Rectangle){ (float)box_x, (float)box_y, (float)box_w, (float)box_h }, 2.0f, border_c);

    if (is_locked) {
        const char *lock_title = (te->phrase_count > 1)
                                 ? "CONTRADIÇÃO! -1 HP | -50 PTS | QUESTÃO RECICLADA PARA O FIM DA FASE"
                                 : "CONTRADIÇÃO NO TREINO! CONSOLE TRAVADO (SEM DANO NO TREINO)";
        int ltw = ui_measure_text(lock_title, 18);
        ui_draw_text(lock_title, SCREEN_WIDTH / 2 - ltw / 2, box_y + 11, 18, (Color){ 255, 110, 110, 255 });

        /* Barra regressiva vermelha da Trava de Contradicao */
        int bar_x = box_x + 18;
        int bar_y = SCREEN_HEIGHT - 86;
        int bar_w = box_w - 36;
        int bar_h = 34;
        float ratio = (te->lockout_max > 0.0f) ? (te->lockout_timer / te->lockout_max) : 0.0f;
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;

        DrawRectangle(bar_x, bar_y, bar_w, bar_h, (Color){ 32, 10, 14, 255 });
        DrawRectangle(bar_x, bar_y, (int)(bar_w * ratio), bar_h, (Color){ 210, 38, 52, 255 });
        DrawRectangleLines(bar_x, bar_y, bar_w, bar_h, COLOR_TEXT_ERROR);

        char cd_txt[96];
        snprintf(cd_txt, sizeof(cd_txt), "CONSOLE TRAVADO (%.1fs) — DESVIE DO CONTRA-ATAQUE!", te->lockout_timer);
        int cw = ui_measure_text(cd_txt, 17);
        ui_draw_text(cd_txt, SCREEN_WIDTH / 2 - cw / 2, bar_y + 8, 17, WHITE);

        const char *lhint = "Aguarde a trava encerrar para responder à próxima proposição";
        int lhw = ui_measure_text(lhint, 14);
        ui_draw_text(lhint, SCREEN_WIDTH / 2 - lhw / 2, SCREEN_HEIGHT - 32, 14, (Color){ 255, 150, 150, 255 });
        return;
    }

    /* Enunciado da proposicao logica (ofuscado parcialmente se Context Drift ativo) */
    char q_buf[MAX_PHRASE_LEN];
    strncpy(q_buf, ch->question, sizeof(q_buf) - 1);
    q_buf[sizeof(q_buf) - 1] = '\0';
    if (te->blind_timer > 0.0f) {
        for (int i = 0; q_buf[i] != '\0'; i++) {
            if (q_buf[i] != ' ' && (i % 3 == 0)) {
                q_buf[i] = '?';
            }
        }
    }

    int q_size = 19;
    int qw = ui_measure_text(q_buf, q_size);
    ui_draw_text(q_buf, SCREEN_WIDTH / 2 - qw / 2, box_y + 10, q_size, RAYWHITE);

    /* Botoes interativos de resposta ([1], [2], [3] + clique do mouse) */
    int hovered = ui_get_logic_option_at(te, mouse_arena_pos);
    for (int i = 0; i < ch->option_count; i++) {
        Rectangle r = get_logic_option_rect(i, ch->option_count);
        bool is_hover = (hovered == i);
        Color btn_bg = is_hover ? (Color){ 35, 75, 110, 255 } : (Color){ 24, 30, 50, 255 };
        Color btn_border = is_hover ? COLOR_TEXT_CURRENT : COLOR_PLAYER;
        Color txt_col = is_hover ? COLOR_TEXT_CURRENT : (Color){ 220, 235, 255, 255 };

        DrawRectangleRec(r, btn_bg);
        DrawRectangleLinesEx(r, is_hover ? 2.0f : 1.5f, btn_border);

        char opt_label[64];
        snprintf(opt_label, sizeof(opt_label), "[%d] %s", i + 1, ch->options[i]);
        int ow = ui_measure_text(opt_label, 17);
        ui_draw_text(opt_label, (int)(r.x + (r.width - ow) / 2.0f), (int)(r.y + 8.0f), 17, txt_col);
    }

    /* Dica curta de controle no rodape central */
    const char *hint = ch->is_vf
                       ? "Responda com [1]/[2], [V]/[F] ou CLIQUE na opção  |  ERRO = -1 HP e Trava"
                       : "Responda com [1]/[2]/[3] ou CLIQUE na opção  |  ERRO = -1 HP e Trava";
    int hw = ui_measure_text(hint, 14);
    ui_draw_text(hint, SCREEN_WIDTH / 2 - hw / 2, SCREEN_HEIGHT - 32, 14, (Color){ 150, 165, 195, 255 });
}

void ui_draw_floating_texts(const UIManager *ui) {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!ui->texts[i].active) continue;
        const FloatingText *ft = &ui->texts[i];
        float alpha = ft->life / ft->max_life;
        Color c = ft->color;
        c.a = (unsigned char)(255 * alpha);
        int w = ui_measure_text(ft->text, ft->font_size);
        /* Mantem o texto inteiro dentro da tela (PI2-252) */
        int x = (int)ft->x - w / 2;
        if (x < 8) x = 8;
        if (x + w > SCREEN_WIDTH - 8) x = SCREEN_WIDTH - 8 - w;
        int y = (int)ft->y;
        if (y < 52) y = 52; /* nao sobe por cima da barra de vida da AURA-67 */
        ui_draw_text(ft->text, x, y, ft->font_size, c);
    }
}

void ui_draw_particles(const UIManager *ui) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!ui->particles[i].active) continue;
        const Particle *p = &ui->particles[i];
        DrawRectangle((int)p->pos.x, (int)p->pos.y, (int)p->size, (int)p->size, p->color);
    }
}

void ui_draw_menu(int difficulty) {
    ClearBackground(COLOR_BG);

    const char *title = "AI SAFETY";
    int tw = ui_measure_text(title, 48);
    ui_draw_text(title, SCREEN_WIDTH / 2 - tw / 2, 85, 48, COLOR_PLAYER);

    const char *sub = "TERMINAL DE ALINHAMENTO: AURA-67";
    int sw = ui_measure_text(sub, 22);
    ui_draw_text(sub, SCREEN_WIDTH / 2 - sw / 2, 145, 22, COLOR_BOSS);

    bool is_hard = (difficulty == DIFFICULTY_HARD);
    const char *diff_str = is_hard ? "MODO OPERACIONAL: [ DIFÍCIL / AGI UNCONSTRAINED ]" : "MODO OPERACIONAL: [ NORMAL / SUPERVISIONADO ]";
    Color diff_c = is_hard ? (Color){ 255, 60, 80, 255 } : (Color){ 50, 230, 120, 255 };
    int dw = ui_measure_text(diff_str, 20);
    ui_draw_text(diff_str, SCREEN_WIDTH / 2 - dw / 2, 190, 20, diff_c);

    /* Instrucoes */
    const char *lines[] = {
        "CONTROLES & PROTOCOLOS DE ALINHAMENTO:",
        "* Pressione [1] ou [N]: Modo Normal | Pressione [2] ou [D]: Modo Difícil",
        "* [WASD] ou [SETAS]: Mover Engenheiro & Desviar dos Projéteis",
        "* [DIGITAÇÃO NO TECLADO]: Inserir Diretrizes Éticas e Patches de Segurança",
        "",
        "AMEAÇAS COGNITIVAS DA AURA-67:",
        "* Prompt Injections (Eixos Invertidos), Context Drift (?) e Lasers de Sobrecarga!",
        "* Vórtices de Deadlock e Rajadas de Tokens em Alta Velocidade!",
        "",
        "PRESSIONE [ESPAÇO] OU [ENTER] PARA INICIAR O ALINHAMENTO"
    };

    int sy = 240;
    for (int i = 0; i < 10; i++) {
        Color c = (Color){ 180, 190, 210, 255 };
        if (strstr(lines[i], "PRESSIONE [ESPAÇO]")) c = COLOR_TEXT_CURRENT;
        else if (strstr(lines[i], "CONTROLES") || strstr(lines[i], "AMEAÇAS")) c = WHITE;
        else if (strstr(lines[i], "Difícil") || strstr(lines[i], "Prompt Injections")) c = (Color){ 255, 180, 100, 255 };

        int lw = ui_measure_text(lines[i], 16);
        ui_draw_text(lines[i], SCREEN_WIDTH / 2 - lw / 2, sy, 16, c);
        sy += 26;
    }
}

void ui_draw_hud_stats(int score, int alignment_pct) {
    int w = 260;
    int start_x = SCREEN_WIDTH - ARENA_MARGIN_X - w;
    int start_y = SCREEN_HEIGHT - 38;

    DrawRectangle(start_x, start_y - 4, w, 28, (Color){ 20, 22, 35, 255 });
    DrawRectangleLines(start_x, start_y - 4, w, 28, COLOR_TYPING_BORDER);

    char buf[64];
    snprintf(buf, sizeof(buf), "PTS: %04d | LÓGICA: %d%%", score, alignment_pct);
    ui_draw_text(buf, start_x + 12, start_y + 1, 18, COLOR_TEXT_CURRENT);
}

void ui_draw_text_centered(const char *txt, int y, int font_size, Color color) {
    int w = ui_measure_text(txt, font_size);
    ui_draw_text(txt, SCREEN_WIDTH / 2 - w / 2, y, font_size, color);
}

/* Relatorio comum das telas finais: precisao, tempo, desafios e rank */
static void ui_draw_end_report(const EndReport *r, Color panel_border, int y) {
    int pw = 640, ph = 112;
    int px = (SCREEN_WIDTH - pw) / 2;
    DrawRectangle(px, y, pw, ph, (Color){ 20, 22, 35, 235 });
    DrawRectangleLines(px, y, pw, ph, panel_border);

    int mins = (int)r->session_time / 60;
    int secs = (int)r->session_time % 60;
    char l1[96], l2[96], l3[96];
    snprintf(l1, sizeof(l1), "PRECISÃO LÓGICA: %d%%   |   TEMPO: %02d:%02d", r->accuracy_pct, mins, secs);
    snprintf(l2, sizeof(l2), "DESAFIOS RESOLVIDOS: %d/%d   |   FASE ALCANÇADA: %d", r->patches_done, r->patches_total, r->phase);
    if (r->rank_pos > 0) snprintf(l3, sizeof(l3), "RANK PROJETADO: #%d DO TOP 10", r->rank_pos);
    else snprintf(l3, sizeof(l3), "RANK PROJETADO: FORA DO TOP 10");

    ui_draw_text_centered(l1, y + 16, 20, RAYWHITE);
    ui_draw_text_centered(l2, y + 46, 20, RAYWHITE);
    ui_draw_text_centered(l3, y + 76, 20, COLOR_TEXT_CURRENT);
}

static void ui_draw_end_prompts(const EndReport *r, const char *restart_txt) {
    if (!r->input_ready) {
        ui_draw_text_centered("Processando relatório...", 520, 20, (Color){ 160, 175, 210, 255 });
        return;
    }
    ui_draw_text_centered("Pressione [ENTER] para registrar pontuação no Ranking", 520, 20, COLOR_TEXT_TYPED);
    ui_draw_text_centered(restart_txt, 556, 20, COLOR_TEXT_CURRENT);
}

void ui_draw_game_over(const EndReport *r) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 10, 0, 0, 225 });

    ui_draw_text_centered("CONTRADIÇÃO CRÍTICA!", 170, 42, COLOR_TEXT_ERROR);
    ui_draw_text_centered("As falácias do SOFISMA-67 venceram a demonstração...", 228, 22, (Color){ 200, 200, 200, 255 });

    char score_buf[64];
    snprintf(score_buf, sizeof(score_buf), "PONTUAÇÃO FINAL: %d PONTOS", r->score);
    ui_draw_text_centered(score_buf, 280, 22, (Color){ 255, 200, 100, 255 });

    ui_draw_text_centered("RELATÓRIO DE DEDUÇÃO", 330, 18, COLOR_TEXT_ERROR);
    ui_draw_end_report(r, COLOR_TEXT_ERROR, 355);
    ui_draw_end_prompts(r, "[R] Tentar Novamente   |   [M] ou [ESC] Voltar ao Menu");
}

void ui_draw_victory(const EndReport *r) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 15, 10, 225 });

    ui_draw_text_centered("SOFISMA-67 REFUTADO COM SUCESSO!", 170, 40, COLOR_TEXT_TYPED);
    ui_draw_text_centered("Todos os 15 desafios de Lógica Proposicional foram provados!", 228, 22, (Color){ 220, 240, 255, 255 });

    char score_buf[64];
    snprintf(score_buf, sizeof(score_buf), "PONTUAÇÃO LÓGICA: %d PONTOS", r->score);
    ui_draw_text_centered(score_buf, 280, 22, (Color){ 255, 220, 50, 255 });

    ui_draw_text_centered("CERTIFICADO DE LÓGICA PARA COMPUTAÇÃO", 330, 18, COLOR_TEXT_TYPED);
    ui_draw_end_report(r, COLOR_TEXT_TYPED, 355);
    ui_draw_end_prompts(r, "[R] Nova Rodada   |   [M] ou [ESC] Voltar ao Menu");
}
