/**
 * @author Le Duy Nhat
 * @edited Nguyen Doan Khanh
 * @date 2022-12
 * @brief 
 */
#pragma once
#include <TFT_eSPI.h>
//#include "Config.h"
#include <lvgl.h>
#define _LVGV_IN_USE_
#include <WiFi.h>
#include <vector>

enum SCREEN_ID {PROGRESS_SCREEN, 
    MAIN_SCREEN, 
    MENU_SCREEN, 
    SETUP_SCREEN, 
    WIFI_SCREEN, 
    CONFIG_SCREEN};

struct ILI9341Status
{
    SCREEN_ID current_screen;
    SCREEN_ID previous_screen;
    uint8_t current_pointer_position;
    uint8_t previous_pointer_position;
};
class ILI9341Display
{
private: 
    // TFT_eSPI tft = TFT_eSPI();
    ILI9341Status status;
    // DeviceManager *devices = DeviceManager::getInstance();
    // DataCore data = DeviceManager::getDataCore();
public:
    void init();
    void progressScreen(bool _sd = false, bool _pms7003 = false, bool _bme280 = false, bool _ds3231 = false, bool _sds011 = false, bool _htu21d = false, bool _sht85 = false);
    void guiHandler();
    // void mainScreen(lv_obj_t * scr);
    void showScreen(lv_obj_t * scr);
    void updateData(float _temp, float _humi, float _pres, float _pm2p5/*, char* _mac*/);
    void updateClock(uint8_t _hour, uint8_t _min);
    // void del_obj1();
    // void del_obj2();
    void showSDsymbol();
    void showNonSDsymbol(lv_obj_t * scr);
    //void showWiFiSymbol(lv_obj_t * scr);
    void showWiFiWaitSymbol();
    //void showNonWiFiSymbol(lv_obj_t * scr);
    void showWiFiConfigSymbol();
    void showWiFiButNoInternetSymbol();
    // void setStyle();
    // void makeKeyboard(lv_obj_t * scr);
    // void buildStatusBar(lv_obj_t * scr);
    // void buildPWMsgBox(lv_obj_t * scr);
    // void buildBody(lv_obj_t * scr);
    // void buildSettings(lv_obj_t * scr);
    // void wifiScreen(lv_obj_t * scr);
    ILI9341Status getStatus();
    void setDisplayScreen(SCREEN_ID _screen);
    void setStatus();
    ILI9341Display();
    ~ILI9341Display();

    void lv_example_style_3_class();
    //void show_wifidisplay_class();

};

