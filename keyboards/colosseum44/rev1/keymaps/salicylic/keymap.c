#include QMK_KEYBOARD_H
#include "bootloader.h"
#ifdef PROTOCOL_LUFA
#include "lufa.h"
#include "split_util.h"
#endif
#ifdef AUDIO_ENABLE
  #include "audio.h"
#endif

extern keymap_config_t keymap_config;

#ifdef RGBLIGHT_ENABLE
//Following line allows macro to read current RGB settings
extern rgblight_config_t rgblight_config;
#endif

extern uint8_t is_master;

// Each layer gets a name for readability, which is then used in the keymap matrix below.
// The underscores don't mean anything - you can have a layer called STUFF or any other name.
// Layer names don't all need to be of the same length, obviously, and you can also skip them
// entirely and just use numbers.
#define _QWERTY 0
#define _LOWER 3
#define _RAISE 4
#define _ADJUST 16

enum custom_keycodes {
  QWERTY = SAFE_RANGE,
  LOWER,
  RAISE,
  ADJUST,
  RGBRST,
  KC_00
};

enum macro_keycodes {
  KC_SAMPLEMACRO,
};


// Fillers to make layering more clear
#define _______ KC_TRNS
#define XXXXXXX KC_NO
#define KC_LOWER LOWER
#define KC_RAISE RAISE

#define JP_CLON KC_QUOT  // : and +
#define JP_AT   KC_LBRC  // @ and `
#define JP_HAT  KC_EQL   // ^ and ~
#define JP_ENUN KC_RO    // \ and _ (EN mark and UNder score)
#define JP_ENVL KC_JYEN  // \ and | (EN mark and Vertical Line)
#define JP_LBRC KC_RBRC  // [ and {
#define JP_RBRC KC_BSLS  // ] and }
#define JP_AMPR KC_CIRC  // &
#define JP_QUES LSFT(KC_SLSH)  // ?
#define JP_TILD LSFT(KC_EQL)  // ~
#define JP_QUOT LSFT(KC_7)  // '
#define JP_LPRN KC_ASTR  // (
#define JP_RPRN KC_LPRN  // )
#define JP_LCBR KC_RCBR  // {
#define JP_RCBR KC_PIPE  // }
#define JP_PIPE LSFT(KC_JYEN)  // |
#define JP_ASTR LSFT(KC_QUOT)  // *
#define JP_EQL LSFT(KC_MINS)  // =
#define JP_PLUS LSFT(KC_SCLN)  // +
#define JP_DQUO LSFT(KC_2)  // "


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

   /* Qwerty
     */
  [_QWERTY] = LAYOUT(
     KC_TAB, KC_Q,    KC_W,     KC_E,   KC_R,    KC_T,                    KC_Y,   KC_U,     KC_I,    KC_O,    KC_P,  KC_GRV,
    KC_LCTL, KC_A,    KC_S,     KC_D,   KC_F,    KC_G,                    KC_H,   KC_J,     KC_K,    KC_L, KC_SCLN, KC_QUOT,
    KC_LSFT, KC_Z,    KC_X,     KC_C,   KC_V,    KC_B,                    KC_N,   KC_M,  KC_COMM,  KC_DOT, KC_SLSH, KC_BSLS,
                    KC_LOWER, KC_SPC, KC_BSPC, KC_LGUI,                  KC_LALT, KC_DEL, KC_ENT, KC_RAISE
  ),
    /* Lower
     */
  [_LOWER] = LAYOUT(
 KC_ESC, KC_EXLM,   KC_AT, KC_HASH,  KC_DLR, KC_PERC,                   KC_CIRC, KC_AMPR,   KC_ASTR, KC_LPRN, KC_RPRN, KC_BSPC,
  KC_LCTL,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,                   KC_MINS,  KC_EQL,   KC_LCBR, KC_RCBR, KC_PIPE,  KC_GRV,
  KC_LSFT,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,                   KC_UNDS, KC_PLUS,   KC_LBRC, KC_RBRC, KC_BSLS, KC_TILD,
                  KC_LOWER, KC_TRNS, KC_TRNS, KC_TRNS,                   KC_TRNS, KC_TRNS, KC_TRNS, KC_RAISE
  ),
    /* Raise
     */

  [_RAISE] = LAYOUT(
   KC_ESC,  KC_1,    KC_2,      KC_3,    KC_4,    KC_5,                    KC_6,    KC_7,     KC_8,    KC_9,    KC_0, KC_BSPC,
  KC_LCTL, KC_F1,   KC_F2,     KC_F3,   KC_F4,   KC_F5,                 KC_LEFT, KC_DOWN,    KC_UP, KC_RGHT,   KC_NO,   KC_NO,
  KC_LSFT, KC_F6,   KC_F7,     KC_F8,   KC_F9,  KC_F10,                  KC_F11,  KC_F12,  KC_COMM,  KC_DOT, KC_SLSH, KC_BSLS,
                  KC_LOWER, KC_TRNS, KC_TRNS, KC_TRNS,                 KC_TRNS, KC_TRNS, KC_TRNS, KC_RAISE
  ),

    /* Adjust
     */

  [_ADJUST] = LAYOUT(
    RGB_TOG,   KC_NO,   KC_NO,    KC_NO,   KC_NO,   KC_NO,              KC_WH_L, KC_WH_D,  KC_WH_U, KC_WH_R,   KC_NO,  KC_DEL,
    RGB_MOD, RGB_HUI, RGB_SAI,  RGB_VAI, RGB_SPI, KC_VOLU,              KC_MS_L, KC_MS_D,  KC_MS_U, KC_MS_R, KC_BTN1, KC_BTN2,
   RGB_RMOD, RGB_HUD, RGB_SAD,  RGB_VAD, RGB_SPD, KC_VOLD,              KC_ACL0, KC_ACL1,  KC_ACL2,   KC_NO,   KC_NO,   KC_NO,
                      KC_LOWER, KC_TRNS, KC_TRNS, KC_TRNS,              KC_TRNS, KC_TRNS, KC_TRNS, KC_RAISE
          )
};

