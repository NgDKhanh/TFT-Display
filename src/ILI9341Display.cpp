#include "..\include\ILI9341Display.h"


// #define _LVGV_IN_USE_
#ifdef _LVGV_IN_USE_

#else
// #include "Graphic/Free_Fonts.h"
// #include "Graphic/Logo_bitmaps.h"
//#include "JPEGDecoder.h"
//#include "SPIFFS.h"

#define FORMAT_SPIFFS_IF_FAILED true

#endif
#define _DB_LOG_

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

TFT_eSPI tft = TFT_eSPI(screenHeight, screenWidth); /* TFT instance */

static lv_timer_t *timer1;
void my_timer_cb(lv_timer_t * tm);
float tempe;
float presse;
float hum;
float pm2p5e;
lv_obj_t *data;

/**************************WIFI CONFIG**************************************/

typedef enum {
  NONE,
  NETWORK_SEARCHING,
  NETWORK_CONNECTED_POPUP,
  NETWORK_CONNECTED,
  NETWORK_CONNECT_FAILED
} Network_Status_t;
Network_Status_t networkStatus = NONE;

/*Change to your screen resolution*/
static lv_style_t label_style;
static lv_obj_t *headerLabel;

// static lv_style_t border_style;
// static lv_style_t popupBox_style;
// static lv_obj_t *timeLabel;
// static lv_obj_t *settings;
// static lv_obj_t *settingBtn;
// static lv_obj_t *settingCloseBtn;
// static lv_obj_t *settingWiFiSwitch;
// static lv_obj_t *wfList;
// static lv_obj_t *settinglabel;
// static lv_obj_t *mboxConnect;
// static lv_obj_t *mboxTitle;
// static lv_obj_t *mboxPassword;
// static lv_obj_t *mboxConnectBtn;
// static lv_obj_t *mboxCloseBtn;
// static lv_obj_t *keyboard;
// static lv_obj_t *popupBox;
// static lv_obj_t *popupBoxCloseBtn;
// static lv_obj_t *bodyScreen;
// static lv_timer_t *timer;

 lv_style_t border_style;
 lv_style_t popupBox_style;
 lv_obj_t *timeLabel;
 lv_obj_t *settings;
 lv_obj_t *settingBtn;
 lv_obj_t *settingCloseBtn;
 lv_obj_t *settingWiFiSwitch;
 lv_obj_t *wfList;
 lv_obj_t *settinglabel;
 lv_obj_t *mboxConnect;
 lv_obj_t *mboxTitle;
 lv_obj_t *mboxPassword;
 lv_obj_t *mboxConnectBtn;
 lv_obj_t *mboxCloseBtn;
 lv_obj_t *keyboard;
 //lv_obj_t *popupBox;
 //lv_obj_t *popupBoxCloseBtn;
 lv_obj_t *bodyScreen;
 lv_timer_t *timer;

 lv_style_t background;


// static int foundNetworks = 0;
int foundNetworks = 0;
unsigned long networkTimeout = 10 * 1000;
String ssidName, ssidPW;
String cur_ssidName;

TaskHandle_t ntScanTaskHandler, ntConnectTaskHandler;
std::vector<String> foundWifiList;


/******************2 screen ****************/

lv_obj_t * scr1;
lv_obj_t * scr2;

/*****************************/
static void btn_event_cb(lv_event_t *e);
static void networkConnector();
static void networkScanner();
static void scanWIFITask(void *pvParameters);
static void beginWIFITask(void *pvParameters);
static void list_event_handler(lv_event_t *e);
static void text_input_event_cb(lv_event_t *e);

static void timerForNetwork(lv_timer_t *timer);
void updateLocalTime(String wifiName);

lv_obj_t * popupMsgBox(String title, String msg, lv_obj_t * scr);

