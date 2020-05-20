// EL SIGUIENTE CODIGO ES DESARROLLADO COMO EJEMPLO SIN GARANTIA PARA LA VERSION BETA DE LA API DE ANALISIS DE IMAGEN POR DEEP LEARNING CON NOMBRE CLAVE "PROYECTO HORUS".
// PARA CONOCER MAS SOBRE EL PROYECTO Y SUS AVANCES ENTRA EN HTTP://www.proyectohorus.com.ar


#include "esp_http_client.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "Arduino.h"

const char* ssid = "ACA VA LA SSID DE TU WIFI";
const char* password = "ACA VA LA CLAVE DE TU WIFI";

// INDICA LOS ms ENTRE ENVIO Y ENVIO DE SOLICITUD A LA API
int capture_interval = 500;

// PARA OBTENER EL UUID DEL PERFIL COMO EN USUARIO Y LA CLAVE PODES ENTRAR EN HTTP://www.proyectohorus.com.ar, desde alli podras administrar via web o bien bajar nuestro administrador instalable.
String APIprofileuuid = "ACA VA EL UUID DEL PERFIL CREADO DESDE EL ADMINISTRADOR DEL PROYECTO HORUS";
String APIUser = "ACA VA TU USUARIO DE PROYECTO HORUS";
String APIPassword = "ACA VA TU CLAVE DE PROYECTO HORUS";
String APItoken = "";

// INDICA SI SE CONECTO CORRECTAMENTE AL WIFI
bool internet_connected = false;
long current_millis;
long last_capture_millis = 0;

// INDICA SI SE RECIBIO LA SOLICITUD DESDE LA API ANTES DE ENVIAR UNA NUEVA
bool Ready = true;

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// LILYGO® TTGO T-Journal ESP32 Camera Module Development Board OV2640
// #define Y2_GPIO_NUM 17
// #define Y3_GPIO_NUM 35
// #define Y4_GPIO_NUM 34
// #define Y5_GPIO_NUM 5
// #define Y6_GPIO_NUM 39
// #define Y7_GPIO_NUM 18
// #define Y8_GPIO_NUM 36
// #define Y9_GPIO_NUM 19
// #define XCLK_GPIO_NUM 27
// #define PCLK_GPIO_NUM 21
// #define VSYNC_GPIO_NUM 22
// #define HREF_GPIO_NUM 26
// #define SIOD_GPIO_NUM 25
// #define SIOC_GPIO_NUM 23
// #define PWDN_GPIO_NUM -1
// #define RESET_GPIO_NUM 15

// EN ESTA FUNCION OBTENEMOS EL TOKEN
String GetToken(String user, String passwd, String profileuuid)
{
      String Token = "";
      
      HTTPClient http;
      
      http.begin("http://server1.proyectohorus.com.ar/api/v2/functions/login"); 
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      int httpCode = http.POST("user=" + user + "&password=" + passwd + "&profileuuid=" + profileuuid);
      
      if (httpCode > 0) 
      {
            String payload = http.getString();
            
            if (getValue(payload,'|',0) == "200")
            {
                  Token = "Bearer " + getValue(payload,'|',1);
                  Serial.println(Token);
            }
            else
            {
                  Serial.println(getValue(payload,'|',1));
            }
      }
      
      http.end();
      
      return Token;
}

// ESTA FUNCION ES SIMILAR A SPLIT EN C# O PYTHON
String getValue(String data, char separator, int index)
{
      int found = 0;
      int strIndex[] = {0, -1};
      int maxIndex = data.length()-1;
      
      for(int i=0; i<=maxIndex && found<=index; i++)
      {
            if(data.charAt(i)==separator || i==maxIndex)
            {
                  found++;
                  strIndex[0] = strIndex[1]+1;
                  strIndex[1] = (i == maxIndex) ? i+1 : i;
            }
      }
      
      return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// CONECTA AL WIFI
bool init_wifi()
{
      int connAttempts = 0;
      Serial.println("\r\nConnecting to: " + String(ssid));
      WiFi.begin(ssid, password);
      
      while (WiFi.status() != WL_CONNECTED ) 
      {
            delay(500);
            Serial.print(".");
            if (connAttempts > 10) 
                  return false;
                  
            connAttempts++;
      }
      return true;
}


// ESTE EVENTO SE EJECUTA AL ENTRAR UN RESPONSE DESDE LA API
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
      switch (evt->event_id) 
      {
            case HTTP_EVENT_ON_DATA:
                  String Data = (char*)evt->data;
                  String Content = Data.substring(0,evt->data_len);
                  
                  if (Content != "")
                  {
                        String ErrorCode = getValue(Content,'|',0);

                        // SI LA API RESPONDIO CON CODIGO DE ERROR 200 SIGNIFICA QUE TODO LLEGO OK.
                        if (ErrorCode == "200")
                        {
                              String StatusCode = getValue(Content,'|',1);
                              String ymin = getValue(Content,'|',2);
                              String xmin = getValue(Content,'|',3);
                              String ymax = getValue(Content,'|',4);
                              String xmax = getValue(Content,'|',5);
                              String UUIDDetection = getValue(Content,'|',6);
                              String UUIDProfile = getValue(Content,'|',7);
                              String Confidence = getValue(Content,'|',8);
                              
                              if (UUIDDetection != "NOT FOUND" and UUIDDetection != "FAIL")
                              {
                                    HTTPClient http;
                                    
                                    http.begin("http://server1.proyectohorus.com.ar/api/v2/admin/accounts/users/profiles/detections=" + UUIDDetection + "/value"); 
                                    http.addHeader("Authorization", APItoken); 
                                    http.addHeader("Content-Type", "text/html"); 
                                    
                                    int httpCode = http.GET();
                                    
                                    if (httpCode > 0) 
                                    {
                                          String payload = http.getString();
                                          Serial.println(getValue(payload,'|',1));

                                          if (getValue(payload,'|',1) == "detected")
                                          {
                                            digitalWrite(2, HIGH);
                                            delay(1000);
                                            digitalWrite(2, LOW);
                                          }
                                          else
                                          {
                                            if (getValue(payload,'|',1) == "no detected")
                                            {
                                              digitalWrite(14, HIGH);
                                              delay(1000);
                                              digitalWrite(14, LOW);
                                            }
                                          }
                                    }
                                    
                                    http.end();
                              }
                        }
                        else
                        {
                              // SI NO LLEGO CON CODIGO 200 IMPLICA QUE ALGO OCURRIO Y PONEMOS EN CERO LA VARIABLE DE TOKEN PARA QUE INTENTE RECONECTAR
                              APItoken = "";
                        }
                        
                        Ready = true;
                  }
                  break;
      }
      return ESP_OK;
}


