/*******************************************************************
   This is a clumsy modified code from the original. This works, but you should go to the original repo
   unless you like bad written code.
   Tucho
   ***********
    A scrolling text sign on 1 RGB LED matrix where the text
    can be updated via telegram.

    Parts Used:
    2 x 64x32 P3 Matrix display * - https://s.click.aliexpress.com/e/_dYz5DLt
    ESP32 D1 Mini * - https://s.click.aliexpress.com/e/_dSi824B
    ESP32 I2S Matrix Shield (From my Tindie) = https://www.tindie.com/products/brianlough/esp32-i2s-matrix-shield/

 *  * = Affilate

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/


    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

// ----------------------------
// Standard Libraries
// ----------------------------

#include <WiFi.h>
#include <WiFiClientSecure.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
// This is the library for interfacing with the display

#include <UniversalTelegramBot.h>
// This is the library for connecting to Telegram

// -------------------------------------
// ------- Replace the following! ------
// -------------------------------------

// Wifi network station credentials
#define WIFI_SSID "xxxxxxxxxxxxxxxxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxxxxxxxxxxxxxxxx"

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "xxxxxxxxxxxxxxxxx:xxxxxxxxxxxxxxxx-xxxxxxxxxxxxxx"

//------- ---------------------- ------
#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another



MatrixPanel_I2S_DMA *dma_display = nullptr;

uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// -------------------------------------
// -------   Text Configuraiton   ------
// -------------------------------------


String text = "Hello!"; //Starting Text

//------- ---------------------- ------



// For scrolling Text
unsigned long isAnimationDue;
#define FONT_SIZE 2 // Text will be FONT_SIZE x 8 pixels tall.
int delayBetweeenAnimations = 35; // How fast it scrolls, Smaller == faster
int scrollXMove = -1; //If positive it would scroll right
int textXPosition;
int textYPosition;

void setup() {
  Serial.begin(115200);
  // Module configuration
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   // module width
    PANEL_RES_Y,   // module height
    PANEL_CHAIN    // Chain length
  );

  mxconfig.gpio.e = 18;
  mxconfig.double_buff = true; // Turn of double buffer
  mxconfig.clkphase = true;
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;

  // Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90); //0-255
  dma_display->clearScreen();
  // fix the screen with green becaus it's an incredibly elegant way to start
  dma_display->fillRect(0, 0, dma_display->width(), dma_display->height(), dma_display->color444(0, 15, 0));
  delay(500);
  dma_display->fillScreen(myBLACK);

  
  dma_display->setTextSize(FONT_SIZE);     // size 1 == 8 pixels high
  dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
  dma_display->setTextColor(myRED);
  
  textXPosition = dma_display->width(); // Will start one pixel off screen
  textYPosition = dma_display->height() / 2 - (FONT_SIZE * 8 / 2); // This will center the text

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());


}

int16_t xOne, yOne;
uint16_t w, h;
bool checkTelegram = false;



void loop() {
  unsigned long now = millis();

  if (now > isAnimationDue)
  {
    dma_display->flipDMABuffer();
      
    // This sets the timer for when we should scroll again.
    isAnimationDue = now + delayBetweeenAnimations;
    textXPosition += scrollXMove;
    // Checking if the very right of the text off screen to the left
    dma_display->getTextBounds(text, textXPosition, textYPosition, &xOne, &yOne, &w, &h);
    if (textXPosition + w <= 0)
    {
      checkTelegram = true;
      textXPosition = dma_display->width();
    }
    dma_display->setCursor(textXPosition, textYPosition);
    // The display has to do less updating if you only black out the area
    // the text is
    dma_display->fillRect(0, textYPosition, dma_display->width(), 16, myBLACK);
    //we print what we want to show
    dma_display->print(text);
    // Telegram will only be checked when there is no data on screen
  // as checking interfeers with scrolling.
  if (checkTelegram)
  {
    checkTelegram = false;
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    if (numNewMessages > 0)
    {
      Serial.println("got response");

      // This is where you would check is the message from a valid source
      // You can get your ID from myIdBot in the Telegram client
      //if(bot.messages[0].chat_id == "175753388")
      if (true) {
        // Takes the contents of the message, and sets it to be displayed.
        text = bot.messages[0].text;
      }
    }
  }
  }
}