void setStyle() {
  lv_style_init(&border_style);
  lv_style_set_border_width(&border_style, 2);
  lv_style_set_border_color(&border_style, lv_color_black());

  lv_style_init(&popupBox_style);
  lv_style_set_radius(&popupBox_style, 10);
  lv_style_set_bg_opa(&popupBox_style, LV_OPA_COVER);
  lv_style_set_border_color(&popupBox_style, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_border_width(&popupBox_style, 5);
}

void buildStatusBar(lv_obj_t * scr) {

  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, lv_color_hex(0xC5C5C5));
  lv_style_set_bg_opa(&style_btn, LV_OPA_50);

  lv_obj_t *statusBar = lv_obj_create(scr);
  lv_obj_set_size(statusBar, tft.width() - 40, 30);
  lv_obj_align(statusBar, LV_ALIGN_TOP_RIGHT, 0, 0);

  lv_obj_remove_style(statusBar, NULL, LV_PART_SCROLLBAR | LV_STATE_ANY);

  timeLabel = lv_label_create(statusBar);
  lv_obj_set_size(timeLabel, tft.width() - 50, 30);

  if(WiFi.status() == WL_CONNECTED){
    String hourMinWithSymbol = LV_SYMBOL_WIFI;
    hourMinWithSymbol += "  ";
    hourMinWithSymbol += cur_ssidName;
    lv_label_set_text(timeLabel, hourMinWithSymbol.c_str());
  }
  else
    lv_label_set_text(timeLabel, "WiFi Not Connected!    " LV_SYMBOL_CLOSE);
  lv_obj_align(timeLabel, LV_ALIGN_LEFT_MID, 8, 4);

  settingBtn = lv_btn_create(statusBar);
  lv_obj_set_size(settingBtn, 30, 30);
  lv_obj_align(settingBtn, LV_ALIGN_RIGHT_MID, 0, 0);

  lv_obj_add_event_cb(settingBtn, btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *label = lv_label_create(settingBtn); /*Add a label to the button*/
  lv_label_set_text(label, LV_SYMBOL_SETTINGS);  /*Set the labels text*/
  lv_obj_center(label);
}

static void btn_popup_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
      lv_obj_move_background(lv_obj_get_parent(btn));
  }
}

static void btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (btn == settingBtn) {
      lv_obj_clear_flag(settings, LV_OBJ_FLAG_HIDDEN);
    } else if (btn == settingCloseBtn) {
      lv_obj_add_flag(settings, LV_OBJ_FLAG_HIDDEN);
    } else if (btn == mboxConnectBtn) {
      ssidPW = String(lv_textarea_get_text(mboxPassword));

      networkConnector();
      lv_obj_move_background(mboxConnect);
      //popupMsgBox("Connecting!", "Attempting to connect to the selected network.", scr2);
    } else if (btn == mboxCloseBtn) {
      lv_obj_move_background(mboxConnect);
    } /*else if (btn == popupBoxCloseBtn) {
      lv_obj_move_background(lv_obj_get_parent(btn));
    }*/

  } else if (code == LV_EVENT_VALUE_CHANGED) {
    if (btn == settingWiFiSwitch) {

      if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {

        if (ntScanTaskHandler == NULL) {
          networkStatus = NETWORK_SEARCHING;
          networkScanner();
          timer = lv_timer_create(timerForNetwork, 1000, wfList);
          lv_list_add_text(wfList, "WiFi: Looking for Networks...");
        }

      } else {

        if (ntScanTaskHandler != NULL) {
          networkStatus = NONE;
          vTaskDelete(ntScanTaskHandler);
          ntScanTaskHandler = NULL;
          lv_timer_del(timer);
          lv_obj_clean(wfList);
        }

        // if (WiFi.status() == WL_CONNECTED) {
        //   WiFi.disconnect(true);
        //   lv_label_set_text(timeLabel, "WiFi Not Connected!    " LV_SYMBOL_CLOSE);
        // }
      }
    }
  }
}

static void showingFoundWiFiList() {
  if (foundWifiList.size() == 0 || foundNetworks == foundWifiList.size())
    return;

  lv_obj_clean(wfList);
  lv_list_add_text(wfList, foundWifiList.size() > 1 ? "WiFi: Found Networks" : "WiFi: Not Found!");

  for (std::vector<String>::iterator item = foundWifiList.begin(); item != foundWifiList.end(); ++item) {
    lv_obj_t *btn = lv_list_add_btn(wfList, LV_SYMBOL_WIFI, (*item).c_str());
    lv_obj_add_event_cb(btn, list_event_handler, LV_EVENT_CLICKED, NULL);
    delay(1);
  }

  foundNetworks = foundWifiList.size();
}

void buildBody(lv_obj_t * scr) {
  bodyScreen = lv_obj_create(scr);
  lv_obj_add_style(bodyScreen, &border_style, 0);
  lv_obj_set_size(bodyScreen, tft.width(), tft.height() - 34);
  lv_obj_align(bodyScreen, LV_ALIGN_BOTTOM_MID, 0, 0);

  // lv_obj_t *label = lv_label_create(bodyScreen);
  // lv_label_set_text(label, "Please click the setting button\n to setup your wifi!");
  // lv_obj_center(label);

  wfList = lv_list_create(bodyScreen);
  //lv_obj_set_size(wfList, tft.width() - 140, 210);
  lv_obj_set_size(wfList, tft.width() - 33, tft.height() - 54);
  lv_obj_center(wfList);
}