// ENVIA LA IMAGEN CAPTURADA DE LA CAMARA A LA API DE HORUS
static esp_err_t take_send_photo()
{
      camera_fb_t * fb = NULL;
      esp_err_t res = ESP_OK;
      
      fb = esp_camera_fb_get();
      
      if (!fb) 
      {
            Serial.println("Camera capture failed");
            return ESP_FAIL;
      }
      
      esp_http_client_handle_t http_client;
      esp_http_client_config_t config_client = {0};
      
      config_client.url = "http://server1.proyectohorus.com.ar/api/v2/functions/object/detection?responseformat=pipe";
      config_client.event_handler = _http_event_handler;
      config_client.method = HTTP_METHOD_POST;
      
      http_client = esp_http_client_init(&config_client);
      
      esp_http_client_set_post_field(http_client, (const char *)fb->buf, fb->len);
      esp_http_client_set_header(http_client, "Content-Type", "image/jpg");
      
      esp_http_client_set_header(http_client, "Authorization",  APItoken.c_str());
      
      esp_err_t err = esp_http_client_perform(http_client);
      
      esp_http_client_cleanup(http_client);
      
      esp_camera_fb_return(fb);
}

void setup()
{
      Serial.begin(115200);

      pinMode(2, OUTPUT);
      pinMode(14, OUTPUT);
      
      
      if (init_wifi()) 
      { 
            internet_connected = true;
            Serial.println("Internet connected");
      }
      
      camera_config_t config;
      config.ledc_channel = LEDC_CHANNEL_0;
      config.ledc_timer = LEDC_TIMER_0;
      config.pin_d0 = Y2_GPIO_NUM;
      config.pin_d1 = Y3_GPIO_NUM;
      config.pin_d2 = Y4_GPIO_NUM;
      config.pin_d3 = Y5_GPIO_NUM;
      config.pin_d4 = Y6_GPIO_NUM;
      config.pin_d5 = Y7_GPIO_NUM;
      config.pin_d6 = Y8_GPIO_NUM;
      config.pin_d7 = Y9_GPIO_NUM;
      config.pin_xclk = XCLK_GPIO_NUM;
      config.pin_pclk = PCLK_GPIO_NUM;
      config.pin_vsync = VSYNC_GPIO_NUM;
      config.pin_href = HREF_GPIO_NUM;
      config.pin_sscb_sda = SIOD_GPIO_NUM;
      config.pin_sscb_scl = SIOC_GPIO_NUM;
      config.pin_pwdn = PWDN_GPIO_NUM;
      config.pin_reset = RESET_GPIO_NUM;
      config.xclk_freq_hz = 20000000;
      config.pixel_format = PIXFORMAT_JPEG;
      config.frame_size = FRAMESIZE_SVGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
      
      esp_err_t err = esp_camera_init(&config);
      
      if (err != ESP_OK) 
      {
            Serial.printf("Camera init failed with error 0x%x", err);
            return;
      }
}

void loop()
{
      // SI EL TOKEN ESTA EN NULO SOLICITO UN TOKEN
      if (APItoken == "")
      {
            APItoken = GetToken(APIUser, APIPassword, APIprofileuuid);
      }
      else
      {
            // SI TENGO TOKEN HAGO MI SOLICITUD DE ANALISIS A LA API
            current_millis = millis();
            if (current_millis - last_capture_millis > capture_interval) 
            { 
                  last_capture_millis = millis();
                  if (Ready == true)
                  {
                        Ready =  false;
                        take_send_photo();
                  }
            }
      }
}