// define variables for reactive RGB
bool TOG_STATUS = false;
int RGB_current_mode;

void persistent_default_layer_set(uint16_t default_layer) {
  eeconfig_update_default_layer(default_layer);
  default_layer_set(default_layer);
}

// Setting ADJUST layer RGB back to default
void update_tri_layer_RGB(uint8_t layer1, uint8_t layer2, uint8_t layer3) {
  if (IS_LAYER_ON(layer1) && IS_LAYER_ON(layer2)) {
    #ifdef RGBLIGHT_ENABLE
      //rgblight_mode(RGB_current_mode);
    #endif
    layer_on(layer3);
  } else {
    layer_off(layer3);
  }
}


bool process_record_user(uint16_t keycode, keyrecord_t *record) {

  switch (keycode) {
    case QWERTY:
      if (record->event.pressed) {
        persistent_default_layer_set(1UL<<_QWERTY);
      }
      return false;
      break;
    case LOWER:
      if (record->event.pressed) {
        layer_on(_LOWER);
        update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
      } else {
        layer_off(_LOWER);
        update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
      }
      return false;
      break;
    case RAISE:
      if (record->event.pressed) {
        layer_on(_RAISE);
        update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
      } else {
        layer_off(_RAISE);
        update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
      }
      return false;
      break;
    case ADJUST:
        if (record->event.pressed) {
          layer_on(_ADJUST);
        } else {
          layer_off(_ADJUST);
        }
        return false;
        break;
      //led operations - RGB mode change now updates the RGB_current_mode to allow the right RGB mode to be set after reactive keys are released
    case RGB_MOD:
      #ifdef RGBLIGHT_ENABLE
        if (record->event.pressed) {
          rgblight_mode(RGB_current_mode);
          rgblight_step();
          RGB_current_mode = rgblight_config.mode;
        }
      #endif
      return false;
      break;
    case RGBRST:
      #ifdef RGBLIGHT_ENABLE
        if (record->event.pressed) {
          eeconfig_update_rgblight_default();
          rgblight_enable();
          rgblight_mode(14);
        }
      #endif
      break;
    case KC_00:
      if (record->event.pressed) {
        SEND_STRING("00");
      }
      return false;
      break;
  }
  return true;
}

void matrix_init_user(void) {
    #ifdef AUDIO_ENABLE
        startup_user();
    #endif
    #ifdef RGBLIGHT_ENABLE
      RGB_current_mode = rgblight_config.mode;
    #endif
}