void buildSettings(lv_obj_t * scr) {
  settings = lv_obj_create(scr);
  lv_obj_add_style(settings, &border_style, 0);
  lv_obj_set_size(settings, tft.width() - 100, tft.height() - 190);
  lv_obj_align(settings, LV_ALIGN_TOP_RIGHT, -20, 20);

  settinglabel = lv_label_create(settings);
  lv_label_set_text(settinglabel, "Settings " LV_SYMBOL_SETTINGS);
  lv_obj_align(settinglabel, LV_ALIGN_TOP_LEFT, 0, 0);

  settingCloseBtn = lv_btn_create(settings);
  lv_obj_set_size(settingCloseBtn, 30, 30);
  lv_obj_align(settingCloseBtn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(settingCloseBtn, btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btnSymbol = lv_label_create(settingCloseBtn);
  lv_label_set_text(btnSymbol, LV_SYMBOL_CLOSE);
  lv_obj_center(btnSymbol);

  settingWiFiSwitch = lv_switch_create(settings);
  lv_obj_add_event_cb(settingWiFiSwitch, btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_align_to(settingWiFiSwitch, settinglabel, LV_ALIGN_TOP_RIGHT, 60, -10);
  lv_obj_add_flag(settings, LV_OBJ_FLAG_HIDDEN);

  // wfList = lv_list_create(settings);
  // lv_obj_set_size(wfList, tft.width() - 120, 210);
  // lv_obj_align_to(wfList, settinglabel, LV_ALIGN_TOP_LEFT, 0, 30);
}

static void list_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);


  if (code == LV_EVENT_CLICKED) {

    String selectedItem = String(lv_list_get_btn_text(wfList, obj));
    for (int i = 0; i < selectedItem.length() - 1; i++) {
      if (selectedItem.substring(i, i + 2) == " (") {
        ssidName = selectedItem.substring(0, i);
        lv_label_set_text_fmt(mboxTitle, "Selected WiFi SSID: %s", ssidName);
        lv_obj_move_foreground(mboxConnect);
        break;
      }
    }
  }
}

/*
 * NETWORK TASKS
 */

static void networkScanner() {
  xTaskCreate(scanWIFITask,
              "ScanWIFITask",
              4096,
              NULL,
              1,
              &ntScanTaskHandler);
}

static void networkConnector() {
  xTaskCreate(beginWIFITask,
              "beginWIFITask",
              2048,
              NULL,
              1,
              &ntConnectTaskHandler);
}

static void timerForNetwork(lv_timer_t *timer) {
  LV_UNUSED(timer);

  switch (networkStatus) {

    case NETWORK_SEARCHING:
      showingFoundWiFiList();
      break;

    case NETWORK_CONNECTED_POPUP:
      popupMsgBox("WiFi Connected!", "Now you'll get the current time soon.", scr2);
      networkStatus = NETWORK_CONNECTED;
      break;

    case NETWORK_CONNECTED:

      showingFoundWiFiList();
      updateLocalTime(cur_ssidName);
      break;

    case NETWORK_CONNECT_FAILED:
      networkStatus = NETWORK_SEARCHING;
      popupMsgBox("Oops!", "Please check your wifi password and try again.", scr2);
      break;

    default:
      break;
  }
}

static void scanWIFITask(void *pvParameters) {
  while (1) {
    foundWifiList.clear();
    int n = WiFi.scanNetworks();
    vTaskDelay(10);
    for (int i = 0; i < n; ++i) {
      String item = WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      foundWifiList.push_back(item);
      vTaskDelay(10);
    }
    vTaskDelay(5000);
  }
}


void beginWIFITask(void *pvParameters) {

  unsigned long startingTime = millis();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  vTaskDelay(100);

  WiFi.begin(ssidName.c_str(), ssidPW.c_str());
  while (WiFi.status() != WL_CONNECTED && (millis() - startingTime) < networkTimeout) {
    vTaskDelay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    networkStatus = NETWORK_CONNECTED_POPUP;
    cur_ssidName = ssidName;
  } else {
    networkStatus = NETWORK_CONNECT_FAILED;
  }

  vTaskDelete(NULL);
}

void buildPWMsgBox(lv_obj_t * scr) {

  mboxConnect = lv_obj_create(scr);
  lv_obj_add_style(mboxConnect, &border_style, 0);
  lv_obj_set_size(mboxConnect, tft.width() * 2 / 3, tft.height() * 0.6);
  lv_obj_align(mboxConnect, LV_ALIGN_CENTER, 0, -10);

  mboxTitle = lv_label_create(mboxConnect);
  lv_label_set_text(mboxTitle, "Selected WiFi SSID: ThatProject");
  lv_obj_align(mboxTitle, LV_ALIGN_TOP_LEFT, 0, 0);

  mboxPassword = lv_textarea_create(mboxConnect);
  lv_obj_set_size(mboxPassword, tft.width() / 2, 40);
  lv_obj_align_to(mboxPassword, mboxTitle, LV_ALIGN_TOP_LEFT, 0, 30);
  lv_textarea_set_placeholder_text(mboxPassword, "Password?");
  lv_obj_add_event_cb(mboxPassword, text_input_event_cb, LV_EVENT_ALL, keyboard);

  mboxConnectBtn = lv_btn_create(mboxConnect);
  lv_obj_add_event_cb(mboxConnectBtn, btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_align(mboxConnectBtn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(mboxConnectBtn);
  lv_label_set_text(btnLabel, "Connect");
  lv_obj_center(btnLabel);

  mboxCloseBtn = lv_btn_create(mboxConnect);
  lv_obj_add_event_cb(mboxCloseBtn, btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_align(mboxCloseBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t *btnLabel2 = lv_label_create(mboxCloseBtn);
  lv_label_set_text(btnLabel2, "Cancel");
  lv_obj_center(btnLabel2);
}

static void text_input_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);

  if (code == LV_EVENT_FOCUSED) {
    lv_obj_move_foreground(keyboard);
    lv_keyboard_set_textarea(keyboard, ta);
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
  }

  if (code == LV_EVENT_DEFOCUSED) {
    lv_keyboard_set_textarea(keyboard, NULL);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
  }
}

void makeKeyboard(lv_obj_t * scr) {
  keyboard = lv_keyboard_create(scr);
  lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t * popupMsgBox(String title, String msg, lv_obj_t * scr) {


  lv_obj_t * popupBox = lv_obj_create(scr);
  lv_obj_add_style(popupBox, &popupBox_style, 0);
  lv_obj_set_size(popupBox, tft.width() * 2 / 3, tft.height() / 2);
  lv_obj_center(popupBox);

  lv_obj_t *popupTitle = lv_label_create(popupBox);
  lv_label_set_text(popupTitle, title.c_str());
  lv_obj_set_width(popupTitle, tft.width() * 2 / 3 - 50);
  lv_obj_align(popupTitle, LV_ALIGN_TOP_LEFT, 0, 0);

  lv_obj_t *popupMSG = lv_label_create(popupBox);
  lv_obj_set_width(popupMSG, tft.width() * 2 / 3 - 50);
  lv_label_set_text(popupMSG, msg.c_str());
  lv_obj_align(popupMSG, LV_ALIGN_TOP_LEFT, 0, 40);

  lv_obj_t *popupBoxCloseBtn = lv_btn_create(popupBox);
  lv_obj_add_event_cb(popupBoxCloseBtn, btn_popup_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_align(popupBoxCloseBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(popupBoxCloseBtn);
  lv_label_set_text(btnLabel, "Okay");
  lv_obj_center(btnLabel);

  return popupBox;
}

void updateLocalTime(String wifiName) {

  String hourMinWithSymbol = LV_SYMBOL_WIFI;
  hourMinWithSymbol += "  ";
  hourMinWithSymbol += wifiName;
  lv_label_set_text(timeLabel, hourMinWithSymbol.c_str());
}

/**************************************************************************/

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch( &touchX, &touchY, 600 );

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print( "Data x " );
        Serial.println( touchX );

        Serial.print( "Data y " );
        Serial.println( touchY );
    }
}

/**
 * SPARC Lab logo image
 */
void lv_img_sparc(lv_obj_t * parent)
{
    /*Now create the actual image*/
    LV_IMG_DECLARE(sparc);
    lv_obj_t *img1 = lv_img_create(parent);
    lv_img_set_src(img1, &sparc);
    lv_obj_align(img1, LV_ALIGN_OUT_TOP_LEFT, 0, 0);
}

/*
 * Goodface image
 */
void lv_img_goodface(lv_obj_t * parent)
{
    /*Now create the actual image*/
    LV_IMG_DECLARE(goodface);
    lv_obj_t *img1 = lv_img_create(parent);
    lv_img_set_src(img1, &goodface);
    lv_obj_align(img1, LV_ALIGN_RIGHT_MID, 0, 0);
}

/*
 * Airsense image
 */
void lv_img_airsense(lv_obj_t * parent)
{
    /*Now create the actual image*/
    LV_IMG_DECLARE(airsense);
    lv_obj_t *img1 = lv_img_create(parent);
    lv_img_set_src(img1, &airsense);
    lv_obj_align(img1, LV_ALIGN_TOP_LEFT, 30, 0);
}

void lv_noWifi(lv_obj_t * parent)
{
    /*Now create the actual image*/
    LV_IMG_DECLARE(nowifi);
    lv_obj_t *img1 = lv_img_create(parent);
    lv_img_set_src(img1, &nowifi);
    lv_obj_align(img1, LV_ALIGN_TOP_RIGHT, -2, 0);
}

void lv_noSD(lv_obj_t * parent)
{
    /*Now create the actual image*/
    LV_IMG_DECLARE(nosd);
    lv_obj_t *img1 = lv_img_create(parent);
    lv_img_set_src(img1, &nosd);
    lv_obj_align(img1, LV_ALIGN_TOP_RIGHT, -30, 0);
}

void lv_Wifi(lv_obj_t * parent)
{
    /*Now create the actual image*/
    LV_IMG_DECLARE(greenwifi);
    lv_obj_t *img1 = lv_img_create(parent);
    lv_img_set_src(img1, &greenwifi);
    lv_obj_align(img1, LV_ALIGN_TOP_RIGHT, -2, 0);
}

void lv_img_datasymbol(lv_obj_t * parent)
{
    /* temperature symbol */
    LV_IMG_DECLARE(temp);
    lv_obj_t *img1 = lv_img_create(parent);
    lv_img_set_src(img1, &temp);
    lv_obj_align(img1, LV_ALIGN_LEFT_MID, 10, -55);

    /* humidity symbol */
    LV_IMG_DECLARE(humidity);
    lv_obj_t *img2 = lv_img_create(parent);
    lv_img_set_src(img2, &humidity);
    lv_obj_align(img2, LV_ALIGN_LEFT_MID, 5, -20);

    /* pressure symbol */
    LV_IMG_DECLARE(press);
    lv_obj_t *img3 = lv_img_create(parent);
    lv_img_set_src(img3, &press);
    lv_obj_align(img3, LV_ALIGN_LEFT_MID, 5, 15);

    /* pm2.5 symbol */
    LV_IMG_DECLARE(pm2p5);
    lv_obj_t *img4 = lv_img_create(parent);
    lv_img_set_src(img4, &pm2p5);
    lv_obj_align(img4, LV_ALIGN_LEFT_MID, 5, 50);


}

// static void event_cb(lv_event_t * e)
// {
//     //LV_LOG_USER("Clicked");

//     static uint32_t cnt = 1;
//     lv_obj_t *label = lv_event_get_target(e);
//     lv_obj_del_delayed(label, 1000);
// }

void lv_label_data(float _temperature, float _humidity, float _press, float _pm2p5, lv_obj_t * parent)
{
    data = lv_label_create(parent);
    // char message[1000];
    // sprintf(message, "%0.1f°C\n\n%0.1f%% \n\n%0.1fPa\n\n    %0.1fug/m3", _temperature, _humidity, _press, _pm2p5);
    //lv_obj_add_event_cb(data, event_cb, LV_EVENT_DELETE, NULL);
    lv_label_set_text_fmt(data,  "%0.1f°C\n\n%0.1f%% \n\n%0.1fPa\n\n    %0.1fug/m3", _temperature, _humidity, _press, _pm2p5);
    lv_obj_align(data, LV_ALIGN_LEFT_MID, 41, 0);
    //lv_event_send(data, LV_EVENT_DELETE, NULL);
    timer1 = lv_timer_create(my_timer_cb, 1000, data);
}

void my_timer_cb(lv_timer_t * tm){
    LV_UNUSED(tm);
    lv_obj_clean(data);
    delay( 100 );
    char message[1000];
    sprintf(message, "%0.1f°C\n\n%0.1f%% \n\n%0.1fPa\n\n    %0.1fug/m3", tempe, hum, presse, pm2p5e);
    lv_label_set_text(data, message);
}

void lv_clock(lv_obj_t * parent, uint8_t _hour, uint8_t _minute, uint8_t _day, uint8_t _month, uint16_t _year){
    lv_obj_t *label = lv_label_create(parent);
    char mes[100];
    sprintf(mes, "     %d:%d\n%d/%d/%d", _hour, _minute, _day, _month, _year);
    lv_label_set_text(label, mes);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
    
}



ILI9341Display::ILI9341Display()
{
}

ILI9341Display::~ILI9341Display()
{
}

void ILI9341Display::init()
{
    //status.current_screen = PROGRESS_SCREEN;
#ifndef _LVGV_IN_USE_
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    tft.init();
    // tft.fillScreen(TFT_BLACK);
    tft.setRotation(DISPLAY_SET_ROTATION);
#else
    lv_init();
#if USE_LV_LOG != 0
    lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

    tft.begin();          /* TFT init */
    tft.setRotation( 3 ); /* Landscape orientation, flipped */

    /*Set the touchscreen calibration data,
     the actual data for your display can be acquired using
     the Generic -> Touch_calibrate example from the TFT_eSPI library*/
    uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
    tft.setTouch( calData );

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );
#endif

#ifdef _DB_LOG_
    Serial.println("Init TFT display");
#endif

}

void ILI9341Display::progressScreen(bool _sd, bool _pms7003, bool _bme280, bool _ds3231, bool _sds011, bool _htu21d, bool _sht85)
{
#ifdef _DB_LOG_
    Serial.println("Progess screen");
#endif
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0, 2);
    tft.setTextColor(TFT_WHITE);
    tft.print("Device ");
    tft.println("DEVICE_NAME");
    tft.print("Firmware version ");
    tft.println("FIRMWARE_VERSION_SPARCLAB");
    tft.println("Checking device, please wait...");
    if (_bme280)
    {
        tft.setTextColor(TFT_WHITE);
        tft.println("BME280 is connected");
    }
    else
    {
        tft.setTextColor(TFT_RED);
        tft.println("BME280 is disconnected");
    }
    if (_sht85)
    {
        tft.setTextColor(TFT_WHITE);
        tft.println("SHT85 is connected");
    }
    else
    {
        tft.setTextColor(TFT_RED);
        tft.println("SHT85 is disconnected");
    }
    if (_ds3231)
    {
        tft.setTextColor(TFT_WHITE);
        tft.println("DS3231 is connected");
    }
    else
    {
        tft.setTextColor(TFT_RED);
        tft.println("DS3231 is disconnected");
    }
#ifdef PMS7003_SENSOR
    if (_pms7003)
    {
        tft.setTextColor(TFT_WHITE);
        tft.println("PMS7003 is connected");
    }
    else
    {
        tft.setTextColor(TFT_RED);
        tft.println("PMS7003 is disconnected");
    }
#endif
    if (_sd)
    {
        tft.setTextColor(TFT_WHITE);
        tft.println("SD card is connected");
    }
    else
    {
        tft.setTextColor(TFT_RED);
        tft.println("SD card is disconnected");
    }
    delay( 1000 );
}
void mainScreen(lv_obj_t * scr)
{
#ifdef _DB_LOG_
    Serial.println("Main screen");
#endif
#ifdef _LVGV_IN_USE_
    lv_style_init(&background);

    lv_style_set_bg_color(&background, lv_color_white());
   
    lv_obj_add_style(scr, &background, 0);

    lv_img_sparc(scr);

    lv_img_goodface(scr);

    lv_img_airsense(scr);

    lv_img_datasymbol(scr);

    lv_label_data(tempe, hum, presse, pm2p5e, scr);

    lv_clock(scr, 20, 30, 21, 12, 2022);
    
    

/*
    cont = lv_menu_cont_create(main_page);
    label = lv_label_create(cont);
    lv_label_set_text(label, "Item 2");
    lv_menu_set_load_page_event(menu, cont, sub_page2);


    cont = lv_menu_cont_create(main_page);
    label = lv_label_create(cont);
    lv_label_set_text(label, "Item 3 (Click me!)");
    lv_menu_set_load_page_event(menu, cont, sub_page3);
*/

    
#else
    tft.setRotation(DISPLAY_SET_ROTATION);
    tft.fillScreen(TFT_WHITE);
    drawSdJpeg("/airsense.jpg", 10, 1);
    drawSdJpeg("/sparc.jpg", 95, 3);
    tft.drawLine(5, 28, 315, 28, TFT_BLACK);

    drawSdJpeg("/press.jpg", 10, 120);
    drawSdJpeg("/humidity.jpg", 10, 80);
    drawSdJpeg("/temp.jpg", 10, 40);
    
#ifdef PMS7003_SENSOR
    drawSdJpeg("/pm2p5.jpg", 170, 50);
    drawSdJpeg("/goodface.jpg", 10, 50);
    drawSdJpeg("/goodaqimeter.jpg", 160, 100);
#endif
#endif
}
void ILI9341Display::guiHandler()
{
#ifdef _LVGV_IN_USE_
    lv_task_handler();
    delay(10);
#endif
}
void ILI9341Display::showScreen(lv_obj_t * scr)
{
    switch (this->status.current_screen)
    {
    case PROGRESS_SCREEN:
        // progressScreen();
        break;
    case MENU_SCREEN:
        break;
    case MAIN_SCREEN:
        mainScreen(scr);
        break;
    default:
        break;
    }
}

void wifiScreen(lv_obj_t * scr){
    setStyle();
    makeKeyboard(scr);
    buildStatusBar(scr);
    buildPWMsgBox(scr);
    buildBody(scr);
    buildSettings(scr);
}

void del_obj2(){
  lv_style_reset(&border_style);
  lv_style_reset(&popupBox_style);
  if (ntScanTaskHandler != NULL) {
    vTaskDelete(ntScanTaskHandler);
    ntScanTaskHandler = NULL;
    lv_timer_del(timer);
    lv_obj_clean(wfList);
  }
  
}

void del_obj1(){
  lv_timer_del(timer1);
  lv_obj_clean(data);
  lv_style_reset(&background);
}

void ILI9341Display::updateData(float _temp, float _humi, float _pres, float _pm2p5/*, char *_mac*/)
{
#ifdef _DB_LOG_
    //Serial.println("Update data to display");
#endif
#ifdef _LVGV_IN_USE_
    //lv_gui_update_value(_temp, _humi, _pres, _pm2p5);
#else
#ifdef PMS7003_SENSOR
    //PM2.5
    tft.setCursor(215, 60, 4);
    tft.setTextColor(TFT_BLACK);
    tft.fillRect(215, 60, 100, 20, TFT_WHITE);
    tft.setCursor(215, 60, 4);
    tft.printf("%.1f", _pm2p5);
    tft.setCursor(275, 65, 2);
    tft.print(" ug/m");
    tft.setCursor(tft.getCursorX(), tft.getCursorY() - 5, 2);
    tft.print("3");

    if (_pm2p5 <= 50)
    {
        drawSdJpeg("/goodface.jpg", 10, 50);
        drawSdJpeg("/goodaqimeter.jpg", 160, 100);
    }
    else if (_pm2p5 <= 100)
    {
        drawSdJpeg("/normalface.jpg", 10, 50);
        drawSdJpeg("/normalaqimeter.jpg", 160, 100);
    }
    else if (_pm2p5 <= 150)
    {
        drawSdJpeg("/notgoodface.jpg", 10, 50);
        drawSdJpeg("/notgoodaqimeter.jpg", 160, 100);
    }
    else if (_pm2p5 <= 200)
    {
        drawSdJpeg("/badface.jpg", 10, 50);
        drawSdJpeg("/badaqimeter.jpg", 160, 100);
    }
    else if (_pm2p5 <= 300)
    {
        drawSdJpeg("/verybadface.jpg", 10, 50);
        drawSdJpeg("/verybadaqimeter.jpg", 160, 100);
    }
    else
    {
        drawSdJpeg("/extremlybadface.jpg", 10, 50);
        drawSdJpeg("/extremlybadaqimeter.jpg", 160, 100);
    }
#endif
    //Temperature
    tft.fillRect(35, 40, 50, 20, TFT_WHITE);
    tft.setCursor(35, 40, 4);
    tft.printf("%.1f", _temp);
    tft.setCursor(85, 41, 2);
    tft.print("o");
    tft.setCursor(tft.getCursorX(), tft.getCursorY() + 5, 2);
    tft.print("C");

    //Humidity
    tft.fillRect(35, 80, 50, 20, TFT_WHITE);
    tft.setCursor(35, 80, 4);
    tft.printf("%.1f", _humi);
    tft.setCursor(85, 80, 2);
    tft.print("%");

    //Pressure
    tft.fillRect(35, 120, 140, 20, TFT_WHITE);
    tft.setCursor(35, 120, 4);
    tft.printf("%.1f", _pres );/// 101325.0);
    tft.setCursor(145, 120, 2);
    tft.print("Pa");

    tft.setCursor(190, 225, 2);
    tft.setTextColor(TFT_BLACK);
    tft.printf("ID: %s", _mac);

#endif

    tempe = _temp;
    hum = _humi;
    presse = _pres;
    pm2p5e = _pm2p5;

}

void ILI9341Display::updateClock(uint8_t _hour, uint8_t _min)
{
#ifdef _DB_LOG_
    Serial.println("Update clock to display");
#endif
#ifdef _LVGV_IN_USE_
    //lv_gui_update_time(_hour, _min);
#else

    tft.fillRect(280, 8, 40, 20, TFT_WHITE);
    tft.setCursor(280, 8, 2);
    tft.setTextColor(TFT_BLACK);

    if (_hour < 10)
        tft.print(0);
    tft.print(_hour);
    tft.print(":");

    if (_min < 10)
        tft.print(0);
    tft.print(_min);
#endif
}

#ifdef _LVGV_IN_USE_
void ILI9341Display::showSDsymbol()
{
    //lv_gui_show_sdcard_symbol();
}
void ILI9341Display::showNonSDsymbol(lv_obj_t * scr)
{
    lv_noSD(scr);
}
void showWiFiSymbol(lv_obj_t * scr)
{
    lv_Wifi(scr);
}
void ILI9341Display::showWiFiWaitSymbol()
{
    //lv_gui_show_wifi_symbol();
}
void showNonWiFiSymbol(lv_obj_t * scr)
{
    lv_noWifi(scr);
}
void ILI9341Display::showWiFiConfigSymbol()
{
    //lv_gui_show_wifi_symbol();
}
void ILI9341Display::showWiFiButNoInternetSymbol()
{
    //lv_gui_show_wifi_symbol();
}

#else
void ILI9341Display::showSDsymbol()
{
    drawSdJpeg("/sd.jpg", 225, 1);
}
void ILI9341Display::showNonSDsymbol()
{
    drawSdJpeg("/nosd.jpg", 225, 1);
}
void ILI9341Display::showWiFiSymbol()
{
    drawSdJpeg("/greenwifi.jpg", 250, 1);
}
void ILI9341Display::showWiFiWaitSymbol()
{
    drawSdJpeg("/nowifi.jpg", 250, 1);
}
void ILI9341Display::showNonWiFiSymbol()
{
    drawSdJpeg("/redwifi.jpg", 250, 1);
}
void ILI9341Display::showWiFiConfigSymbol()
{
    drawSdJpeg("/orangewifi.jpg", 250, 1);
}
void ILI9341Display::showWiFiButNoInternetSymbol()
{
    drawSdJpeg("/wifinointernet.jpg", 250, 1);
}
#endif
ILI9341Status ILI9341Display::getStatus()
{
    return this->status;
}
void ILI9341Display::setDisplayScreen(SCREEN_ID _screen)
{
    this->status.previous_screen = status.current_screen;
    this->status.current_screen = _screen;
}

/***************************** TEST ************************************/


static void btn_event_cb1(lv_event_t * e);
static void btn_event_cb2(lv_event_t * e);


void show_maindisplay(){
    scr1 = lv_obj_create(NULL);
    lv_obj_t *btn1 = lv_btn_create(scr1);  
    lv_obj_set_size(btn1, 40, 30);
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    //lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, btn_click_action);
    lv_obj_add_event_cb(btn1, btn_event_cb1, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

    lv_obj_t * label1 = lv_label_create(btn1);          /*Add a label to the button*/
    lv_label_set_text(label1, LV_SYMBOL_RIGHT);                     /*Set the labels text*/
    lv_obj_center(label1);
    mainScreen(scr1);
    if(WiFi.status() != WL_CONNECTED){
        showNonWiFiSymbol(scr1);
    }
    else {
        showWiFiSymbol(scr1);
    }
}
void show_wifidisplay(){
    scr2 = lv_obj_create(NULL);
    lv_obj_t *btn1 = lv_btn_create(scr2);  
    lv_obj_set_size(btn1, 40, 30);
    lv_obj_align(btn1, LV_ALIGN_TOP_LEFT, 0, 0);
    //lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, btn_click_action);
    lv_obj_add_event_cb(btn1, btn_event_cb2, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

    lv_obj_t * label1 = lv_label_create(btn1);          /*Add a label to the button*/
    lv_label_set_text(label1, LV_SYMBOL_LEFT);                     /*Set the labels text*/
    lv_obj_center(label1);
    wifiScreen(scr2);
}

// void lv_example_style_3()
// {
//     scr1 = lv_obj_create(NULL);

//     /*Change the active screen's background color*/
//     lv_obj_set_style_bg_color(scr1, lv_color_hex(0x003a57), LV_PART_MAIN);

//     /*Create a white label, set its text and align it to the center*/
//     lv_obj_t * label = lv_label_create(scr1);
//     lv_label_set_text(label, "Hello world");
//     lv_obj_set_style_text_color(scr1, lv_color_hex(0xffffff), LV_PART_MAIN);
//     lv_obj_align(scr1, LV_ALIGN_CENTER, 0, 0);

//     lv_obj_t *btn1 = lv_btn_create(scr1);  
//     lv_obj_set_size(btn1, 120, 40);
//     lv_obj_set_pos(btn1, 0, 0);
//     //lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, btn_click_action);
//     lv_obj_add_event_cb(btn1, btn_event_cb1, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

//     lv_obj_t * label1 = lv_label_create(btn1);          /*Add a label to the button*/
//     //lv_label_set_text(label1, "Button2");                     /*Set the labels text*/
//     if(WiFi.status() == WL_CONNECTED)
//         lv_label_set_text(label1, "connected");
//     else
//         lv_label_set_text(label1, "not connect");  
//     lv_obj_center(label1);
    
// }

static void btn_event_cb1(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
            //lv_obj_clean(lv_scr_act());
            show_wifidisplay();
            //lv_example_style_3();
            lv_scr_load(scr2);

            
            //lv_obj_clean(lv_scr_act());
            lv_obj_del(scr1);
            del_obj1();
        
    }
}

static void btn_event_cb2(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
            //lv_obj_clean(lv_scr_act());
            show_maindisplay();
            //lv_example_style_3();
            lv_scr_load(scr1);

            //lv_obj_clean(lv_scr_act());
            lv_obj_del(scr2);
            del_obj2();
            
    }
}

void ILI9341Display::lv_example_style_3_class(){
  show_maindisplay();
  lv_scr_load(scr1);
}