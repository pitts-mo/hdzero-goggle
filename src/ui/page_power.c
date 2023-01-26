#include "page_power.h"

#include <stdio.h>

#include <log/log.h>
#include <minIni.h>

#include "core/common.hh"
#include "mcp3021.h"
#include "page_common.h"
#include "ui/ui_style.h"

static slider_group_t slider_group_c_voltage;
static btn_group_t btn_group_s_count_mode;
static slider_group_t slider_group_s_count;
static btn_group_t btn_group_warn_type;

static lv_coord_t col_dsc[] = {160, 200, 160, 160, 120, 160, LV_GRID_TEMPLATE_LAST};
static lv_coord_t row_dsc[] = {60, 60, 60, 60, 60, 60, 60, 60, 60, 60, LV_GRID_TEMPLATE_LAST};
lv_obj_t *label_cell_count;

static lv_obj_t *page_power_create(lv_obj_t *parent, panel_arr_t *arr) {
    lv_obj_t *page = lv_menu_page_create(parent, NULL);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(page, 1053, 900);
    lv_obj_add_style(page, &style_subpage, LV_PART_MAIN);
    lv_obj_set_style_pad_top(page, 94, 0);

    lv_obj_t *section = lv_menu_section_create(page);
    lv_obj_add_style(section, &style_submenu, LV_PART_MAIN);
    lv_obj_set_size(section, 1053, 894);

    create_text(NULL, section, false, "Power:", LV_MENU_ITEM_BUILDER_VARIANT_2);

    lv_obj_t *cont = lv_obj_create(section);
    lv_obj_set_size(cont, 960, 600);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(cont, &style_context, LV_PART_MAIN);

    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);

    create_select_item(arr, cont);

    // create menu entries
    create_label_item(cont, "Battery", 1, ROW_BATT_C_LABEL, 1);
    label_cell_count = create_label_item(cont, "-S", 2, ROW_BATT_C_LABEL, 1);
    create_btn_group_item(&btn_group_s_count_mode, cont, 2, "Cell Count Mode", "Auto", "Manual", "", "", ROW_S_COUNT_MODE);
    create_slider_item(&slider_group_s_count, cont, "Cell Count", S_MAX_COUNT, g_setting.power.s_count, ROW_S_COUNT);
    create_slider_item(&slider_group_c_voltage, cont, "Cell Voltage", CELL_VOLTAGE_MAX, g_setting.power.voltage, ROW_CELL_VOLTAGE);
    create_btn_group_item(&btn_group_warn_type, cont, 3, "Warning Type", "Beep", "Visual", "Both", "", ROW_WARN_TYPE);
    create_label_item(cont, "< Back", 1, ROW_BACK, 1);

    // set menu entry min/max values and labels
    char str[5];
    sprintf(str, "%d.%d", g_setting.power.voltage / 10, g_setting.power.voltage % 10);
    lv_slider_set_range(slider_group_c_voltage.slider, CELL_VOLTAGE_MIN, CELL_VOLTAGE_MAX);
    lv_label_set_text(slider_group_c_voltage.label, str);

    sprintf(str, "%d", g_setting.power.s_count);
    lv_slider_set_range(slider_group_s_count.slider, S_MIN_COUNT, S_MAX_COUNT);
    lv_label_set_text(slider_group_s_count.label, str);

    // set menu entry current values, loaded from stored settings
    btn_group_set_sel(&btn_group_s_count_mode, g_setting.power.s_count_mode);
    lv_slider_set_value(slider_group_s_count.slider, g_setting.power.s_count, LV_ANIM_OFF);
    lv_slider_set_value(slider_group_c_voltage.slider, g_setting.power.voltage, LV_ANIM_OFF);
    btn_group_set_sel(&btn_group_warn_type, g_setting.power.warning_type);
    set_battery_S();

    return page;
}

void set_battery_S() {
    char str[10];

    switch (g_setting.power.s_count_mode)
    {
    default:
    case 0: // auto
        g_battery.type = mcp_detect_type();
        break;
    case 1: // manual
        g_battery.type = g_setting.power.s_count;
        break;
    }

    sprintf(str, "%dS", g_battery.type);
    lv_label_set_text(label_cell_count, str);
}

