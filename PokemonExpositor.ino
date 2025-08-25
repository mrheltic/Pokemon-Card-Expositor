/*
 * Pokemon Image Viewer per ESP32-S3 con LVGL v9
 * Visualizza immagine PNG convertita in ARGB8888
 */

#include <Arduino.h>
#include <lvgl.h>
#include "pokemon_image.h"

// Configurazioni display
#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   600

// Buffer LVGL
static lv_color_t *buf1;
static lv_color_t *buf2;
static lv_display_t *display;

// Oggetti UI
static lv_obj_t *img_pokemon;
static lv_obj_t *label_info;

// Funzioni
void setup_lvgl();
void create_ui();
void display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void lv_tick_task(void *arg);

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("================================");
    Serial.println("Pokemon Image Viewer - LVGL v9");
    Serial.println("================================");
    
    // Informazioni memoria
    Serial.printf("PSRAM: %d bytes disponibili\n", ESP.getFreePsram());
    Serial.printf("Heap: %d bytes disponibili\n", ESP.getFreeHeap());
    
    // Informazioni immagine
    Serial.printf("Immagine: %dx%d pixels\n", POKEMON_IMAGE_WIDTH, POKEMON_IMAGE_HEIGHT);
    Serial.printf("Dimensioni dati: %d bytes\n", sizeof(pokemon_image_data));
    
    // Inizializza LVGL
    setup_lvgl();
    
    // Crea interfaccia
    create_ui();
    
    Serial.println("Setup completato!");
}

void loop() {
    lv_timer_handler();
    delay(5);
}

void setup_lvgl() {
    Serial.println("Inizializzazione LVGL v9...");
    
    // Inizializza LVGL
    lv_init();
    
    // Alloca buffer per display (usando PSRAM se disponibile)
    size_t buffer_size = SCREEN_WIDTH * 40; // 40 linee
    
    if (ESP.getFreePsram() > 0) {
        buf1 = (lv_color_t*)ps_malloc(buffer_size * sizeof(lv_color_t));
        buf2 = (lv_color_t*)ps_malloc(buffer_size * sizeof(lv_color_t));
        Serial.println("Buffer allocati in PSRAM");
    } else {
        buf1 = (lv_color_t*)malloc(buffer_size * sizeof(lv_color_t));
        buf2 = (lv_color_t*)malloc(buffer_size * sizeof(lv_color_t));
        Serial.println("Buffer allocati in heap normale");
    }
    
    if (!buf1 || !buf2) {
        Serial.println("ERRORE: Impossibile allocare buffer display!");
        while(1) delay(1000);
    }
    
    // Crea display
    display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(display, display_flush_cb);
    lv_display_set_buffers(display, buf1, buf2, buffer_size * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    // Timer per LVGL
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 5 * 1000)); // 5ms
    
    Serial.println("LVGL inizializzato con successo");
}

void create_ui() {
    Serial.println("Creazione interfaccia utente...");
    
    // Schermo principale
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    
    // Container per immagine centrata
    lv_obj_t *img_container = lv_obj_create(scr);
    lv_obj_set_size(img_container, SCREEN_WIDTH, SCREEN_HEIGHT - 60);
    lv_obj_align(img_container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(img_container, lv_color_black(), 0);
    lv_obj_set_style_border_width(img_container, 0, 0);
    lv_obj_set_style_pad_all(img_container, 0, 0);
    lv_obj_clear_flag(img_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // Oggetto immagine Pokemon
    img_pokemon = lv_image_create(img_container);
    lv_image_set_src(img_pokemon, &pokemon_image_dsc);
    lv_obj_center(img_pokemon);
    
    // Calcola zoom per adattare al display
    float scale_x = (float)SCREEN_WIDTH / POKEMON_IMAGE_WIDTH;
    float scale_y = (float)(SCREEN_HEIGHT - 60) / POKEMON_IMAGE_HEIGHT;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;
    
    // Applica zoom se necessario (max 100% per mantenere qualità)
    if (scale < 1.0f) {
        int zoom = (int)(scale * 256); // LVGL zoom: 256 = 100%
        lv_image_set_scale(img_pokemon, zoom);
        Serial.printf("Immagine scalata al %d%% per adattarsi al display\n", (int)(scale * 100));
    }
    
    // Label informazioni
    label_info = lv_label_create(scr);
    lv_label_set_text_fmt(label_info, "Pokemon Card %dx%d ARGB8888", 
                          POKEMON_IMAGE_WIDTH, POKEMON_IMAGE_HEIGHT);
    lv_obj_align(label_info, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_color(label_info, lv_color_white(), 0);
    lv_obj_set_style_bg_color(label_info, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label_info, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_pad_all(label_info, 5, LV_PART_MAIN);
    
    Serial.println("Interfaccia creata con successo");
}

// Callback flush display (simulato per test)
void display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    // In un'implementazione reale qui invieresti i dati al display hardware
    // Per ora simuliamo semplicemente
    lv_display_flush_ready(disp);
}

// Timer tick per LVGL
void lv_tick_task(void *arg) {
    lv_tick_inc(5);
}