void power_s_count_inc(void) {
    int32_t value = 0;

    value = lv_slider_get_value(slider_group_s_count.slider);
    if (value < S_MAX_COUNT)
        value += 1;

    lv_slider_set_value(slider_group_s_count.slider, value, LV_ANIM_OFF);

    char buf[5];
    sprintf(buf, "%d", value);
    lv_label_set_text(slider_group_s_count.label, buf);

    g_setting.power.s_count = value;
    LOGI("s_count:%d", g_setting.power.s_count);
    ini_putl("power", "s_count", g_setting.power.s_count, SETTING_INI);
    set_battery_S();
}

void power_s_count_dec(void) {
    int32_t value = 0;

    value = lv_slider_get_value(slider_group_s_count.slider);
    if (value > S_MIN_COUNT)
        value -= 1;

    lv_slider_set_value(slider_group_s_count.slider, value, LV_ANIM_OFF);

    char buf[5];
    sprintf(buf, "%d", value);
    lv_label_set_text(slider_group_s_count.label, buf);

    g_setting.power.s_count = value;
    LOGI("s_count:%d", g_setting.power.s_count);
    ini_putl("power", "s_count", g_setting.power.s_count, SETTING_INI);
    set_battery_S();
}

void power_voltage_inc(void) {
    int32_t value = 0;

    value = lv_slider_get_value(slider_group_c_voltage.slider);
    if (value < CELL_VOLTAGE_MAX)
        value += 1;

    lv_slider_set_value(slider_group_c_voltage.slider, value, LV_ANIM_OFF);

    char buf[5];
    sprintf(buf, "%d.%d", value / 10, value % 10);
    lv_label_set_text(slider_group_c_voltage.label, buf);

    g_setting.power.voltage = value;
    LOGI("vol:%d", g_setting.power.voltage);
    ini_putl("power", "voltage", g_setting.power.voltage, SETTING_INI);
}
void power_voltage_dec(void) {
    int32_t value = 0;

    value = lv_slider_get_value(slider_group_c_voltage.slider);
    if (value > CELL_VOLTAGE_MIN)
        value -= 1;

    lv_slider_set_value(slider_group_c_voltage.slider, value, LV_ANIM_OFF);
    char buf[5];
    sprintf(buf, "%d.%d", value / 10, value % 10);
    lv_label_set_text(slider_group_c_voltage.label, buf);

    g_setting.power.voltage = value;
    LOGI("vol:%d", g_setting.power.voltage);
    ini_putl("power", "voltage", g_setting.power.voltage, SETTING_INI);
}

static void page_power_on_click(uint8_t key, int sel) {

    switch (sel) {

    case ROW_S_COUNT_MODE:
        btn_group_toggle_sel(&btn_group_s_count_mode);
        g_setting.power.s_count_mode = btn_group_get_sel(&btn_group_s_count_mode);
        ini_putl("power", "s_count_mode", g_setting.power.s_count_mode, SETTING_INI);
        set_battery_S();
        break;

    case ROW_S_COUNT:
        if (g_menu_op == PAGE_POWER_SLIDE_S_COUNT) {
            g_menu_op = OPLEVEL_SUBMENU;
            lv_obj_add_style(slider_group_s_count.slider, &style_silder_main, LV_PART_MAIN);
        } else {
            g_menu_op = PAGE_POWER_SLIDE_S_COUNT;
            lv_obj_add_style(slider_group_s_count.slider, &style_silder_select, LV_PART_MAIN);
        }
        break;

    case ROW_CELL_VOLTAGE:
        if (g_menu_op == PAGE_POWER_SLIDE_CELL_VOLTAGE) {
            g_menu_op = OPLEVEL_SUBMENU;
            lv_obj_add_style(slider_group_c_voltage.slider, &style_silder_main, LV_PART_MAIN);
        } else {
            g_menu_op = PAGE_POWER_SLIDE_CELL_VOLTAGE;
            lv_obj_add_style(slider_group_c_voltage.slider, &style_silder_select, LV_PART_MAIN);
        }
        break;
    
    case ROW_WARN_TYPE:
        btn_group_toggle_sel(&btn_group_warn_type);
        g_setting.power.warning_type = btn_group_get_sel(&btn_group_warn_type);
        ini_putl("power", "warning_type", g_setting.power.warning_type, SETTING_INI);
        break;

    default:
        break;
    }
}

page_pack_t pp_power = {
    .p_arr = {
        .cur = 0,
        .max = 4,
    },

    .create = page_power_create,
    .enter = NULL,
    .exit = NULL,
    .on_roller = NULL,
    .on_click = page_power_on_click,
    .on_right_button = NULL,
};